/*
 * Cagebreak: A Wayland tiling compositor.
 *
 * Copyright (C) 2020-2022 The Cagebreak Authors
 * Copyright (C) 2018-2019 Jente Hidskes
 *
 * See the LICENSE file accompanying this file.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdbool.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/box.h>
#include <wlr/util/edges.h>
#include <wlr/util/log.h>

#include "output.h"
#include "server.h"
#include "view.h"
#include "workspace.h"
#include "xdg_shell.h"

static void
xdg_decoration_handle_destroy(struct wl_listener *listener, void *_data) {
	struct cg_xdg_decoration *xdg_decoration =
	    wl_container_of(listener, xdg_decoration, destroy);

	wl_list_remove(&xdg_decoration->destroy.link);
	wl_list_remove(&xdg_decoration->request_mode.link);
	free(xdg_decoration);
}

#if CG_HAS_FANALYZE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wanalyzer-malloc-leak"
#endif
static void
xdg_decoration_handle_request_mode(struct wl_listener *listener, void *_data) {
	struct cg_xdg_decoration *xdg_decoration =
	    wl_container_of(listener, xdg_decoration, request_mode);
	enum wlr_xdg_toplevel_decoration_v1_mode mode;

	mode = WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE;
	wlr_xdg_toplevel_decoration_v1_set_mode(xdg_decoration->wlr_decoration,
	                                        mode);
}
#if CG_HAS_FANALYZE
#pragma GCC diagnostic pop
#endif

static void
xdg_popup_destroy(struct cg_view_child *child) {
	if(!child) {
		return;
	}

	view_damage_child(child, true);
	struct cg_xdg_popup *popup = (struct cg_xdg_popup *)child;
	wl_list_remove(&popup->destroy.link);
	wl_list_remove(&popup->map.link);
	wl_list_remove(&popup->unmap.link);
	wl_list_remove(&popup->new_popup.link);
	view_child_finish(&popup->view_child);
	free(popup);
}

static void
handle_xdg_popup_map(struct wl_listener *listener, void *_data) {
	struct cg_xdg_popup *popup = wl_container_of(listener, popup, map);
	view_damage_whole(popup->view_child.view);
}

static void
handle_xdg_popup_unmap(struct wl_listener *listener, void *_data) {
	struct cg_xdg_popup *popup = wl_container_of(listener, popup, unmap);
	view_damage_whole(popup->view_child.view);
}

static void
handle_xdg_popup_destroy(struct wl_listener *listener, void *_data) {
	struct cg_xdg_popup *popup = wl_container_of(listener, popup, destroy);
	struct cg_view_child *view_child = (struct cg_view_child *)popup;
	xdg_popup_destroy(view_child);
}

static void
xdg_popup_create(struct cg_view *view, struct wlr_xdg_popup *wlr_popup);

static void
popup_handle_new_xdg_popup(struct wl_listener *listener, void *data) {
	struct cg_xdg_popup *popup = wl_container_of(listener, popup, new_popup);
	struct wlr_xdg_popup *wlr_popup = data;
	xdg_popup_create(popup->view_child.view, wlr_popup);
}

static void
popup_unconstrain(struct cg_xdg_popup *popup) {
	struct cg_view *view = popup->view_child.view;
	struct wlr_box *popup_box = &popup->wlr_popup->geometry;

	struct wlr_output_layout *output_layout = view->server->output_layout;
	struct wlr_box *view_output_box = wlr_output_layout_get_box(
	    output_layout, view->workspace->output->wlr_output);
	struct wlr_output *wlr_output = wlr_output_layout_output_at(
	    output_layout, view_output_box->x + view->ox + popup_box->x,
	    view_output_box->y + view->oy + popup_box->y);
	struct wlr_box *output_box =
	    wlr_output_layout_get_box(output_layout, wlr_output);

	struct wlr_box output_toplevel_box = {.x = -view->ox,
	                                      .y = -view->oy,
	                                      .width = output_box->width,
	                                      .height = output_box->height};

	wlr_xdg_popup_unconstrain_from_box(popup->wlr_popup, &output_toplevel_box);
}

static void
xdg_popup_create(struct cg_view *view, struct wlr_xdg_popup *wlr_popup) {
	struct cg_xdg_popup *popup = calloc(1, sizeof(struct cg_xdg_popup));
	if(!popup) {
		return;
	}

	popup->wlr_popup = wlr_popup;
	view_child_init(&popup->view_child, NULL, view, wlr_popup->base->surface);
	popup->view_child.destroy = xdg_popup_destroy;
	popup->destroy.notify = handle_xdg_popup_destroy;
	wl_signal_add(&wlr_popup->base->events.destroy, &popup->destroy);
	popup->map.notify = handle_xdg_popup_map;
	wl_signal_add(&wlr_popup->base->events.map, &popup->map);
	popup->unmap.notify = handle_xdg_popup_unmap;
	wl_signal_add(&wlr_popup->base->events.unmap, &popup->unmap);
	popup->new_popup.notify = popup_handle_new_xdg_popup;
	wl_signal_add(&wlr_popup->base->events.new_popup, &popup->new_popup);

	popup_unconstrain(popup);
}

static void
handle_new_xdg_popup(struct wl_listener *listener, void *data) {
	struct cg_xdg_shell_view *xdg_shell_view =
	    wl_container_of(listener, xdg_shell_view, new_popup);
	struct wlr_xdg_popup *wlr_popup = data;
	xdg_popup_create(&xdg_shell_view->view, wlr_popup);
}

static struct cg_xdg_shell_view *
xdg_shell_view_from_view(struct cg_view *view) {
	return (struct cg_xdg_shell_view *)view;
}

static const struct cg_xdg_shell_view *
xdg_shell_view_from_const_view(const struct cg_view *view) {
	return (const struct cg_xdg_shell_view *)view;
}

static char *
get_title(const struct cg_view *view) {
	const struct cg_xdg_shell_view *xdg_shell_view =
	    xdg_shell_view_from_const_view(view);
	return xdg_shell_view->xdg_surface->toplevel->title;
}

static bool
is_primary(const struct cg_view *view) {
	const struct cg_xdg_shell_view *xdg_shell_view =
	    xdg_shell_view_from_const_view(view);
	struct wlr_xdg_surface *parent =
	    xdg_shell_view->xdg_surface->toplevel->parent;
	/* FIXME: role is 0? */
	return parent == NULL; /*&& role == WLR_XDG_SURFACE_ROLE_TOPLEVEL */
}

