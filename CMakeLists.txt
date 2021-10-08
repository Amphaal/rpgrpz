# policies first
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0011 NEW)
    cmake_policy(SET CMP0020 NEW)
    cmake_policy(SET CMP0003 NEW)
endif()

# use ccache
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    message("== Using CCache ! ==")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CCACHE_PROGRAM}")
endif()

# default configuration
list(APPEND CMAKE_MODULE_PATH 
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Qt
)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)

#############
## STARTUP ##
#############

# pre-startup
cmake_minimum_required(VERSION 3.10.2)
message(STATUS "Using toolchain file: [${CMAKE_TOOLCHAIN_FILE}].")

# project setup
project(RPGRPZ
    VERSION 1.0.28
    DESCRIPTION "Simple Tabletop RPG experience"
    HOMEPAGE_URL "https://github.com/Amphaal/rpgrpz"
    LANGUAGES CXX
)
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER) #lowered project name

########################
## VERSION.H HANDLING ##
########################

SET(APP_PUBLISHER "LVWL")
SET(APP_PATCHNOTE_URL ${PROJECT_HOMEPAGE_URL}/releases)
SET(SENTRY_ENDPOINT "https://ecb72ca256e744b990fd4f50aa52ea6f@o333337.ingest.sentry.io/5517405")
SET(APP_MAINTENANCETOOL_PATH "../maintenancetool")

# configure header with basic app informations
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/templates/_version.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h
)

################################
## CPP Compiler Configuration ##
################################

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_C_STANDARD 11)

# Qt
include(XCQt)
SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTORCC ON)
SET(CMAKE_AUTOUIC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON) #for moc auto 

# make sure _WIN32 preprocessor is used even while cross compiling
IF(MINGW)
    add_compile_definitions(_WIN32)
endif()

#################################
## define bundle configuration ##
#################################

