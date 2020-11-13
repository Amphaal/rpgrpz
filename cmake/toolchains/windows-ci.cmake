SET(CMAKE_SYSTEM_NAME "Windows")
SET(CMAKE_SYSTEM_VERSION 6.1 CACHE INTERNAL "") #target Windows 7
SET(MINGW ON)

# search for programs in the build host directories
set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

SET(MINGW64_ROOT "/mingw64")

list(APPEND CMAKE_PREFIX_PATH 
    ${MINGW64_ROOT}
)

#We want Release with debug infos
SET(CMAKE_BUILD_TYPE Release)

SET (CMAKE_C_COMPILER                       "clang")
SET (CMAKE_CXX_COMPILER                     "clang++")
SET (CMAKE_AR                               "llvm-ar")
SET (CMAKE_RANLIB                           "llvm-ranlib")
SET (CMAKE_NM                               "llvm-nm")

SET (CMAKE_C_FLAGS                          "-fuse-ld=lld")
SET (CMAKE_CXX_FLAGS                        ${CMAKE_C_FLAGS})

SET (CMAKE_C_COMPILER_TARGET                "x86_64-w64-mingw32")
SET (CMAKE_CXX_COMPILER_TARGET              ${CMAKE_C_COMPILER_TARGET})

SET (GCC_VERSION                            "10.2.0")

SET (CMAKE_EXE_LINKER_FLAGS_INIT            "--sysroot=${MINGW64_ROOT}/${CMAKE_C_COMPILER_TARGET} -L ${MINGW64_ROOT}/lib -L ${MINGW64_ROOT}/lib/gcc/${CMAKE_C_COMPILER_TARGET}/${GCC_VERSION}")
SET (CMAKE_SHARED_LINKER_FLAGS_INIT         ${CMAKE_EXE_LINKER_FLAGS_INIT})

SET (CMAKE_C_STANDARD_INCLUDE_DIRECTORIES  
    ${MINGW64_ROOT}/include
    ${MINGW64_ROOT}/include/c++/${GCC_VERSION}
    ${MINGW64_ROOT}/include/c++/${GCC_VERSION}/${CMAKE_C_COMPILER_TARGET}
    ${MINGW64_ROOT}/${CMAKE_C_COMPILER_TARGET}/include
)
SET (CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_STANDARD_INCLUDE_DIRECTORIES})
