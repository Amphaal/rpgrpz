#!/bin/bash
cd ..
mkdir _genRelease
cd _genRelease
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/clang-lld-mingw64-linux.cmake -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
ninja package