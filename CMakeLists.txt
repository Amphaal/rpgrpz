project(RPGRPZ)
cmake_minimum_required(VERSION 3.14.3)

#version
SET(CMAKE_PROJECT_VERSION_MAJOR 0)
SET(CMAKE_PROJECT_VERSION_MINOR 20)
SET(CMAKE_PROJECT_VERSION_PATCH 2)

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

#set default maintenance tool path
SET(MAINTENANCE_TOOL_LOCATION "")
SET(IS_DEBUG_APP OFF)

#if debug build
IF(CMAKE_INSTALL_CONFIG_NAME STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "Debug")
    #define debug
    SET(IS_DEBUG_APP ON)
ENDIF()

#force build type for Md / Mt flags attribution
IF(NOT DEFINED CMAKE_BUILD_TYPE)
    IF(CMAKE_INSTALL_CONFIG_NAME STREQUAL "Debug")
        SET(CMAKE_BUILD_TYPE "Debug")
    else()
        SET(CMAKE_BUILD_TYPE "Release")
    endif()
endif()

#config file to source code
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/templates/_version.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h
)

##################
## Enable Conan ##
##################

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(KEEP_RPATHS)

################################
## CPP Compiler Configuration ##
################################

SET(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

IF(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN) #std::btye c++17 ambiguity
    add_definitions(-D_CRT_SECURE_NO_WARNINGS) #ctime_s...
endif()

if (APPLE)
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
    SET(RPGRPZ_ICO ${CMAKE_CURRENT_SOURCE_DIR}/resources/resources.rc) #add icon
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
    ${RPGRPZ_ICO}
    resources/resources.qrc
)

###########
# OpenSSL #
###########

find_package(OpenSSL REQUIRED COMPONENTS SSL Crypto)
list(APPEND EXT_LIBS
    OpenSSL::SSL
    OpenSSL::Crypto
)

############# 
# Gstreamer #
#############

#pkgConfig setup
IF(WIN32)
    #SET(PKG_CONFIG_EXECUTABLE "C:/msys64/mingw64/bin/x86_64-w64-mingw32-pkg-config.exe") #install pkgconfiglite from Choco
    SET(W32_GSTREAMER_LOCATION $ENV{GSTREAMER_1_0_ROOT_X86_64})
    IF(NOT DEFINED W32_GSTREAMER_LOCATION)
        SET(W32_GSTREAMER_LOCATION "C:/gstreamer/1.0/x86_64")
    endif()
    string(REPLACE "\\" "/" GSTREAMER_LOCATION ${W32_GSTREAMER_LOCATION})
    SET(GSTREAMER_SLIB_LOCATION "${GSTREAMER_LOCATION}/bin")
endif()

if(APPLE)
    SET(PKG_CONFIG_EXECUTABLE "/usr/local/Cellar/pkg-config/0.29.2/bin/pkg-config")
    SET(GSTREAMER_LOCATION "/Library/Frameworks/GStreamer.framework/Libraries/pkgconfig")
    SET(GSTREAMER_SLIB_LOCATION "/Library/Frameworks/GStreamer.framework/Libraries")
endif(APPLE)

include(GStreamerHelper)
FindGStreamer()

list(APPEND EXT_LIBS
    PkgConfig::Gst
    PkgConfig::GstPluginsBase
)

########
# QT 5 #
########

#override maindir
IF(NOT DEFINED QT_MAIN_DIR)
    IF(WIN32)
        SET(QT_MAIN_DIR "C:/Qt/5.13.2")
    endif()
    IF(APPLE)
        SET(QT_MAIN_DIR "$ENV{HOME}/Qt/5.13.2")
    endif()
endif()


#force official binaires from Qt
IF(WIN32)
    SET(Qt5_DIR "${QT_MAIN_DIR}/msvc2017_64/lib/cmake/Qt5")
