SET(CMAKE_SYSTEM_NAME "Windows")
SET(CMAKE_SYSTEM_VERSION 6.1 CACHE INTERNAL "") #target Windows 7

# search for programs in the build host directories
set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

list(APPEND CMAKE_PREFIX_PATH
    /mingw64/x86_64-w64-mingw32 
    /mingw64
)

#.dll are in bin folder
SET(MINGW_DLL_PATH /mingw64/bin)

#We want Release with debug infos
SET(CMAKE_BUILD_TYPE RelWithDebInfo)

SET (CMAKE_C_COMPILER             "x86_64-w64-mingw32-clang")
SET (CMAKE_C_FLAGS                "-fuse-ld=lld")
SET (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")

SET (CMAKE_CXX_COMPILER             "x86_64-w64-mingw32-clang++")
SET (CMAKE_CXX_FLAGS                "-fuse-ld=lld")
SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")