#policies first
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0011 NEW)
    cmake_policy(SET CMP0020 NEW)
    cmake_policy(SET CMP0003 NEW)
endif()

#default configuration
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#check env variable presence
include(CheckEnvVariables)

#############
## STARTUP ##
#############

#pre-startup
cmake_minimum_required(VERSION 3.15.6)
message(STATUS "Using toolchain file: [${CMAKE_TOOLCHAIN_FILE}].")

#project setup
project(RPGRPZ
    VERSION 0.20.5
    DESCRIPTION "Simple Tabletop RPG experience"
    HOMEPAGE_URL "https://github.com/Amphaal/rpgrpz"
    LANGUAGES C CXX
)
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER) #lowered project name

########################
## VERSION.H HANDLING ##
########################

SET(APP_PUBLISHER "LVWL")
SET(APP_PATCHNOTE_URL "https://github.com/Amphaal/rpgrpz/releases")
SET(SENTRY_ENDPOINT "http://3a471bcc32d94c628a191d3d7d2eabff@zonme.to2x.ovh:9000/2")

#config file to source code
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/templates/_version.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h
)

################################
## CPP Compiler Configuration ##
################################

SET(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Qt
SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON) #for moc auto 

##########################
## Bundle configuration ##
##########################

#default source contains
SET(SOURCES
    resources/resources.qrc
)

IF(WIN32)
    #set bundle type
    SET(BUNDLE_TYPE "WIN32") 
    
    #allows Debug cout/cerr console... from LLDB/GDB
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        SET(BUNDLE_TYPE "") 
    endif()
    
    #add Windows ressources
    list(APPEND SOURCES 
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/resources.rc
    )
endif()

IF(APPLE)
    #SET(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
    SET(BUNDLE_TYPE MACOSX_BUNDLE) #set bundle type
    SET(MACOSX_BUNDLE_ICON_FILE "app.icns") #define icon, parsed into _Info.plist

    #locate icon in package
    set_source_files_properties(
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/app.icns 
        PROPERTIES MACOSX_PACKAGE_LOCATION "Resources"
    ) 
endif()

####################################
# pkgConfig and find_library setup #
####################################

IF(WIN32)
    list(APPEND CMAKE_PREFIX_PATH "$ENV{MSYS2_ROOT}/mingw64/x86_64-w64-mingw32")
    list(APPEND CMAKE_LIBRARY_PATH "$ENV{MSYS2_ROOT}/mingw64/bin")
endif()

#############
# miniupnpc #
#############

#dev deps and linkage
find_package(miniupnpc REQUIRED)
list(APPEND EXT_LIBS ${MINIUPNP_LIBRARY})
include_directories(${MINIUPNP_INCLUDE_DIR})

############# 
# Gstreamer #
#############

include(GStreamerHelper)
FindGStreamer()

list(APPEND EXT_LIBS
    PkgConfig::Gst
    PkgConfig::GstPluginsBase
)

########
# QT 5 #
########

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Network Svg OpenGL LinguistTools)
list(APPEND EXT_LIBS
    Qt5::Core
    Qt5::Gui 
    Qt5::Widgets
    Qt5::Network
    Qt5::Svg
    Qt5::OpenGL
)

###########
# OpenSSL #
###########

find_package(openssl REQUIRED)
list(APPEND EXT_LIBS
    OpenSSL::SSL
    OpenSSL::Crypto
    ws2_32
)

#################
# Sentry-native #
#################

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/_req/sentry-native)
include_directories(${Sentry-Native_SOURCE_DIR}/include)
list(APPEND EXT_LIBS
    sentry
)

#####################
# list source files #
#####################