endif()
IF(APPLE)
    SET(Qt5_DIR "${QT_MAIN_DIR}/clang_64/lib/cmake/Qt5")
endif()

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Network Svg OpenGL LinguistTools)
list(APPEND EXT_LIBS
    Qt5::Core
    Qt5::Gui 
    Qt5::Widgets
    Qt5::Network
    Qt5::Svg
    Qt5::OpenGL
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

#generate translations
qt5_create_translation(QM_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_i18n/fr.ts
)

#########################
# Executable Generation #
#########################

# Create executable
add_executable(${PROJECT_NAME} ${BUNDLE_TYPE} ${SOURCES} ${QM_FILES})

#link !
conan_target_link_libraries(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} ${EXT_LIBS})

if(APPLE)
    #add highdpi support / menu behavior via plist template (MACOS)
    set_target_properties(${PROJECT_NAME} PROPERTIES 
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/templates/_Info.plist
    ) 
endif()

if(WIN32)
    #accelerate builds times
    include(cotire)
    cotire(${PROJECT_NAME})
endif()

#####################
# QT LIBRARIES COPY #
#####################

IF(WIN32)
    SET(QT_DEPLOY_BIN_PATH "${QT_MAIN_DIR}/msvc2017_64/bin")
endif()

IF(APPLE)
    SET(QT_DEPLOY_BIN_PATH "${QT_MAIN_DIR}/clang_64/bin")
endif()

#using deployqt to import all the QT ${CMAKE_SHARED_LIBRARY_SUFFIX} requirements based on used components
include(deployqt)
deployqt(${PROJECT_NAME})

###############
# OTHERS COPY #
###############

#translations
add_custom_command(TARGET ${PROJECT_NAME}   
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${QM_FILES} $<TARGET_FILE_DIR:${PROJECT_NAME}>/translations
)

######################
# GST LIBRARIES COPY #
######################

if(WIN32)
    #missing deps for gstreamer
    list(APPEND GST_SLIBS
        avcodec-58
        avfilter-7
        avformat-58
        avutil-56
        bz2
        libgmp-10
        libgnutls-30
        libhogweed-4
        libnettle-6
        libsoup-2.4-1
        libtasn1-6
        libwinpthread-1
        libxml2-2
        swresample-3
    )
endif()

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

#import all missing GSTREAMER libs
foreach(_gstSLib IN ITEMS ${GST_SLIBS})
    add_custom_command(TARGET ${PROJECT_NAME}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
            "${GSTREAMER_SLIB_LOCATION}/${CMAKE_SHARED_LIBRARY_PREFIX}${_gstSLib}${CMAKE_SHARED_LIBRARY_SUFFIX}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    )
endforeach()

#missing specifics
IF(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME}       
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        "${GSTREAMER_LOCATION}/lib/gio/modules/giognutls${CMAKE_SHARED_LIBRARY_SUFFIX}" 
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/gio/giognutls${CMAKE_SHARED_LIBRARY_SUFFIX}
    )
endif()

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
    DeployGStreamer(${PROJECT_NAME} "${_gstreamer_plugins}")
endif()


###########
# OpenSSL #
###########

if(WIN32)

    #override
    IF(NOT DEFINED OPENSSL_BIN_DIR)
        SET(OPENSSL_BIN_DIR "C:/Qt/Tools/OpenSSL/Win_x64/bin")
    endif()

    add_custom_command(TARGET ${PROJECT_NAME}       
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        "${OPENSSL_BIN_DIR}/libcrypto-1_1-x64.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )

    add_custom_command(TARGET ${PROJECT_NAME}       
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        "${OPENSSL_BIN_DIR}/libssl-1_1-x64.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )

endif()

######################
# add Publish target #
######################

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    include(CMakePublish)
endif()

#################
# Debug helpers #
#################

#include(PrintAllVariables)

#include(PrintTargetProperties)
#print_target_properties(PkgConfig::GstPluginsBase)