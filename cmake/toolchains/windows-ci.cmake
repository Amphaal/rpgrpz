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

#We want Release with debug infos
SET(CMAKE_BUILD_TYPE RelWithDebInfo)

SET (CMAKE_ASM_MASM_COMPILER                "${CMAKE_SOURCE_DIR}/cmake/wrappers/xuasm.sh")
SET (CMAKE_RC_COMPILER                      "${CMAKE_SOURCE_DIR}/cmake/wrappers/xwindres.sh")
SET (CMAKE_C_COMPILER                       "clang")
SET (CMAKE_CXX_COMPILER                     "clang++")
SET (CMAKE_AR                               "llvm-ar")
SET (CMAKE_RANLIB                           "llvm-ranlib")
SET (CMAKE_NM                               "llvm-nm")

SET (CMAKE_C_FLAGS                          "-target x86_64-w64-mingw32 -fuse-ld=lld")
SET (CMAKE_CXX_FLAGS                        ${CMAKE_C_FLAGS})

SET (CMAKE_C_FLAGS_RELWITHDEBINFO           "-O2 -g")
SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO         ${CMAKE_C_FLAGS_RELWITHDEBINFO})

SET (CMAKE_EXE_LINKER_FLAGS_INIT            "-L ${MINGW64_ROOT}/lib -L ${MINGW64_ROOT}/x86_64-w64-mingw32/lib -L ${MINGW64_ROOT}/lib/gcc/x86_64-w64-mingw32/9.3.0")
SET (CMAKE_SHARED_LINKER_FLAGS_INIT         ${CMAKE_EXE_LINKER_FLAGS_INIT})

SET (CMAKE_C_STANDARD_INCLUDE_DIRECTORIES  
    ${MINGW64_ROOT}/include
    ${MINGW64_ROOT}/include/c++/9.3.0
    ${MINGW64_ROOT}/include/c++/9.3.0/x86_64-w64-mingw32
    ${MINGW64_ROOT}/x86_64-w64-mingw32/include
)
SET (CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_STANDARD_INCLUDE_DIRECTORIES})

#MOC
add_executable(Qt5::moc IMPORTED)
set_target_properties(Qt5::moc PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/cmake/wrappers/xmoc.sh
)
#RCC
add_executable(Qt5::rcc IMPORTED)
set_target_properties(Qt5::rcc PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/cmake/wrappers/xrcc.sh
)
#UIC
add_executable(Qt5::uic IMPORTED)
set_target_properties(Qt5::uic PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/cmake/wrappers/xuic.sh
)
#lrelease
add_executable(Qt5::lrelease IMPORTED)
set_target_properties(Qt5::lrelease PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/cmake/wrappers/xlrelease.sh
)
#lupdate
add_executable(Qt5::lupdate IMPORTED)
set_target_properties(Qt5::lupdate PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/cmake/wrappers/xlupdate.sh
)
