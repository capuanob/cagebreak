# Cagebreak: A Wayland Tiling Compositor Inspired by Ratpoison

This is Cagebreak, a Wayland tiling compositor.

The goal of this project is to provide a successor to ratpoison for Wayland
users. However, this is no reimplementation of ratpoison.

Should you like to know if a feature will be implemented, open an issue
or get in touch with the development team.

For documentation of Cagebreak, please see
  * the man pages
    * [cagebreak](man/cagebreak.1.md)
    * [configuration](man/cagebreak-config.5.md)
  * also the [FAQ](FAQ.md)

Cagebreak is based on [Cage](https://github.com/Hjdskes/cage), a Wayland kiosk
compositor.

Cagebreak is developed under Arch Linux and uses the libraries
as they are obtained through pacman. However, cagebreak should also work on
other distributions given the proper library versions.

## Installation

If you are using archlinux, just use the PKGBUILDs from the aur:

  * Using [cagebreak](https://aur.archlinux.org/packages/cagebreak), Cagebreak is
    compiled on the target system (since release 1.3.0)
  * Using [cagebreak-bin](https://aur.archlinux.org/packages/cagebreak-bin),
    the pre-built binaries are extracted to
    appropriate paths on the target system (since release 1.3.2)

See [cagebreak-pkgbuild](https://github.com/project-repo/cagebreak-pkgbuild) for details.

### Obtaining Source Code

There are different ways to obtain cagebreak source:

  * git clone (for all releases)
  * download release asset tarballs (starting at release 1.2.1)

### Verifying Source Code

There are corresponding methods of verifying that you obtained the correct code:

  * our git history includes signed tags for releases
  * release assets starting at release 1.2.1 contain a signature for the tarball

### Building Cagebreak

You can build Cagebreak with the [meson](https://mesonbuild.com/) build system. It
requires wayland, wlroots and xkbcommon to be installed. Note that Cagebreak is
developed against the latest tag of wlroots, in order not to constantly chase
breaking changes as soon as they occur.

Simply execute the following steps to build Cagebreak:

```
$ meson build
$ ninja -C build
```

#### Release Build

By default, this builds a debug build. To build a release build, use `meson
build --buildtype=release`.

##### Xwayland Support

Cagebreak comes with compile-time support for XWayland. To enable this,
first make sure that your version of wlroots is compiled with this
option. Then, add `-Dxwayland=true` to the `meson` command above. Note
that you'll need to have the XWayland binary installed on your system
for this to work.

##### Man Pages

Cagebreak has man pages. To use them, make sure that you have `scdoc`
installed. Then, add `-Dman-pages=true` to the `meson` command.

### Running Cagebreak

You can start Cagebreak by running `./build/cagebreak`. If you run it from
within an existing X11 or Wayland session, it will open in a virtual output as
a window in your existing session. If you run it in a TTY, it'll run with the
KMS+DRM backend. For more configuration options, see the man pages.

## Contributing to Cagebreak

Cagebreak is currently developed to fit the needs of its creators. Should you
desire to implement a feature, please let us know in advance by opening
an issue. However, the feature set is intentionally limited (i.e. we removed
support for a desktop background) and will continue to be so in the future.

Nonetheless, don't be intimidated by the (slightly lengthy) release checklist
or any other part of this file. Do what you can, open an issue and we will
collaborate toward a solution.

### Branching Strategy and Versioning

There exists a branch `development` to which all reasonable code
is comitted for final testing.

Once `development` is ready for a release, it is merged into `master` (possibly via
a cherry-picked branch), creating a new release, which is tagged and signed.

All releases are tagged according to [semantic versioning](https://semver.org) guidelines.

In the past, our git history did not perfectly reflect this scheme.

### Releases

The release checklist must be completely fulfilled in one run for a release to
occur.

  * [ ] `git checkout development`
  * [ ] `git pull origin development`
  * [ ] `git push origin development`
  * [ ] `ninja -C build clang-format` makes no changes
  * [ ] New version number determined according to [semantic versioning](https://semver.org) guidelines
  * [ ] Relevant Documentation completed
    * [ ] New features
      * [ ] man pages
        * [ ] man/cagebreak
        * [ ] man/cagebreak-config
        * [ ] Set EPOCH to release day in man generation in meson.build
      * [ ] FAQ.md
      * [ ] Changelog.md for major and minor releases but not patches
    * [ ] Check features for SECURITY.md relevance (changes to socket scope
          for example)
    * [ ] Fixed bugs documented in Bugs.md
      * [ ] Include issue description from github
  * [ ] Testing
    * [ ] Manual testing
    * [ ] Libfuzzer testing
    * [ ] Build version without xwayland support
  * [ ] Version Number
    * [ ] meson.build
    * [ ] git tag
    * [ ] man pages
  * [ ] meson.build reproducible build versions are current archlinux libraries and gcc
  * [ ] Cagebreak is reproducible on multiple machines
  * [ ] Documented reproducible build artefacts
    * [ ] Hashes of the artefacts in README.md
    * [ ] Renamed previous signatures
    * [ ] Created gpg signature of the artefacts
      * [ ] `gpg --detach-sign -u keyid cagebreak`
      * [ ] `gpg --detach-sign -u keyid cagebreak.1`
      * [ ] `gpg --detach-sign -u keyid cagebreak-config.5`
  * [ ] `git add` relevant files
  * [ ] `git commit`
  * [ ] `git push origin development`
  * [ ] Determined commit and tag message (Start with "Release version_number\n\n")
    * [ ] Mentioned fixed Bugs.md issues ("Fixed Issue n")
    * [ ] Mentioned other important changes
  * [ ] `git checkout master`
  * [ ] `git merge --squash development`
  * [ ] `git commit` and insert message
  * [ ] `git tag -u keyid version HEAD` and insert message
  * [ ] `git tag -v version` and check output
  * [ ] `git push --tags origin master`
  * [ ] `git checkout development`
  * [ ] `git merge master`
  * [ ] `git push --tags origin development`
  * [ ] `git archive --prefix=cagebreak/ -o release_version.tar.gz tags/version .`
  * [ ] Create release-artefacts_version.tar.gz
    * [ ] `mkdir release-artefacts_version`
    * [ ] `cp build/cagebreak release-artefacts_version/`
    * [ ] `cp build/cagebreak.sig release-artefacts_version/`
    * [ ] `cp build/cagebreak.1 release-artefacts_version/`
    * [ ] `cp build/cagebreak.1.sig release-artefacts_version/`
    * [ ] `cp build/cagebreak-config.5 release-artefacts_version/`
    * [ ] `cp build/cagebreak-config.5.sig release-artefacts_version/`
    * [ ] `cp LICENSE release-artefacts_version/`
    * [ ] `cp README.md release-artefacts_version/`
    * [ ] `cp SECURITY.md release-artefacts_version/`
    * [ ] `cp FAQ.md release-artefacts_version/`
    * [ ] `export SOURCE_DATE_EPOCH=$(git log -1 --pretty=%ct) ; tar --sort=name --mtime= --owner=0 --group=0 --numeric-owner -czf release-artefacts_version.tar.gz release-artefacts_version`
  * [ ] Checked archive
    * [ ] tar -xvf release_version.tar.gz
    * [ ] cd cagebreak
    * [ ] meson build --buildtype=release
    * [ ] ninja -C build
    * [ ] gpg --verify ../signatures/cagebreak.sig build/cagebreak
    * [ ] cd ..
    * [ ] rm -rf cagebreak
  * [ ] `gpg --detach-sign -u keyid release_version.tar.gz`
  * [ ] `gpg --detach-sign -u keyid release-artefacts_version.tar.gz`
  * [ ] Upload archives and signatures as release assets

### Reproducible Builds

Cagebreak offers reproducible builds given the exact library versions specified
in `meson.build`. Should a version mismatch occur, a warning will be emitted. We have
decided on this compromise to allow flexibility and security. In general we will
adapt the versions to the packages available under Arch Linux at the time of
release.

There are reproducibility issues up to and including release `1.2.0`. See
`Issue 5` in [Bugs.md](Bugs.md).

#### Reproducible Build Instructions

All hashes and signatures are provided for the following build instructions.

```
meson build -Dxwayland=true -Dman-pages=true --buildtype=release
ninja -C build
```

#### Hashes for Builds

For every release after 1.0.5, hashes will be provided.

For every release after 1.7.0, hashes will be provided for man pages too.

See [Hashes.md](Hashes.md)

#### GPG Signatures

For every release after 1.0.5, a GPG signature will be provided in `signatures`.

The current signature is called `cagebreak.sig`, whereas all older signatures
will be named after their release version.

Due to errors in the release process, the releases 1.7.1 and 1.7.2 did not include the release
signatures in the appropriate folder of the git repository. However, signatures were provided
as release-artefacts at the time of release. The signatures were introduced into the
repository with 1.7.3. The integrity of cagebreak is still the same because the signatures were
provided as release-artefacts (which were themselves signed) and the hashes in README.md
are part of a signed release tag.

#### Signing Keys

All releases are signed by at least one of the following collection of
keys.

  * E79F6D9E113529F4B1FFE4D5C4F974D70CEC2C5B
  * 4739D329C9187A1C2795C20A02ABFDEC3A40545F
  * 7535AB89220A5C15A728B75F74104CC7DCA5D7A8
  * 827BC2320D535AEAD0540E6E2E66F65D99761A6F
  * A88D7431E5BAAD0B6EAE550AC8D61D8BD4FA3C46
  * 8F872885968EB8C589A32E9539ACC012896D450F
  * 896B92AF738C974E0065BF42F2576BD366156BB9
  * AA927AFD50AF7C6810E69FE8274F2C605359E31B
  * BE2DED372287BC4EB2213E13A0C743848A638955
  * 0F3476E4B2404F95EC41600683D5810F7911B020

Should we at any point retire a key, we will only replace it with keys signed
by at least one of the above collection.

We registered project-repo.co and added mail addresses after release `1.3.0`.

We now have a mail address and its key is signed by signing keys. See Security
Bugs for details.

The full public keys can be found in `keys/` along with any revocation certificates.

### GCC and -fanalyzer

Cagebreak should compile with any reasonably new gcc or clang. Consider
a gcc version of at least [10.1](https://gcc.gnu.org/gcc-10/changes.html) if
you want to get the benefit of the brand-new
[-fanalyzer](https://gcc.gnu.org/onlinedocs/gcc/Static-Analyzer-Options.html)
flag. However, this new flag sometimes produces false-postives and we
selectively disable warnings for affected code segments as described below.

Meson is configured to set `CG_HAS_FANALYZE` if `-fanalyzer` is available.
Therefore, to maintain portability, false-positive fanalyzer warnings are to be
disabled using the following syntax:

```
#if CG_HAS_FANALYZE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "WARNING OPTION"
#endif
```
and after

```
#if CG_HAS_FANALYZE
#pragma GCC diagnostic pop
#endif
```

### Fuzzing

Along with the project source code, a fuzzing framework based on `libfuzzer` is
supplied. This allows for the testing of the parsing code responsible for reading
the `cagebreak` configuration file. When `libfuzzer` is available (please
use the `clang` compiler to enable it), building the fuzz-testing software can
be enabled by passing `-Dfuzz=true` to meson. This generates a `build/fuzz/fuzz-parse`
binary according to the `libfuzzer` specifications. Further documentation on
how to run this binary can be found [here](https://llvm.org/docs/LibFuzzer.html).

Here is an example workflow:

```
rm -rf build
CC=clang meson build -Dfuzz=true -Db_sanitize=address,undefined -Db_lundef=false
ninja -C build/
mkdir build/fuzz_corpus
cp examples/config build/fuzz_corpus/
WLR_BACKENDS=headless ./build/fuzz/fuzz-parse -jobs=12 -max_len=50000 -close_fd_mask=3 build/fuzz_corpus/
```

You may want to tweak `-jobs` or add other options depending on your own setup.
We have found code path discovery to increase rapidly when the fuzzer is supplied
with an initial config file. We are working on improving our fuzzing coverage to
find bugs in other areas of the code.

#### Caveat

Currently, there are memory leaks which do not seem to stem from our code but rather
the code of wl-roots or some other library we depend on. We are working on the problem.
In the meantime, add `-Db_detect-leaks=0` to the meson command to exclude memory leaks.

## Bugs

For any bug, please [create an
issue](https://github.com/project-repo/cagebreak/issues/new) on
[GitHub](https://github.com/project-rep/cagebreak).

Fixed bugs are to be assigned a number and summarized inside Bugs.md for future reference
independent of github, in case this service is unavailable.

Mail contact: `cagebreak @ project-repo . co`

GPG Fingerprints:

  * B15B92642760E11FE002DE168708D42451A94AB5
  * F8DD9F8DD12B85A28F5827C4678E34D2E753AA3C
  * 3ACEA46CCECD59E4C8222F791CBEB493681E8693

See [SECURITY.md](SECURITY.md) for details.

## Changelog

See [Changelog.md](Changelog.md)

## Contributors

  * Aisha Tammy
    * [make man pages optional](https://github.com/project-repo/cagebreak/pull/4), released
      in 1.6.0 with slight modifications

## License

Please see [LICENSE](https://github.com/project-repo/cagebreak/blob/master/LICENSE)
