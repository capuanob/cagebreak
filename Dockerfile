# Build Stage
FROM --platform=linux/amd64 ubuntu:22.04 as builder

ARG USERNAME=fuzzer
ARG USER_UID=1000
ARG USER_GID=$USER_UID

## Refuses to run as root, create another user
RUN groupadd --gid $USER_GID $USERNAME \
       && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME

## Install build dependencies.
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y cmake clang libwlroots-dev meson ninja-build libxkbcommon-dev libcairo2-dev libpango1.0-dev


## Add source code to the build stage.
WORKDIR /home/fuzzer/
ADD . /cagebreak
RUN chown -R $USERNAME /cagebreak

USER $USERNAME

ENV XDG_RUNTIME_DIR=/tmp/
ENV WLR_BACKENDS=headless
ENV ASAN_OPTIONS=detect_leaks=0

## Build
WORKDIR /cagebreak
RUN mkdir -p build
RUN CC=clang meson build -Dfuzz=true -Db_sanitize=address,undefined -Db_lundef=false -Db_detect-leaks=0
RUN ninja -C build/
RUN mkdir build/fuzz_corpus
RUN cp examples/config build/fuzz_corpus/

## Package Stage
FROM --platform=linux/amd64 ubuntu:22.04 as packager
RUN apt-get update && \
	DEBIAN_FRONTEND=noninteractive apt-get install -y libwayland-server0 libwayland-client0 libwlroots10 libxkbcommon0 libinput10 libevdev2 libudev1 libpixman-1-0 libpango-1.0-0 libglib2.0-0 libcairo2 libpangocairo-1.0-0 
ENV XDG_RUNTIME_DIR=/tmp/
ENV WLR_BACKENDS=headless
ENV ASAN_OPTIONS=detect_leaks=0

COPY --from=builder /cagebreak/build/fuzz/fuzz-parse /fuzz-parse
COPY --from=builder /cagebreak/build/fuzz/libexecl_override.so /usr/lib
