SET(CMAKE_OSX_DEPLOYMENT_TARGET 10.13)

list(APPEND CMAKE_PREFIX_PATH 
    "/usr/local/opt"
    "/usr/local/opt/qt"
    "/usr/local/opt/glib-networking"
    "/usr/local/opt/openssl@1.1"
)

SET(CMAKE_BUILD_TYPE Debug)