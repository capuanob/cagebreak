# Build Stage
FROM --platform=linux/amd64 ubuntu:22.04 as builder

## Install build dependencies.
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y git cmake clang libwlroots-dev meson ninja-build libxkbcommon-dev libcairo2-dev libpango1.0-dev

## Add source code to the build stage.
RUN git clone https://github.com/capuanob/cagebreak.git

## Build
WORKDIR /cagebreak
RUN mkdir -p build
RUN CC=clang meson build -Dfuzz=true -Db_sanitize=address,undefined -Db_lundef=false -Db_detect_leaks=0
RUN ninja -C build/
RUN mkdir build/fuzz_corpus
RUN cp examples/config build/fuzz_corpus/

# Package Stage
#FROM --platform=linux/amd64 ubuntu:20.04
#COPY --from=builder /dimsum/build/dimsum-fuzz /

