#policies first
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0011 NEW)
    cmake_policy(SET CMP0020 NEW)
    cmake_policy(SET CMP0003 NEW)
endif()

#use ccache
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    message("== Using CCache ! ==")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CCACHE_PROGRAM}")
endif()

#default configuration
list(APPEND CMAKE_MODULE_PATH 
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Qt
)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#############
## STARTUP ##
#############

#pre-startup
cmake_minimum_required(VERSION 3.10.2)
message(STATUS "Using toolchain file: [${CMAKE_TOOLCHAIN_FILE}].")

#project setup
project(RPGRPZ
    VERSION 1.0.20
    DESCRIPTION "Simple Tabletop RPG experience"
    HOMEPAGE_URL "https://github.com/Amphaal/rpgrpz"
    LANGUAGES CXX
)
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER) #lowered project name
SET(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME ${PROJECT_NAME_LOWER})

########################
## VERSION.H HANDLING ##
########################

SET(APP_PUBLISHER "LVWL")
SET(APP_PATCHNOTE_URL "https://github.com/Amphaal/rpgrpz/releases")
SET(SENTRY_ENDPOINT "https://ecb72ca256e744b990fd4f50aa52ea6f@o333337.ingest.sentry.io/5517405")

#configure header with basic app informations
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

#Qt
include(XCQt)
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

#define app executable
add_executable(appExec ${APP_BUNDLE_TYPE})
set_target_properties(appExec PROPERTIES OUTPUT_NAME "${PROJECT_NAME}") #define executable name
target_compile_definitions(appExec PRIVATE $<$<CONFIG:Debug>:_DEBUG>) #define _DEBUG markup if in DEBUG config

#generate pdb
IF(MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")  
    target_link_options(appExec PRIVATE -Wl,-pdb=) # generate pdb files
    target_compile_options(appExec PRIVATE -gcodeview) #use codeview for pdbs
endif()

#warnings for debug
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_link_options(appExec PRIVATE 
        -Wall 
        -Wextra 
        -Wpedantic 
        -Wno-unused-parameter
    )
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
target_link_libraries(appExec PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
)

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
    Multimedia
)
target_link_libraries(appExec PRIVATE
    Qt5::Core
    Qt5::Gui 
    Qt5::Widgets
    Qt5::Network
    Qt5::Svg
    Qt5::OpenGL
    Qt5::Multimedia
)

# QT LIBRARIES COPY
include(DeployQt)
DeployQt(appExec)

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

#############
# snowflake #
#############

add_subdirectory(deps/snowflake)

###########
# clipper #
###########

add_subdirectory(deps/clipper)

############################
# autoupdatercore (for Qt) #
############################

add_subdirectory(deps/autoupdatercore)

#################
# Sentry-native #
#################

add_subdirectory(deps/sentry-native)
target_link_libraries(appExec PRIVATE sentry)

# track version
add_custom_command(TARGET appExec 
    COMMAND printf "${CMAKE_PROJECT_VERSION}" > version
    COMMENT "Create file containing app version for sentry-native version tracking"
)

################
# NetworkCandy #
################

add_subdirectory(deps/NetworkCandy)
target_link_libraries(appExec PRIVATE nw-candy)

#############
# AudioTube #
#############

add_subdirectory(deps/AudioTube)
target_link_libraries(appExec PRIVATE audiotube)

#########
# Dicer #
#########

add_subdirectory(deps/Dicer)
target_link_libraries(appExec PRIVATE dicer)

############# 
# Gstreamer #
#############

#find Gstreamer
LIST(APPEND GST_MODULES_TO_IMPORT 
    gstreamer-1.0
    gstreamer-controller-1.0
    gstreamer-net-1.0
    gstreamer-audio-1.0
)
pkg_check_modules(Gst REQUIRED IMPORTED_TARGET ${GST_MODULES_TO_IMPORT})

#since mms-bitfields flag from glib is PUBLIC and throws a bunch of warnings...
IF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(PkgConfig::Gst INTERFACE "-Wno-incompatible-ms-struct")
ENDIF()

#link
target_link_libraries(appExec PRIVATE     
    PkgConfig::Gst
)

IF(MINGW)
    ####define gstreamer plugins to use
    #core
    LIST(APPEND _gstreamer_plugins 
        gstplayback
        gstautodetect
        gstcoreelements
    )
    #opus stream read
    LIST(APPEND _gstreamer_plugins 
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
    pkg_get_variable(GSTREAMER_PLUGINS_LIB_LOCATION gstreamer-1.0 
        pluginsdir
    )

    IF(MINGW)
        LIST(APPEND _gstreamer_plugins
            gstwaveform
        )
    endif()

    message("Searching for gst plugins in [${GSTREAMER_PLUGINS_LIB_LOCATION}]...")

    #copy gstreamer plugins into output
    foreach(_gstPluginLib IN ITEMS ${_gstreamer_plugins})

        #using find_file because find_library is kinda broken with .dll on MinGW
        find_file(${_gstPluginLib}_LIBRARY 
            NAMES   lib${_gstPluginLib}.dll
                    lib${_gstPluginLib}.so
            PATHS ${GSTREAMER_PLUGINS_LIB_LOCATION}
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

#missing deps for gstreamer
if(MINGW)
    list(APPEND GST_SLIBS
        gmp-10
        gnutls-30
        hogweed-6
        nettle-8
        soup-2.4-1
        tasn1-6
        winpthread-1
        xml2-2
    )

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

#####################
# list source files #
#####################

file(GLOB_RECURSE APP_CPP_FILES "src/*.cpp")
file(GLOB_RECURSE APP_HPP_FILES "src/*.hpp")
file(GLOB_RECURSE APP_H_FILES "src/*.h")
file(GLOB_RECURSE APP_UI_FILES "src/*.ui")

#defined as public for tests
target_sources(appExec PUBLIC
    ${APP_CPP_FILES}
    ${APP_HPP_FILES}
)

################
# translations #
################

list(APPEND TR_SOURCE_FILES
    ${APP_CPP_FILES}
    ${APP_HPP_FILES}
    ${APP_H_FILES}
    ${APP_UI_FILES}
)

include(QtTranslation)
HandleQtTranslation(appExec 
    ${TR_SOURCE_FILES} 
    "src/_i18n/fr.ts"
)

########################
# Runtime dependencies #
########################

include(CMakeDependencies)

######################
# add Publish target #
######################

# publish script
include(CMakePublish)
