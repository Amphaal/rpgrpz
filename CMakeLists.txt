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

#################################
## define bundle configuration ##
#################################

#define build type if any
IF(WIN32)
    SET(APP_BUNDLE_TYPE "WIN32") 
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        SET(APP_BUNDLE_TYPE "") #allows Debug cout/cerr console... from LLDB/GDB
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

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    IF(WIN32)  
        target_link_options(appExec PRIVATE -Wl,-pdb=) # generate pdb files
    endif()
else()
    set_target_properties(appExec PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE) #enable LTO
endif()

#define test executable
add_executable(testsExec)
SET(TEST_EXEC_OUTPUT_NAME "${PROJECT_NAME}_tests")
set_target_properties(testsExec PROPERTIES OUTPUT_NAME "${TEST_EXEC_OUTPUT_NAME}") #define executable name
target_sources(testsExec PRIVATE "tests/tests.cpp")
target_link_libraries(testsExec INTERFACE appExec)

#########################
## ressources handling ##
#########################

#default QT ressources
target_sources(appExec PRIVATE "resources/resources.qrc")

IF(WIN32)  
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

IF(WIN32)
    SET(PKG_CONFIG_USE_CMAKE_PREFIX_PATH TRUE)
    list(APPEND CMAKE_PREFIX_PATH "$ENV{MINGW64_ROOT}/lib/cmake;$ENV{MINGW64_ROOT};$ENV{MINGW64_ROOT}/x86_64-w64-mingw32") #contains libm (math.h), helps gst linkage
    list(APPEND CMAKE_MODULE_PATH "$ENV{MINGW64_ROOT}/lib/cmake") #helps to find cmake packages
    list(APPEND CMAKE_LIBRARY_PATH "$ENV{MINGW64_ROOT}/lib") #helps to find dll.a files for LDD linkage
    list(APPEND CMAKE_INCLUDE_PATH "$ENV{MINGW64_ROOT}/include") #helps to find header files
    list(APPEND CMAKE_PROGRAM_PATH "$ENV{MINGW64_ROOT}/bin") #contains most of .dll files to be installed with executable
endif()

#using pkgConfig, set PKG_CONFIG_EXECUTABLE if not found
find_package(PkgConfig REQUIRED)

#############
# miniupnpc #
#############

#find miniupnpc
find_package(miniupnpc REQUIRED)

#link
target_link_libraries(appExec ${MINIUPNP_LIBRARY})
target_include_directories(appExec PRIVATE ${MINIUPNP_INCLUDE_DIR})

#copy lib into a target
find_library(miniupnpc_BIN_LIBRARY NAMES "miniupnpc" PATHS "${CMAKE_PROGRAM_PATH}" NO_DEFAULT_PATH)
add_custom_command(TARGET appExec 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        ${miniupnpc_BIN_LIBRARY} $<TARGET_FILE_DIR:appExec>
)

##############
# googletest #
##############

#find googletest
pkg_check_modules(GTest REQUIRED IMPORTED_TARGET
    gtest_main
)

#link
target_link_libraries(testsExec PRIVATE PkgConfig::GTest)
target_include_directories(testsExec PRIVATE ${GTest_INCLUDE_DIRS})

find_library(gtest_main_BIN_LIBRARY NAMES "gtest_main" PATHS "${CMAKE_PROGRAM_PATH}" NO_DEFAULT_PATH)
find_library(gtest_BIN_LIBRARY NAMES "gtest" PATHS "${CMAKE_PROGRAM_PATH}" NO_DEFAULT_PATH)

add_custom_command(TARGET appExec 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${gtest_main_BIN_LIBRARY} $<TARGET_FILE_DIR:testsExec>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${gtest_BIN_LIBRARY} $<TARGET_FILE_DIR:testsExec>
)

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

#link
target_link_libraries(appExec     
    PkgConfig::Gst
)

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

#copy gstreamer plugins into output
foreach(_gstPluginLib IN ITEMS ${_gstreamer_plugins})
    
    find_library(${_gstPluginLib}_LIBRARY NAMES "${_gstPluginLib}" PATHS "${GSTREAMER_PLUGINS_DLL_LOCATION}" NO_DEFAULT_PATH)
    
    list(APPEND _gstPluginLib_bin ${${_gstPluginLib}_LIBRARY})
    
    add_custom_command(TARGET appExec 
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
            ${${_gstPluginLib}_LIBRARY}
            $<TARGET_FILE_DIR:appExec>/gst-plugins
    )

endforeach()

# ######################
# # GST LIBRARIES COPY #
# ######################

