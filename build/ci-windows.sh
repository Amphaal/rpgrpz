#!/bin/sh
cd ..
git clone https://github.com/Amphaal/rpgrpz.git
mkdir _genRelease
cd _genRelease
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/clang-lld-mingw64-win.cmake -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
ninja package