SET(CMAKE_SYSTEM_NAME "Windows")

# search for programs in the build host directories
set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

list(APPEND CMAKE_PREFIX_PATH 
    /mingw64
    /mingw64/x86_64-w64-mingw32
)

#.dll are in bin folder
SET(CMAKE_LIBRARY_PATH /mingw64/bin) #must be defined for peldd

SET(CMAKE_LIBRARY_PATH ${CMAKE_SYSTEM_LIBRARY_PATH}) #must be defined for peldd
SET(CMAKE_BUILD_TYPE RelWithDebInfo)

SET (CMAKE_C_COMPILER             "x86_64-w64-mingw32-clang")
SET (CMAKE_C_FLAGS                "-fuse-ld=lld -Wall -Wno-incompatible-ms-struct -Wno-incompatible-pointer-types -Wno-macro-redefined")
SET (CMAKE_C_FLAGS_DEBUG          "-O0 -g")
SET (CMAKE_C_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
SET (CMAKE_C_FLAGS_RELEASE        "-O4 -DNDEBUG")
SET (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")

SET (CMAKE_CXX_COMPILER             "x86_64-w64-mingw32-clang++")
SET (CMAKE_CXX_FLAGS                "-fuse-ld=lld -Wall -Wno-incompatible-ms-struct -Wno-incompatible-pointer-types -Wno-macro-redefined")
SET (CMAKE_CXX_FLAGS_DEBUG          "-O0 -g")
SET (CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
SET (CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")