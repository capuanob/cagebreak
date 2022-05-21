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
    DEBIAN_FRONTEND=noninteractive apt-get install -y git cmake clang libwlroots-dev meson ninja-build libxkbcommon-dev libcairo2-dev libpango1.0-dev

USER $USERNAME

## Add source code to the build stage.
WORKDIR /home/fuzzer/
RUN git clone https://github.com/capuanob/cagebreak.git

ENV XDG_RUNTIME_DIR=/tmp/
ENV WLR_BACKENDS=headless
ENV ASAN_OPTIONS=detect_leaks=0

## Build
WORKDIR cagebreak
RUN mkdir -p build
RUN CC=clang meson build -Dfuzz=true -Db_sanitize=address,undefined -Db_lundef=false -Db_detect-leaks=0
RUN ninja -C build/
RUN mkdir build/fuzz_corpus
RUN cp examples/config build/fuzz_corpus/



## Set up fuzzing!
ENTRYPOINT []
CMD XDG_RUNTIME_DIR=/tmp/ WLR_BACKENDS=headless ./build/fuzz/fuzz-parse -max_len=50000 -close_fd_mask=3 build/fuzz_corpus/ -detect_leaks=0