static void
activate(struct cg_view *view, bool activate) {
	struct cg_xdg_shell_view *xdg_shell_view = xdg_shell_view_from_view(view);
	wlr_xdg_toplevel_set_activated(xdg_shell_view->xdg_surface, activate);
}

static void
close(struct cg_view *view) {
	struct cg_xdg_shell_view *xdg_shell_view = xdg_shell_view_from_view(view);
	struct wlr_xdg_surface *surface = xdg_shell_view->xdg_surface;
	if(surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
		wlr_xdg_toplevel_send_close(surface);
	}
}

static void
maximize(struct cg_view *view, int width, int height) {
	struct cg_xdg_shell_view *xdg_shell_view = xdg_shell_view_from_view(view);
	wlr_xdg_toplevel_set_size(xdg_shell_view->xdg_surface, width, height);
	enum wlr_edges edges =
	    WLR_EDGE_LEFT | WLR_EDGE_RIGHT | WLR_EDGE_TOP | WLR_EDGE_BOTTOM;
	wlr_xdg_toplevel_set_tiled(xdg_shell_view->xdg_surface, edges);
}

static void
destroy(struct cg_view *view) {
	struct cg_xdg_shell_view *xdg_shell_view = xdg_shell_view_from_view(view);
	free(xdg_shell_view);
}

static void
for_each_surface(struct cg_view *view, wlr_surface_iterator_func_t iterator,
                 void *data) {
	struct cg_xdg_shell_view *xdg_shell_view = xdg_shell_view_from_view(view);
	wlr_xdg_surface_for_each_surface(xdg_shell_view->xdg_surface, iterator,
	                                 data);
}

static void
for_each_popup(struct cg_view *view, wlr_surface_iterator_func_t iterator,
               void *data) {
	struct cg_xdg_shell_view *xdg_shell_view = xdg_shell_view_from_view(view);
	wlr_xdg_surface_for_each_popup_surface(xdg_shell_view->xdg_surface,
	                                       iterator, data);
}

static struct wlr_surface *
wlr_surface_at(const struct cg_view *view, double sx, double sy, double *sub_x,
               double *sub_y) {
	const struct cg_xdg_shell_view *xdg_shell_view =
	    xdg_shell_view_from_const_view(view);
	return wlr_xdg_surface_surface_at(xdg_shell_view->xdg_surface, sx, sy,
	                                  sub_x, sub_y);
}

static void
handle_xdg_shell_surface_request_fullscreen(struct wl_listener *listener,
                                            void *data) {
	struct cg_xdg_shell_view *xdg_shell_view =
	    wl_container_of(listener, xdg_shell_view, request_fullscreen);
	struct wlr_xdg_toplevel_set_fullscreen_event *event = data;
	wlr_xdg_toplevel_set_fullscreen(xdg_shell_view->xdg_surface,
	                                event->fullscreen);
}

static void
handle_xdg_shell_surface_commit(struct wl_listener *listener, void *_data) {
	struct cg_xdg_shell_view *xdg_shell_view =
	    wl_container_of(listener, xdg_shell_view, commit);
	struct cg_view *view = &xdg_shell_view->view;
	view_damage_part(view);
}