file(GLOB_RECURSE CPP_FILES src/*.cpp)
file(GLOB_RECURSE HPP_FILES src/*.hpp)

list(APPEND SOURCES
    ${CPP_FILES}
    ${HPP_FILES}
)

################
# translations #
################

SET(QT_TRANSLATIONS_SCAN_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src)
list(APPEND QT_TRANSLATIONS_TS_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_i18n/fr.ts
)

#translate at target generation
qt5_create_translation(QM_FILES 
    ${QT_TRANSLATIONS_SCAN_DIRECTORY}
    ${QT_TRANSLATIONS_TS_FILES}
)

#########################
# Executable Generation #
#########################

# Create executable
add_executable(app ${BUNDLE_TYPE} ${SOURCES} ${QM_FILES})
set_target_properties(app PROPERTIES OUTPUT_NAME ${PROJECT_NAME}) #define executable name 
target_compile_definitions(app PRIVATE $<$<CONFIG:Debug>:_DEBUG>) #define _DEBUG markup if in DEBUG config
target_link_libraries(app ${EXT_LIBS}) #link libraries

if(APPLE)
    #add highdpi support / menu behavior via plist template (MACOS)
    set_target_properties(app PROPERTIES 
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/templates/_Info.plist
    ) 
endif()

#####################
# QT LIBRARIES COPY #
#####################

if(WIN32)
    include(Windeployqt)
    windeployqt(app)
endif()

if(APPLE)
    include(Macdeployqt)
    macdeployqt(app)
endif()

#####################
# TRANSLATIONS COPY #
#####################

add_custom_command(TARGET app POST_BUILD  
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${QM_FILES} $<TARGET_FILE_DIR:app>/translations
)

install(
    FILES ${QM_FILES}
    DESTINATION ./translations/
    COMPONENT appTranslations
)

##################
# miniupnpc COPY #
##################

#copy lib into a target
find_library(miniupnpc_BIN_LIBRARY NAMES "miniupnpc" PATHS "${CMAKE_LIBRARY_PATH}" NO_DEFAULT_PATH)
add_custom_command(TARGET app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${miniupnpc_BIN_LIBRARY} $<TARGET_FILE_DIR:app>
)

#install
install(
    FILES ${miniupnpc_BIN_LIBRARY} 
    DESTINATION . 
    COMPONENT miniupnpc
)

##################
# OpenSSL COPY #
##################

find_library(OPENSSL_CPYPTO_BIN_LIBRARY NAMES "crypto-1_1-x64" PATHS "${CMAKE_LIBRARY_PATH}" NO_DEFAULT_PATH)
find_library(OPENSSL_SSL_BIN_LIBRARY NAMES "ssl-1_1-x64" PATHS "${CMAKE_LIBRARY_PATH}" NO_DEFAULT_PATH)

#copy lib into a target
add_custom_command(TARGET app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OPENSSL_CPYPTO_BIN_LIBRARY} $<TARGET_FILE_DIR:app>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OPENSSL_SSL_BIN_LIBRARY} $<TARGET_FILE_DIR:app>
)

#install
install(
    FILES ${OPENSSL_CPYPTO_BIN_LIBRARY} ${OPENSSL_SSL_BIN_LIBRARY}
    DESTINATION . 
    COMPONENT openssl
)

# ######################
# # GST LIBRARIES COPY #
# ######################

if(APPLE)
    #missing deps for gstreamer
    list(APPEND GST_SLIBS
        avcodec
        avfilter
        avformat
        avutil
        bz2
        gmp
        gnutls
        hogweed
        intl
        nettle
        soup-2.4
        swresample
        tasn1
        xml2
    )
endif()

if(WIN32)
    #missing deps for gstreamer
    list(APPEND GST_SLIBS
        avcodec-58
        avfilter-7
        avformat-58
        avutil-56
        libgmp-10
        libgnutls-30
        libhogweed-5
        libnettle-7
        libsoup-2.4-1
        libtasn1-6
        libwinpthread-1
        libxml2-2
        swresample-3
    )
endif()

#import all missing GSTREAMER libs
foreach(_gstSLib IN ITEMS ${GST_SLIBS})
    find_library(${_gstSLib}_LIBRARY NAMES "${_gstSLib}" PATHS "${CMAKE_LIBRARY_PATH}" NO_DEFAULT_PATH)
    add_custom_command(TARGET app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
            ${${_gstSLib}_LIBRARY}
            $<TARGET_FILE_DIR:app>
    )
endforeach()


#copy gstreamer plugins  
    #LIST(APPEND _gstreamer_plugins all)
        
    #core
    LIST(APPEND _gstreamer_plugins 
        gstplayback
        gstautodetect
        gstcoreelements
    )

    #opus stream read
    LIST(APPEND _gstreamer_plugins 
        gstwaveform
        gstsoup
        gsttypefindfunctions
        gstmatroska
        gstopus
        gstvolume
        gstaudioconvert
        gstaudioresample
    )

DeployGStreamer(app "${_gstreamer_plugins}")

#platform specifics
IF(WIN32)

    find_library(giognutls_LIBRARY NAMES "giognutls" PATHS "$ENV{MSYS2_ROOT}/mingw64/lib/gio/modules" NO_DEFAULT_PATH)
    get_filename_component(giognutls_NAME ${giognutls_LIBRARY} NAME)

    add_custom_command(TARGET app       
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
            ${giognutls_LIBRARY} 
            $<TARGET_FILE_DIR:app>/gio/${giognutls_NAME}
    )

    install(
        DIRECTORY ./gio
        DESTINATION .
        COMPONENT gst-gio
    )
    
endif()

################
# missing libs #
################

#scan for missing lib dependencies, then fetch and copy them
if(WIN32)
    STRING(REGEX REPLACE "/" "\\\\\\\\\\\\" ESCAPED_CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH})
    add_custom_command(TARGET app POST_BUILD
        COMMAND ntldd -R -D ${CMAKE_LIBRARY_PATH} $<TARGET_FILE:app> | grep -o '${ESCAPED_CMAKE_LIBRARY_PATH}\\\\\\\\.*\\s' | sed 's/\\\\\\\\/\\\\//g' | xargs cp -n -t ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
endif()

######################
# add Publish target #
######################

include(CMakePublish)

#################
# Debug helpers #
#################

#include(PrintAllVariables)

#include(PrintTargetProperties)
#print_target_properties(PkgConfig::GstPluginsBase)