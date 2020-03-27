#policies first
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0011 NEW)
    cmake_policy(SET CMP0020 NEW)
    cmake_policy(SET CMP0003 NEW)
endif()

#use ccache
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CCACHE_PROGRAM}")
endif()

#default configuration
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#############
## STARTUP ##
#############

#pre-startup
cmake_minimum_required(VERSION 3.17.0)
message(STATUS "Using toolchain file: [${CMAKE_TOOLCHAIN_FILE}].")

#project setup
project(RPGRPZ
    VERSION 0.20.5
    DESCRIPTION "Simple Tabletop RPG experience"
    HOMEPAGE_URL "https://github.com/Amphaal/rpgrpz"
    LANGUAGES C CXX
)
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER) #lowered project name
SET(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME ${PROJECT_NAME_LOWER})

########################
## VERSION.H HANDLING ##
########################

SET(APP_PUBLISHER "LVWL")
SET(APP_PATCHNOTE_URL "https://github.com/Amphaal/rpgrpz/releases")
SET(SENTRY_ENDPOINT "http://3a471bcc32d94c628a191d3d7d2eabff@zonme.to2x.ovh:9000/2")

#configure header with basic app informations
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/templates/_version.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h
)

############
# XCompile #
############

SET(CROSSCOMPILING_EMULATOR "/usr/bin/wine")
SET(CMAKE_CROSSCOMPILING_EMULATOR "/usr/bin/wine")

#if XCompiling Windows, use emulator
# IF(CMAKE_CROSSCOMPILING AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
#     #MOC
#     set(AUTOMOC_EXECUTABLE "/usr/bin/${CMAKE_CROSSCOMPILING_EMULATOR}")
#     set(CMAKE_AUTOMOC_MOC_OPTIONS ${MINGW64_ROOT}/bin/moc.exe)
#     add_executable(Qt5::moc IMPORTED)
#     set_target_properties(Qt5::moc PROPERTIES
#         IMPORTED_LOCATION ${AUTOMOC_EXECUTABLE}
#     )
# endif()

################################
## CPP Compiler Configuration ##
################################

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

#Qt
SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTORCC ON)
SET(CMAKE_AUTOUIC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON) #for moc auto 

#make sure _WIN32 preprocessor is used even while cross compiling
IF(MINGW)
    add_compile_definitions(_WIN32)
endif()

#################################
## define bundle configuration ##
#################################

