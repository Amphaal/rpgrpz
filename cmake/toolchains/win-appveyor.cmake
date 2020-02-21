SET(CMAKE_BUILD_TYPE "RelWithDebInfo") 
SET(OpenSSL_DIR "C:\\OpenSSL-Win64")
SET(Qt5_DIR "C:\\Qt\\5.14.0\\mingw73_64")
list(APPEND CMAKE_PREFIX_PATH "C:\\gstreamer\\1.0\\x86_64")  

SET (CMAKE_C_COMPILER             "clang")
SET (CMAKE_C_FLAGS                "-fuse-ld=lld -Wall -Wno-incompatible-ms-struct -Wno-incompatible-pointer-types -Wno-macro-redefined")
SET (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")

SET (CMAKE_CXX_COMPILER             "clang++")
SET (CMAKE_CXX_FLAGS                "-fuse-ld=lld -Wall -Wno-incompatible-ms-struct -Wno-incompatible-pointer-types -Wno-macro-redefined")
SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")