SET(CMAKE_SYSTEM_NAME "Windows")
SET(CMAKE_SYSTEM_VERSION 6.1 CACHE INTERNAL "") #target Windows 7

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

#.dll are in bin folder
SET(MINGW_DLL_PATH ${MINGW64_ROOT}/bin)

SET(XCOMPIL_STANDARD_INCLUDE_DIRECTORIES 
    ${MINGW64_ROOT}/include
    ${MINGW64_ROOT}/x86_64-w64-mingw32/include
)
SET(XCOMPIL_STANDARD_LIBRARIES
    ${MINGW64_ROOT}/lib
    ${MINGW64_ROOT}/x86_64-w64-mingw32/lib
)

#We want Release with debug infos
SET(CMAKE_BUILD_TYPE RelWithDebInfo)

SET (CMAKE_ASM_MASM_COMPILER                "${CMAKE_SOURCE_DIR}/cmake/wrappers/xuasm.sh")
SET (CMAKE_RC_COMPILER                      "${CMAKE_SOURCE_DIR}/cmake/wrappers/xwindres.sh")

SET (CMAKE_C_COMPILER                       "clang")
SET (CMAKE_C_LINKER_FLAGS_INIT              "-fuse-ld=lld")
SET (CMAKE_C_FLAGS                          "-target x86_64-w64-mingw32 -nostdinc++")
SET (CMAKE_C_FLAGS_RELWITHDEBINFO           "-O2 -g")
SET (CMAKE_C_STANDARD_INCLUDE_DIRECTORIES   ${XCOMPIL_STANDARD_INCLUDE_DIRECTORIES})
SET (CMAKE_C_STANDARD_LIBRARIES             ${XCOMPIL_STANDARD_LIBRARIES})

SET (CMAKE_CXX_COMPILER                     "clang++")
SET (CMAKE_CXX_LINKER_FLAGS_INIT            "-fuse-ld=lld")
SET (CMAKE_CXX_FLAGS                        "-target x86_64-w64-mingw32 -nostdinc++")
SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO         "-O2 -g")
SET (CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${XCOMPIL_STANDARD_INCLUDE_DIRECTORIES})
SET (CMAKE_CXX_STANDARD_LIBRARIES           ${XCOMPIL_STANDARD_LIBRARIES})