#define build type if any
IF(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        SET(APP_BUNDLE_TYPE "") #allows Debug cout/cerr console... from LLDB/GDB
    else()
        SET(APP_BUNDLE_TYPE "WIN32") 
    endif()
ELSEIF(APPLE)

    #SET(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
    SET(APP_BUNDLE_TYPE MACOSX_BUNDLE) #set bundle type

endif()

######################################
## define executables and libraries ##
######################################

#define app executable
add_executable(appExec ${APP_BUNDLE_TYPE})
set_target_properties(appExec PROPERTIES OUTPUT_NAME "${PROJECT_NAME}") #define executable name
target_compile_definitions(appExec PRIVATE $<$<CONFIG:Debug>:_DEBUG>) #define _DEBUG markup if in DEBUG config

#generate pdb
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    IF(MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")  
        target_link_options(appExec PRIVATE -Wl,-pdb=) # generate pdb files
    endif()
endif()

#warnings for debug
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    IF()  
        target_link_options(appExec PRIVATE 
            -Wall 
            -Wextra 
            -Wpedantic 
            -Wno-unused-parameter
        )
    endif()
endif()

#optimise for release
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set_target_properties(appExec PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE) #enable LTO
endif()

#########################
## ressources handling ##
#########################

#default QT ressources
target_sources(appExec PRIVATE "resources/resources.qrc")

IF(CMAKE_SYSTEM_NAME STREQUAL "Windows")  
    target_sources(appExec PRIVATE "resources/resources.rc") #add Windows ressources (icons)
ELSEIF(APPLE)   
    
    target_sources(appExec PRIVATE "resources/icons/app.icns") #add MacOS icons in output package

    #add highdpi support / menu behavior via plist template
    set_target_properties(appExec PROPERTIES 
        MACOSX_BUNDLE_INFO_PLIST "templates/_Info.plist"
        MACOSX_BUNDLE_ICON_FILE "app.icns"
        MACOSX_PACKAGE_LOCATION "Resources"
    ) 
endif()

####################################
# pkgConfig and find_library setup #
####################################

#fetch content handling
include(FetchContent)

#using pkgConfig, set PKG_CONFIG_EXECUTABLE if not found
find_package(PkgConfig REQUIRED)

###########
# OpenSSL #
###########

#find
find_package(OpenSSL REQUIRED)

#link
target_link_libraries(appExec
    OpenSSL::SSL
    OpenSSL::Crypto
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(appExec ws2_32) #WSA functions !
endif()

if(MINGW)
    #find libs
    find_file(OPENSSL_CPYPTO_DLL 
        NAMES ${CMAKE_SHARED_LIBRARY_PREFIX}crypto-1_1-x64${CMAKE_SHARED_LIBRARY_SUFFIX}
        PATH_SUFFIXES "bin" 
    ) 
    find_file(OPENSSL_SSL_DLL 
        NAMES ${CMAKE_SHARED_LIBRARY_PREFIX}ssl-1_1-x64${CMAKE_SHARED_LIBRARY_SUFFIX}
        PATH_SUFFIXES "bin" 
    ) 

    IF(NOT OPENSSL_CPYPTO_DLL OR NOT OPENSSL_SSL_DLL)
        message(FATAL_ERROR "Openssl .dll NOT FOUND !")
    else()
        message("Openssl .dll Found ! [${OPENSSL_CPYPTO_DLL} ${OPENSSL_SSL_DLL}]")
    endif()

    #copy lib into a target
    add_custom_command(TARGET appExec 
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OPENSSL_CPYPTO_DLL} $<TARGET_FILE_DIR:appExec>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OPENSSL_SSL_DLL} $<TARGET_FILE_DIR:appExec>
    )
endif()

########
# QT 5 #
########

find_package(Qt5 REQUIRED COMPONENTS 
    Core 
    Gui 
    Widgets 
    Network 
    Svg 
    OpenGL 
    LinguistTools
)
target_link_libraries(appExec
    Qt5::Core
    Qt5::Gui 
    Qt5::Widgets
    Qt5::Network
    Qt5::Svg
    Qt5::OpenGL
)

# QT LIBRARIES COPY
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    include(Windeployqt)
    windeployqt(appExec)
elseif(APPLE)
    include(Macdeployqt)
    macdeployqt(appExec)
endif()

#missing libs
IF(MINGW)
    #for icons handling
    find_file(libjpeg_LIBRARY 
        NAMES ${CMAKE_SHARED_LIBRARY_PREFIX}jpeg-8${CMAKE_SHARED_LIBRARY_SUFFIX}
        PATH_SUFFIXES "bin" 
    ) 

    IF(NOT libjpeg_LIBRARY)
        message(FATAL_ERROR "libjpeg .dll NOT FOUND !")
    else()
        message("libjpeg_LIBRARY found ! [${libjpeg_LIBRARY}]")
    endif()

    add_custom_command(TARGET appExec 
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${libjpeg_LIBRARY} $<TARGET_FILE_DIR:appExec>
    )
endif()

###########
# pe-util #
###########

#pe-util is required to find all .dll dependencies
IF(MINGW)
    include(ExternalProject)
    ExternalProject_Add(peldd
        GIT_REPOSITORY  "https://github.com/Amphaal/pe-util"
        INSTALL_COMMAND ""
    )
endif()

#################
# Sentry-native #
#################

add_subdirectory("external/sentry-native")
target_compile_options(sentry PUBLIC -Wno-unused-parameter -Wno-format-pedantic) #temporary fix
target_include_directories(appExec PRIVATE "${Sentry-Native_SOURCE_DIR}/include")
target_link_libraries(appExec sentry)

#############
# AudioTube #
#############

add_subdirectory("external/audiotube")
target_link_libraries(appExec audiotube)

############# 
# Gstreamer #
#############

#find Gstreamer
pkg_check_modules(Gst REQUIRED IMPORTED_TARGET
    gstreamer-1.0
    gstreamer-controller-1.0
    gstreamer-net-1.0
    gstreamer-audio-1.0
)

#since mms-bitfields flag from glib is PUBLIC and throws a bunch of warnings...
IF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(PkgConfig::Gst INTERFACE "-Wno-incompatible-ms-struct")
ENDIF()

#link
target_link_libraries(appExec     
    PkgConfig::Gst
)

IF(MINGW OR APPLE)
    ####define gstreamer plugins to use
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

    #create plugin directory
    add_custom_command(TARGET appExec 
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:appExec>/gst-plugins
    )

    #get plugin shared libraries location
    pkg_get_variable(GSTREAMER_PLUGINS_DLL_LOCATION gstreamer-1.0 
        pluginsdir
    )

    message("Searching for gst plugins in [${GSTREAMER_PLUGINS_DLL_LOCATION}]...")

    #copy gstreamer plugins into output
    foreach(_gstPluginLib IN ITEMS ${_gstreamer_plugins})

        #using find_file because find_library is kinda broken with .dll on MinGW
        find_file(${_gstPluginLib}_LIBRARY 
            NAMES ${CMAKE_SHARED_LIBRARY_PREFIX}${_gstPluginLib}${CMAKE_SHARED_LIBRARY_SUFFIX}
            PATHS ${GSTREAMER_PLUGINS_DLL_LOCATION}
            NO_DEFAULT_PATH
        )

        IF(NOT ${_gstPluginLib}_LIBRARY)
            message(FATAL_ERROR "${_gstPluginLib}_LIBRARY NOT FOUND !")
        else()
            message("${_gstPluginLib}_LIBRARY Found ! [${${_gstPluginLib}_LIBRARY}]")
        endif()

        add_custom_command(TARGET appExec 
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${${_gstPluginLib}_LIBRARY} $<TARGET_FILE_DIR:appExec>/gst-plugins
        )

    endforeach()
endif()

# ######################
# # GST LIBRARIES COPY #
# ######################

#platform specifics missing dependency
IF(MINGW)

    #broken find_library...
    find_file(giognutls_DLL 
        NAMES ${CMAKE_SHARED_LIBRARY_PREFIX}giognutls${CMAKE_SHARED_LIBRARY_SUFFIX}
        PATH_SUFFIXES "lib/gio/modules"
    )

    IF(NOT giognutls_DLL)
        message(FATAL_ERROR "giognutls .dll NOT FOUND !")
    else()
        message("giognutls_DLL Found ! [${giognutls_DLL}]")
    endif()

    add_custom_command(TARGET appExec
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gio       
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${giognutls_DLL} $<TARGET_FILE_DIR:appExec>/gio
    ) 

endif()

#missing deps for gstreamer
if(APPLE)
    list(APPEND GST_SLIBS
        bz2
        gmp
        gnutls
        hogweed
        intl
        nettle
        soup-2.4
        tasn1
        xml2
    )
elseif(MINGW)
    list(APPEND GST_SLIBS
        gmp-10
        gnutls-30
        hogweed-5
        nettle-7
        soup-2.4-1
        tasn1-6
        winpthread-1
        xml2-2
    )
endif()

IF(MINGW OR APPLE)
    #import all missing GSTREAMER libs
    foreach(_gstSLib IN ITEMS ${GST_SLIBS})

        #broken find_library...
        find_file(${_gstSLib}_LIBRARY 
            NAMES ${CMAKE_SHARED_LIBRARY_PREFIX}${_gstSLib}${CMAKE_SHARED_LIBRARY_SUFFIX}
            PATH_SUFFIXES "bin"
        )

        IF(NOT ${_gstSLib}_LIBRARY)
            message(FATAL_ERROR "${_gstSLib}_LIBRARY NOT FOUND !")
        else()
            message("${_gstSLib}_LIBRARY Found ! [${${_gstSLib}_LIBRARY}]")
        endif()

        add_custom_command(TARGET appExec 
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${${_gstSLib}_LIBRARY} $<TARGET_FILE_DIR:appExec>
        )

    endforeach()
endif()

#############
# miniupnpc #
#############

#find miniupnpc
find_package(miniupnpc REQUIRED)

#link
target_link_libraries(appExec ${MINIUPNP_LIBRARY})
target_include_directories(appExec PRIVATE ${MINIUPNP_INCLUDE_DIR})

#####################
# list source files #
#####################

file(GLOB_RECURSE APP_CPP_FILES "src/*.cpp")
file(GLOB_RECURSE APP_HPP_FILES "src/*.hpp")

#defined as public for tests
target_sources(appExec PUBLIC
    ${APP_CPP_FILES}
    ${APP_HPP_FILES}
)

################
# translations #
################

#define translation files
list(APPEND APP_TRANSLATION_FILES 
    "src/_i18n/fr.ts"
)

#updates TS files from sources
qt5_create_translation(QM_FILES "src"
    ${APP_TRANSLATION_FILES}
)

#generate QM files
qt5_add_translation(APP_BINARY_TRANSLATION_FILES ${APP_TRANSLATION_FILES})

#generate QM files from TS
target_sources(appExec PUBLIC
    ${APP_BINARY_TRANSLATION_FILES}
)

#copy translations into output
add_custom_command(TARGET appExec   
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${QM_FILES} $<TARGET_FILE_DIR:appExec>/translations
)

################
# missing libs #
################

#scan for missing lib dependencies, then fetch and copy them
if(MINGW)
    if(NOT MINGW_DLL_PATH)
        message(FATAL_ERROR "MINGW_DLL_PATH must be set ! Please use a toolchain file !")
    endif()
    add_custom_command(TARGET appExec POST_BUILD
        COMMAND peldd-prefix/src/peldd-build/peldd -a
            -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} 
            -p ${MINGW_DLL_PATH}
            $<TARGET_FILE:appExec> 
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gio/* 
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gst-plugins/*
            | sed 's/\\\\\\\\/\\\\//g'
            | xargs cp -n -t ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
endif()

######################
# add Publish target #
######################

include(CMakePublish)
