project(RPGRPZ)
cmake_minimum_required(VERSION 3.14.3)

#version
SET(CMAKE_PROJECT_VERSION_MAJOR 0)
SET(CMAKE_PROJECT_VERSION_MINOR 17)
SET(CMAKE_PROJECT_VERSION_PATCH 8)

#default configuration
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

########################
## VERSION.H HANDLING ##
########################

#app dest path
SET(APP_PUBLISHER "LVWL")
SET(APP_ROOT_URL "https://zonme.to2x.ovh")
SET(APP_MAIN_URL ${APP_ROOT_URL}/rpgrpz)
SET(APP_PATCHNOTE_URL "https://github.com/Amphaal/rpgrpz/releases")
SET(PEM_CERT_NAME "cacert.pem")

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

message("####################################################")
message("### BUILD_TYPE : Building RPG-RPZ for ${CMAKE_BUILD_TYPE}... ###")
message("####################################################")


#config file to source code
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/templates/version.h.in 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h
)

#download cacert file to source code
file(DOWNLOAD 
    "https://crt.sh/?d=0687260331A72403D909F105E69BCF0D32E1BD2493FFC6D9206D11BCD6770739" 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/${PEM_CERT_NAME}
)
SET(CACERT src/${PEM_CERT_NAME})
get_filename_component(CACERT_FP ${CACERT} ABSOLUTE)

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
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.14)
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
    set_source_files_properties(${CACERT} PROPERTIES MACOSX_PACKAGE_LOCATION "MacOS") #locate cert in package
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
    SET(PKG_CONFIG_EXECUTABLE "C:/msys64/mingw64/bin/x86_64-w64-mingw32-pkg-config.exe")
    SET(W32_GSTREAMER_LOCATION $ENV{GSTREAMER_1_0_ROOT_X86_64})
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

#force official binaires from Qt
IF(WIN32)
    SET(Qt5_DIR "C:/Qt/5.13.0/msvc2017_64/lib/cmake/Qt5")
endif()
IF(APPLE)
    SET(Qt5_DIR "/Qt/5.13.0/clang_64/lib/cmake/Qt5")
endif()

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Network Svg OpenGL)
list(APPEND EXT_LIBS
    Qt5::Core
    Qt5::Gui 
    Qt5::Widgets
    Qt5::Network
    Qt5::Svg
    Qt5::OpenGL
)

#################################
# mandatory explicit generation #
#################################

file(GLOB_RECURSE CPP_FILES src/*.cpp)
file(GLOB_RECURSE HPP_FILES src/*.hpp)

list(APPEND SOURCES
    ${CPP_FILES}
    ${HPP_FILES}
)

# #Debug files
# list(APPEND SOURCES
#     "src/helpers/_appContext.cpp"
#     "src/_tests/main.cpp"
# )


#########################
# Executable Generation #
#########################

# Create executable
add_executable(${PROJECT_NAME} ${BUNDLE_TYPE} ${SOURCES}) 

conan_target_link_libraries(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} ${EXT_LIBS})

if(APPLE)
    #add highdpi support / menu behavior via plist template (MACOS)
    set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/templates/Info.plist.in) 
endif()

if(WIN32)
    #accelerate builds times
    include(cotire)
    cotire(${PROJECT_NAME})
endif()

################
# INSTALL COPY #
################

#CA CERT
add_custom_command(TARGET ${PROJECT_NAME}       
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CACERT_FP} $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

#####################
# QT LIBRARIES COPY #
#####################

IF(WIN32)
    SET(QT_DEPLOY_BIN_PATH "C:/Qt/5.13.0/msvc2017_64/bin")
endif()

IF(APPLE)
    SET(QT_DEPLOY_BIN_PATH "/Qt/5.13.0/clang_64/bin")
endif()

#using deployqt to import all the QT ${CMAKE_SHARED_LIBRARY_SUFFIX} requirements based on used components
include(deployqt)
deployqt(${PROJECT_NAME})


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

    add_custom_command(TARGET ${PROJECT_NAME}       
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        "C:/Qt/Tools/OpenSSL/Win_x64/bin/libcrypto-1_1-x64.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )

    add_custom_command(TARGET ${PROJECT_NAME}       
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        "C:/Qt/Tools/OpenSSL/Win_x64/bin/libssl-1_1-x64.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
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