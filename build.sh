#!/bin/sh
# configure, build and install the debug and release builds (linux and macos)
# see build-readme.txt for the prerequisites
set -e
cd "$(dirname "$0")"

# use ninja when it is installed, otherwise the cmake default (unix makefiles)
if command -v ninja >/dev/null 2>&1; then
  GENERATOR="-G Ninja"
fi

cmake $GENERATOR -DCMAKE_BUILD_TYPE=Debug -Bbuild/debug
cmake --build build/debug --target install --parallel

cmake $GENERATOR -DCMAKE_BUILD_TYPE=RelWithDebInfo -Bbuild/release
cmake --build build/release --target install --parallel
