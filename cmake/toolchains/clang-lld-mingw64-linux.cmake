SET(CMAKE_SYSTEM_NAME "Windows")

# where is the target environment
SET(CMAKE_FIND_ROOT_PATH  "$ENV{MINGW64_ROOT}")

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

SET (CMAKE_C_COMPILER             "clang")
SET (CMAKE_C_FLAGS                "-fuse-ld=lld -Wall -Wno-incompatible-ms-struct -Wno-incompatible-pointer-types -Wno-macro-redefined")
SET (CMAKE_C_FLAGS_RELEASE        "-O4 -DNDEBUG")
SET (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")

SET (CMAKE_CXX_COMPILER             "clang++")
SET (CMAKE_CXX_FLAGS                "-fuse-ld=lld -Wall -Wno-incompatible-ms-struct -Wno-incompatible-pointer-types -Wno-macro-redefined")
SET (CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")