#platform specifics missing dependency
IF(WIN32)
    find_library(giognutls_LIBRARY NAMES "giognutls" PATHS "$ENV{MINGW64_ROOT}/lib/gio/modules" NO_DEFAULT_PATH)
    add_custom_command(TARGET appExec
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gio       
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${giognutls_LIBRARY} $<TARGET_FILE_DIR:appExec>/gio
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
elseif(WIN32)
    list(APPEND GST_SLIBS
        libgmp-10
        libgnutls-30
        libhogweed-5
        libnettle-7
        libsoup-2.4-1
        libtasn1-6
        libwinpthread-1
        libxml2-2
    )
endif()

#import all missing GSTREAMER libs
foreach(_gstSLib IN ITEMS ${GST_SLIBS})
    find_library(${_gstSLib}_LIBRARY NAMES "${_gstSLib}" PATHS "${CMAKE_PROGRAM_PATH}" NO_DEFAULT_PATH)
    add_custom_command(TARGET appExec 
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
            ${${_gstSLib}_LIBRARY}
            $<TARGET_FILE_DIR:appExec>
    )
endforeach()

########
# QT 5 #
########

#find Qt5
find_package(Qt5 REQUIRED COMPONENTS 
    Core 
    Gui 
    Widgets 
    Network 
    Svg 
    OpenGL 
    LinguistTools
)

#link
target_link_libraries(appExec
    Qt5::Core
    Qt5::Gui 
    Qt5::Widgets
    Qt5::Network
    Qt5::Svg
    Qt5::OpenGL
)

# QT LIBRARIES COPY
if(WIN32)
    include(Windeployqt)
    windeployqt(appExec)
endif()

if(APPLE)
    include(Macdeployqt)
    macdeployqt(appExec)
endif()

#missing libs
IF(WIN32)
    find_library(libjpeg_LIBRARY NAMES "libjpeg-8" PATHS "${CMAKE_PROGRAM_PATH}" NO_DEFAULT_PATH) #for icons handling
    add_custom_command(TARGET appExec 
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
            ${libjpeg_LIBRARY} $<TARGET_FILE_DIR:appExec>
    )
endif()

###########
# OpenSSL #
###########

#find
find_package(openssl REQUIRED)

#link
target_link_libraries(appExec
    OpenSSL::SSL
    OpenSSL::Crypto
)

if(WIN32)
    target_link_libraries(appExec ws2_32) #WSA functions !
endif()

#find libs
find_library(OPENSSL_CPYPTO_BIN_LIBRARY NAMES "crypto-1_1-x64" PATHS "${CMAKE_PROGRAM_PATH}" NO_DEFAULT_PATH)
find_library(OPENSSL_SSL_BIN_LIBRARY NAMES "ssl-1_1-x64" PATHS "${CMAKE_PROGRAM_PATH}" NO_DEFAULT_PATH)

#copy lib into a target
add_custom_command(TARGET appExec 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OPENSSL_CPYPTO_BIN_LIBRARY} $<TARGET_FILE_DIR:appExec>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OPENSSL_SSL_BIN_LIBRARY} $<TARGET_FILE_DIR:appExec>
)

#################
# Sentry-native #
#################

#declare git repo
include(FetchContent)
FetchContent_Declare(
  sentry-native
  GIT_REPOSITORY https://github.com/Amphaal/sentry-native.git
  GIT_TAG        feature/c-ver
)

#fetch and add as 
FetchContent_MakeAvailable(sentry-native)

#link
target_include_directories(appExec PRIVATE "${Sentry-Native_SOURCE_DIR}/include")
target_link_libraries(appExec sentry)

#install
install(TARGETS sentry
    RUNTIME
        DESTINATION .
    PUBLIC_HEADER
        OPTIONAL
)

############
# Breakpad #
############

#find breakpad
pkg_check_modules(BrkpdCli REQUIRED IMPORTED_TARGET
    breakpad-client
)

#link
target_link_libraries(appExec
    PkgConfig::BrkpdCli
)

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
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        ${QM_FILES} $<TARGET_FILE_DIR:appExec>/translations
)

################
# missing libs #
################

#scan for missing lib dependencies, then fetch and copy them
if(WIN32)

    STRING(REGEX REPLACE "/" "\\\\\\\\\\\\" ESCAPED_CMAKE_PROGRAM_PATH ${CMAKE_PROGRAM_PATH})

    add_custom_command(TARGET appExec POST_BUILD
        COMMAND ntldd -R -D ${CMAKE_PROGRAM_PATH} $<TARGET_FILE:appExec> | grep -o '${ESCAPED_CMAKE_PROGRAM_PATH}\\\\\\\\.*\\s' | sed 's/\\\\\\\\/\\\\//g' | xargs cp -n -t ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMAND ntldd -R -D ${CMAKE_PROGRAM_PATH} ${giognutls_LIBRARY} | grep -o '${ESCAPED_CMAKE_PROGRAM_PATH}\\\\\\\\.*\\s' | sed 's/\\\\\\\\/\\\\//g' | xargs cp -n -t ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}        
        COMMAND ntldd -R -D ${CMAKE_PROGRAM_PATH} ${_gstPluginLib_bin} | grep -o '${ESCAPED_CMAKE_PROGRAM_PATH}\\\\\\\\.*\\s' | sed 's/\\\\\\\\/\\\\//g' | xargs cp -n -t ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
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
#print_target_properties(package)