# define build type if any
IF(NOT DEFINED APP_BUNDLE_TYPE)
    IF(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        SET(APP_BUNDLE_TYPE WIN32) 
    ELSEIF(APPLE)
        SET(APP_BUNDLE_TYPE MACOSX_BUNDLE)
    endif()
endif()

######################################
## define executables and libraries ##
######################################

# define app executable
add_executable(appExec ${APP_BUNDLE_TYPE})
set_target_properties(appExec PROPERTIES OUTPUT_NAME "${PROJECT_NAME}") #define executable name
target_compile_definitions(appExec PRIVATE $<$<CONFIG:Debug>:_DEBUG>) #define _DEBUG markup if in DEBUG config

# warnings for debug
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_link_options(appExec PRIVATE 
        -Wall 
        -Wextra 
        -Wpedantic 
        -Wno-unused-parameter
    )
endif()

# optimise for release
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set_target_properties(appExec PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE) #enable LTO
endif()

#################
## install app ##
#################

# app
SET(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "App")
install(TARGETS appExec
    COMPONENT "App"
    BUNDLE DESTINATION "."
)

# spdlog
if(MINGW)
    find_file(MINGW_SPDLOG libspdlog.dll REQUIRED)
    install(FILES ${MINGW_SPDLOG}
        TYPE BIN
        COMPONENT "Runtime"
    )
endif()

# runtime
if(MINGW)
    find_file(MINGW_STDLIB libstdc++-6.dll REQUIRED)
    find_file(MINGW_GCC_SEH libgcc_s_seh-1.dll REQUIRED)
    find_file(MINGW_WINPTHREAD libwinpthread-1.dll REQUIRED)
    install(
        FILES 
            ${MINGW_STDLIB}
            ${MINGW_GCC_SEH}
            ${MINGW_WINPTHREAD}
        TYPE BIN
        COMPONENT "Runtime"
    )
    list(APPEND PEUTIL_WHITELIST_EXTENSIONS 
        "libstdc++-6.dll"
        "libgcc_s_seh-1.dll"
        "libwinpthread-1.dll"
    )
endif()

#########################
## ressources handling ##
#########################

# default QT ressources
target_sources(appExec PRIVATE "resources/resources.qrc")

IF(CMAKE_SYSTEM_NAME STREQUAL "Windows")  
    target_sources(appExec PRIVATE "resources/icons/resources.rc") #add Windows ressources (icons)
ELSEIF(APPLE)   
    target_sources(appExec PRIVATE "resources/icons/app.icns") #add MacOS icons in output package
    set_source_files_properties("resources/icons/app.icns"
        PROPERTIES 
        MACOSX_PACKAGE_LOCATION "Resources"
    )
    set_target_properties(appExec PROPERTIES 
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/templates/_Info.plist"
        MACOSX_BUNDLE_ICON_FILE "app.icns"
    ) 
endif()

####################################
# pkgConfig and find_library setup #
####################################

# fetch content handling
include(FetchContent)

# using pkgConfig, set PKG_CONFIG_EXECUTABLE if not found
find_package(PkgConfig REQUIRED)

###########
# OpenSSL #
###########

# find
find_package(OpenSSL REQUIRED)

# link
target_link_libraries(appExec PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
)

# install
if(MINGW)
    find_file(MINGW_OPENSSL_SSL libssl-1_1-x64.dll REQUIRED)
    find_file(MINGW_OPENSSL_CRYPTO libcrypto-1_1-x64.dll REQUIRED)
    install(
        FILES
            ${MINGW_OPENSSL_SSL} 
            ${MINGW_OPENSSL_CRYPTO}
        TYPE BIN
        COMPONENT "OpenSSL"
    )
endif()

########
# QT 6 #
########

find_package(Qt6 REQUIRED COMPONENTS 
    Core 
    Gui 
    Widgets 
    Network 
    Svg 
    OpenGL 
    LinguistTools
)
target_link_libraries(appExec PRIVATE
    Qt6::Core
    Qt6::Gui 
    Qt6::Widgets
    Qt6::Network
    Qt6::Svg
    Qt6::OpenGL
)

# QT LIBRARIES COPY
include(DeployQt)
DeployQt(appExec)

#############
# snowflake #
#############

add_subdirectory(deps/snowflake)
target_link_libraries(appExec PRIVATE snowflake)

###########
# clipper #
###########

add_subdirectory(deps/clipper)
target_link_libraries(appExec PRIVATE clipper)

############################
# autoupdatercore (for Qt) #
############################

add_subdirectory(deps/autoupdatercore)
target_link_libraries(appExec PRIVATE autoupdatercore)

#################
# Sentry-native #
#################

SET(SENTRY_BUILD_SHARED_LIBS ON CACHE INTERNAL "")
add_subdirectory(deps/sentry-native EXCLUDE_FROM_ALL)

# get version
get_directory_property(SENTRY_NATIVE_PROJECT_VERSION
    DIRECTORY deps/sentry-native 
    DEFINITION PROJECT_VERSION
)

# link
target_link_libraries(appExec PRIVATE sentry)

# track version into file
execute_process(
    COMMAND printf "${CMAKE_PROJECT_VERSION}"
    OUTPUT_FILE ${CMAKE_BINARY_DIR}/version
)

# install
install(FILES 
    $<TARGET_FILE:sentry>
    TYPE BIN
    COMPONENT "SentryNative"
)

# add crashpad_handler next to app, and to install process
set_target_properties(crashpad_handler PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY $<TARGET_FILE_DIR:appExec>
)
install(TARGETS crashpad_handler 
    COMPONENT "SentryNative"
)

# generate pdb
IF(MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")  
    
    target_link_options(appExec PRIVATE -Wl,-pdb=) # generate pdb files
    target_compile_options(appExec PRIVATE -gcodeview) #use codeview for pdbs
    
    # add .pdb to install
    install(
        FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<TARGET_FILE_BASE_NAME:appExec>.pdb
        TYPE BIN
        COMPONENT "App"
    )

endif()

################
# NetworkCandy #
################

add_subdirectory(deps/NetworkCandy)
target_link_libraries(appExec PRIVATE nw-candy)

# install 
if(MINGW)
    find_file(MINGW_MINIUPNPC libminiupnpc.dll REQUIRED)
    install(FILES ${MINGW_MINIUPNPC}
        TYPE BIN
        COMPONENT "Runtime"
    )
endif()

#############
# AudioTube #
#############

# add as shared lib
SET(AUDIOTUBE_SHARED ON CACHE INTERNAL "")
add_subdirectory(deps/AudioTube)
get_directory_property(AUDIOTUBE_PROJECT_VERSION
    DIRECTORY deps/AudioTube 
    DEFINITION PROJECT_VERSION
)

# link
target_link_libraries(appExec PRIVATE audiotube)

# install
install(FILES $<TARGET_FILE:audiotube>
    TYPE BIN
    COMPONENT "AudioTube"
)

#########
# Dicer #
#########

add_subdirectory(deps/Dicer)
target_link_libraries(appExec PRIVATE dicer)

####################
# Gstreamer - base #
####################

# find Gstreamer
pkg_check_modules(Gst REQUIRED IMPORTED_TARGET gstreamer-1.0)
message("GStreamer found at : [${Gst_LIBRARY_DIRS}]") # helps if pkfconf uses the wrong search path

# link
target_link_libraries(appExec PRIVATE PkgConfig::Gst)

######################
# Gstreamer - GnuTLS #
######################

SET(GNTLS_LIB_EXT ${CMAKE_SHARED_LIBRARY_SUFFIX})
if(APPLE)
    SET(GNTLS_LIB_EXT ".so")
endif()

# find module
find_file(giognutls_LIB 
    NAMES ${CMAKE_SHARED_LIBRARY_PREFIX}giognutls${GNTLS_LIB_EXT}
    PATH_SUFFIXES "lib/gio/modules"
    REQUIRED
)

# copy file
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gio)
file(COPY ${giognutls_LIB} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gio)

# install
install(
    DIRECTORY $<TARGET_FILE_DIR:appExec>/gio
    TYPE BIN
    COMPONENT "GStreamer"
)

#######################
# Gstreamer - plugins #
#######################

# get plugin shared libraries location
pkg_get_variable(GSTREAMER_PLUGINS_LIB_LOCATION gstreamer-1.0 
    pluginsdir
)

# plugins are in separate folders on MacOS
if(APPLE)
    # since gst-plugins-good has no .pc file associated, just hack plugin folder logic
    string(REPLACE "/gstreamer/" "/gst-plugins-base/" GSTREAMER_PLUGINS_LIB_LOCATION_BASE ${GSTREAMER_PLUGINS_LIB_LOCATION})
    string(REPLACE "/gstreamer/" "/gst-plugins-good/" GSTREAMER_PLUGINS_LIB_LOCATION_GOOD ${GSTREAMER_PLUGINS_LIB_LOCATION})
    LIST(APPEND GSTREAMER_PLUGINS_LIB_LOCATION
        ${GSTREAMER_PLUGINS_LIB_LOCATION_BASE}
        ${GSTREAMER_PLUGINS_LIB_LOCATION_GOOD}
    )
endif()

# define gstreamer plugins to use
    # core
    LIST(APPEND REQUIRED_GST_PLUGINS 
        playback
        autodetect
        coreelements
    )
    # opus stream read
    LIST(APPEND REQUIRED_GST_PLUGINS 
        soup
        typefindfunctions
        matroska
        opus
        volume
        audioconvert
        audioresample
    )

    # windows specific
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        LIST(APPEND REQUIRED_GST_PLUGINS
            waveform
        )
    endif()

# copy gstreamer plugins into output
foreach(_gstPluginLib IN ITEMS ${REQUIRED_GST_PLUGINS})

    # using find_file because find_library is kinda broken with .dll on MinGW
    find_file(${_gstPluginLib}_LIBRARY 
        NAMES ${CMAKE_SHARED_LIBRARY_PREFIX}gst${_gstPluginLib}${CMAKE_SHARED_LIBRARY_SUFFIX}
        PATHS ${GSTREAMER_PLUGINS_LIB_LOCATION}
        NO_DEFAULT_PATH
        REQUIRED
    )

    list(APPEND GST_ALL_USED_PLUGINS
        ${${_gstPluginLib}_LIBRARY} 
    )
    
endforeach()

# install
include(GNUInstallDirs)
install(FILES ${GST_ALL_USED_PLUGINS}
    DESTINATION ${CMAKE_INSTALL_BINDIR}/gst-plugins
    COMPONENT "GStreamer"
)

#####################
# list source files #
#####################

file(GLOB_RECURSE APP_CPP_FILES "src/*.cpp")
file(GLOB_RECURSE APP_HPP_FILES "src/*.hpp")
file(GLOB_RECURSE APP_H_FILES "src/*.h")

# defined as public for tests
target_sources(appExec PUBLIC
    ${APP_CPP_FILES}
    ${APP_HPP_FILES}
)

################
# translations #
################

include(QtTranslation)

# app sources
list(APPEND TS_SOURCE_FILES
    ${APP_CPP_FILES}
    ${APP_HPP_FILES}
    ${APP_H_FILES}
)
list(APPEND TS_FILES
    "src/_i18n/fr.ts"
)
HandleQtTranslation(appExec
    "App"
    "${TS_SOURCE_FILES}"
    ${TS_FILES}
)

# IFW sources
list(APPEND TS_INSTALL_SOURCE_FILES
    "src/_ifw/EndInstallerForm.ui"
)
list(APPEND TS_INSTALL_FILES
    "src/_i18n/EndInstallerForm_fr.ts"
)
HandleQtTranslation(appExec
    OFF
    "${TS_INSTALL_SOURCE_FILES}"
    ${TS_INSTALL_FILES}
)

########################
# Runtime dependencies #
########################

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    include(CMakeDependencies)

    # QT
    DeployPEDependencies(appExec "Qt"
        ${CMAKE_BINARY_DIR}/QtRuntime/**/*.dll
    )

    # GST
    DeployPEDependencies(appExec "GStreamer"
        ${PEDeps_Qt_DIR}/*.dll
        ${giognutls_LIB}
        ${GST_ALL_USED_PLUGINS}
    )
endif()

######################
# add Publish target #
######################

# publish script
include(CMakePublish)