static void
handle_xdg_shell_surface_unmap(struct wl_listener *listener, void *_data) {
	struct cg_xdg_shell_view *xdg_shell_view =
	    wl_container_of(listener, xdg_shell_view, unmap);
	struct cg_view *view = &xdg_shell_view->view;

	wl_list_remove(&xdg_shell_view->new_popup.link);
	wl_list_remove(&xdg_shell_view->commit.link);

	view_unmap(view);
}

static void
handle_xdg_shell_surface_map(struct wl_listener *listener, void *_data) {
	struct cg_xdg_shell_view *xdg_shell_view =
	    wl_container_of(listener, xdg_shell_view, map);
	struct cg_view *view = &xdg_shell_view->view;

	xdg_shell_view->commit.notify = handle_xdg_shell_surface_commit;
	wl_signal_add(&xdg_shell_view->xdg_surface->surface->events.commit,
	              &xdg_shell_view->commit);
	xdg_shell_view->new_popup.notify = handle_new_xdg_popup;
	wl_signal_add(&xdg_shell_view->xdg_surface->events.new_popup,
	              &xdg_shell_view->new_popup);

	view_map(view, xdg_shell_view->xdg_surface->surface,
	         view->server->curr_output
	             ->workspaces[view->server->curr_output->curr_workspace]);
	view_damage_whole(view);
}

static void
handle_xdg_shell_surface_destroy(struct wl_listener *listener, void *_data) {
	struct cg_xdg_shell_view *xdg_shell_view =
	    wl_container_of(listener, xdg_shell_view, destroy);
	struct cg_view *view = &xdg_shell_view->view;

	wl_list_remove(&xdg_shell_view->map.link);
	wl_list_remove(&xdg_shell_view->unmap.link);
	wl_list_remove(&xdg_shell_view->destroy.link);
	wl_list_remove(&xdg_shell_view->request_fullscreen.link);
	xdg_shell_view->xdg_surface = NULL;

	view_destroy(view);
}

static const struct cg_view_impl xdg_shell_view_impl = {
    .get_title = get_title,
    .is_primary = is_primary,
    .activate = activate,
    .close = close,
    .maximize = maximize,
    .destroy = destroy,
    .for_each_surface = for_each_surface,
    .for_each_popup = for_each_popup,
    .wlr_surface_at = wlr_surface_at,
};

void
handle_xdg_shell_surface_new(struct wl_listener *listener, void *data) {
	struct cg_server *server =
	    wl_container_of(listener, server, new_xdg_shell_surface);
	struct wlr_xdg_surface *xdg_surface = data;

	if(xdg_surface->role != WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
		return;
	}

	struct cg_xdg_shell_view *xdg_shell_view =
	    calloc(1, sizeof(struct cg_xdg_shell_view));
	if(!xdg_shell_view) {
		wlr_log(WLR_ERROR, "Failed to allocate XDG Shell view");
		return;
	}
	view_init(&xdg_shell_view->view, CG_XDG_SHELL_VIEW, &xdg_shell_view_impl,
	          server);

	xdg_shell_view->xdg_surface = xdg_surface;

	xdg_shell_view->map.notify = handle_xdg_shell_surface_map;
	wl_signal_add(&xdg_surface->events.map, &xdg_shell_view->map);
	xdg_shell_view->unmap.notify = handle_xdg_shell_surface_unmap;
	wl_signal_add(&xdg_surface->events.unmap, &xdg_shell_view->unmap);
	xdg_shell_view->destroy.notify = handle_xdg_shell_surface_destroy;
	wl_signal_add(&xdg_surface->events.destroy, &xdg_shell_view->destroy);
	xdg_shell_view->request_fullscreen.notify =
	    handle_xdg_shell_surface_request_fullscreen;
	wl_signal_add(&xdg_surface->toplevel->events.request_fullscreen,
	              &xdg_shell_view->request_fullscreen);
}

void
handle_xdg_toplevel_decoration(struct wl_listener *listener, void *data) {
	struct cg_server *server =
	    wl_container_of(listener, server, xdg_toplevel_decoration);
	struct wlr_xdg_toplevel_decoration_v1 *wlr_decoration = data;

	struct cg_xdg_decoration *xdg_decoration =
	    calloc(1, sizeof(struct cg_xdg_decoration));
	if(!xdg_decoration) {
		return;
	}

	xdg_decoration->wlr_decoration = wlr_decoration;
	xdg_decoration->server = server;

	xdg_decoration->destroy.notify = xdg_decoration_handle_destroy;
	wl_signal_add(&wlr_decoration->events.destroy, &xdg_decoration->destroy);
	xdg_decoration->request_mode.notify = xdg_decoration_handle_request_mode;
	wl_signal_add(&wlr_decoration->events.request_mode,
	              &xdg_decoration->request_mode);

	xdg_decoration_handle_request_mode(&xdg_decoration->request_mode,
	                                   wlr_decoration);
}
