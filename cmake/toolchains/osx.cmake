SET(CMAKE_OSX_DEPLOYMENT_TARGET 10.13)
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON) #prevent linking errors once app shared
SET(CMAKE_MACOSX_RPATH ON)

list(APPEND CMAKE_PREFIX_PATH 
    "/usr/local/opt"
)

SET(CMAKE_BUILD_TYPE Debug)

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,-no_compact_unwind") #prevent warning