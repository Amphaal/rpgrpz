#policies first
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0011 NEW)
    cmake_policy(SET CMP0020 NEW)
    cmake_policy(SET CMP0003 NEW)
endif()

#toolchain
message(STATUS "Using toolchain file: [${CMAKE_TOOLCHAIN_FILE}].")

#project setup
project(RPGRPZ)
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)
cmake_minimum_required(VERSION 3.15.6)

#version
SET(CMAKE_PROJECT_VERSION_MAJOR 0)
SET(CMAKE_PROJECT_VERSION_MINOR 20)
SET(CMAKE_PROJECT_VERSION_PATCH 5)

#default configuration
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

########################
## VERSION.H HANDLING ##
########################

#app dest path
SET(APP_PUBLISHER "LVWL")
SET(APP_DEFAULT_URL "https://github.com/Amphaal/rpgrpz")
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

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13)
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON) #prevent linking errors once app shared
    SET(CMAKE_MACOSX_RPATH ON)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-no_compact_unwind") #prevent warning
endif (APPLE)


#########################
## Libraries specifics ##
#########################

#Qt
SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON) #for moc auto 

##########################
## Bundle configuration ##
##########################

IF(WIN32)
    SET(BUNDLE_TYPE WIN32) #set bundle type
    SET(APP_RC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/resources/resources.rc) #add icon
endif()

IF(APPLE)
    #SET(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
    SET(BUNDLE_TYPE MACOSX_BUNDLE) #set bundle type
    SET(MACOSX_BUNDLE_ICON_FILE "app.icns") #define icon
    SET(APP_ICO ${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/app.icns) #add icon
    set_source_files_properties(${APP_ICO} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources") #locate icon in package
endif()

#default source contains
SET(SOURCES
    ${APP_RC_FILE}
    resources/resources.qrc
)

#############
# miniupnpc #
#############

find_package(miniupnpc REQUIRED)
list(APPEND EXT_LIBS ${MINIUPNP_LIBRARY})
include_directories(${MINIUPNP_INCLUDE_DIR})

############# 
# Gstreamer #
#############

#pkgConfig setup
IF(WIN32)
    list(APPEND CMAKE_PREFIX_PATH "C:/msys64/mingw64/x86_64-w64-mingw32")
    list(APPEND CMAKE_LIBRARY_PATH "C:/msys64/mingw64/bin")
endif()

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

#link the rest !
target_link_libraries(app ${EXT_LIBS})
set_target_properties(app PROPERTIES 
    OUTPUT_NAME ${PROJECT_NAME}
) 

if(APPLE)
    #add highdpi support / menu behavior via plist template (MACOS)
    set_target_properties(app PROPERTIES 
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/templates/_Info.plist
    ) 
endif()

#####################
# QT LIBRARIES COPY #
#####################

#using deployqt to import all the QT ${CMAKE_SHARED_LIBRARY_SUFFIX} requirements based on used components
include(deployqt)
deployqt(app)

###############
# OTHERS COPY #
###############

#translations
add_custom_command(TARGET app POST_BUILD  
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${QM_FILES} $<TARGET_FILE_DIR:app>/translations
)

#miniUPnPc
find_library(miniupnpc_LIBRARY miniupnpc)
add_custom_command(TARGET app POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${miniupnpc_LIBRARY} $<TARGET_FILE_DIR:app>
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

#copy gstreamer ${CMAKE_SHARED_LIBRARY_SUFFIX}s
IF(NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gst-plugins)
    #LIST(APPEND _gstreamer_plugins all)
    LIST(APPEND _gstreamer_plugins 
        gstaudioconvert
        gstautodetect
        gstaudioresample
        gstcoreelements
        gstlibav
        gstmatroska
        gstplayback
        gstsoup
        gsttypefindfunctions
        gstvolume
        gstwasapi
    )
    DeployGStreamer(app "${_gstreamer_plugins}")
endif()

################
# missing libs #
################

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