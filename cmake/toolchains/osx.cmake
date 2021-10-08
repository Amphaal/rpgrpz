SET(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)

list(APPEND CMAKE_PREFIX_PATH 
    "/usr/local/opt"
    "/usr/local/opt/qt"
    "/usr/local/opt/glib-networking"
    "/usr/local/opt/openssl@1.1"
)

remove_definitions(-D_LIBCPP_ENABLE_CXX20_REMOVED_ALLOCATOR_MEMBERS)

SET(CMAKE_BUILD_TYPE Debug)