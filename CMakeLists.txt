#policies first
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0011 NEW)
    cmake_policy(SET CMP0020 NEW)
    cmake_policy(SET CMP0003 NEW)
endif()

#check required ENV vars
list(APPEND ENV_VAR_TO_CHECK
    GSTREAMER_1_0_ROOT_X86_64
    QTDIR
    QT_TOOLS
    VCPKG_ROOT
    SENTRY_NATIVE_ROOT
)
foreach(C_ENV IN LISTS ENV_VAR_TO_CHECK)
    if(NOT DEFINED ENV{${C_ENV}})
        message(FATAL_ERROR "Required ENV variable ${C_ENV} does not exists. Please check README.md for more details !")
    endif()
endforeach()

#define toolchain
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

#project setup
project(RPGRPZ)
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)
cmake_minimum_required(VERSION 3.16.0)

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
SET(SENTRY_ENDPOINT "http://9224cd6702ba4791a147f192af767519@zonme.to2x.ovh:9000/1")

##crashpad
SET(CRASHPAD_HANDLER_NAME "crashpad_handler")
IF(WIN32)
    SET(CRASHPAD_HANDLER_NAME "${CRASHPAD_HANDLER_NAME}.exe")
ENDIF()

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
    SET(W32_GSTREAMER_LOCATION $ENV{GSTREAMER_1_0_ROOT_X86_64})
    string(REPLACE "\\" "/" GSTREAMER_LOCATION ${W32_GSTREAMER_LOCATION})
    SET(GSTREAMER_SLIB_LOCATION "${GSTREAMER_LOCATION}/bin")
endif()

if(APPLE)
    SET(PKG_CONFIG_EXECUTABLE "/usr/local/Cellar/pkg-config/0.29.2/bin/pkg-config")
    SET(GSTREAMER_LOCATION "${W32_GSTREAMER_LOCATION}/Libraries/pkgconfig")
    SET(GSTREAMER_SLIB_LOCATION "${W32_GSTREAMER_LOCATION}/Libraries")
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

SET(Qt5_DIR "$ENV{QTDIR}/lib/cmake/Qt5")
SET(QT_DEPLOY_BIN_PATH "$ENV{QTDIR}/bin")

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

SET(OpenSSL_DIR "$ENV{QT_TOOLS}/OpenSSL/Win_x64")

find_package(openssl REQUIRED)
list(APPEND EXT_LIBS
    OpenSSL::SSL
    OpenSSL::Crypto
    ws2_32
)

#################
# Sentry-native #
#################

#add sentry-native sources
include_external_msproject(sentry_crashpad $ENV{SENTRY_NATIVE_ROOT}/gen_windows/sentry_crashpad.vcxproj)
include_external_msproject(crashpad_handler $ENV{SENTRY_NATIVE_ROOT}/gen_windows/crashpad_handler.vcxproj)

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

#sentry
add_dependencies(app sentry_crashpad)
add_dependencies(app crashpad_handler)
target_include_directories(app PUBLIC $ENV{SENTRY_NATIVE_ROOT}/include)

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

if(WIN32)
    #accelerate builds times
    include(cotire)
    cotire(app)
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

SET(SENTRY_BINARIES_DIR $ENV{SENTRY_NATIVE_ROOT}/gen_windows/bin/x86_64)

#crashpad
add_custom_command(TARGET app   
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SENTRY_BINARIES_DIR}/$<CONFIG>/${CRASHPAD_HANDLER_NAME} $<TARGET_FILE_DIR:app>
)

#sentry
add_custom_command(TARGET app
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SENTRY_BINARIES_DIR}/$<CONFIG>/sentry_crashpad.dll $<TARGET_FILE_DIR:app>
)

#translations
add_custom_command(TARGET app  
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${QM_FILES} $<TARGET_FILE_DIR:app>/translations
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
    add_custom_command(TARGET app
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
            "${GSTREAMER_SLIB_LOCATION}/${CMAKE_SHARED_LIBRARY_PREFIX}${_gstSLib}${CMAKE_SHARED_LIBRARY_SUFFIX}"
            "$<TARGET_FILE_DIR:app>"
    )
endforeach()

#missing specifics
IF(WIN32)
    add_custom_command(TARGET app       
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        "${GSTREAMER_LOCATION}/lib/gio/modules/giognutls${CMAKE_SHARED_LIBRARY_SUFFIX}" 
        $<TARGET_FILE_DIR:app>/gio/giognutls${CMAKE_SHARED_LIBRARY_SUFFIX}
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
    DeployGStreamer(app "${_gstreamer_plugins}")
endif()


###########
# OpenSSL #
###########

if(WIN32)

    #override
    SET(OPENSSL_BIN_DIR "${OpenSSL_DIR}/bin")

    add_custom_command(TARGET app       
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        "${OPENSSL_BIN_DIR}/libcrypto-1_1-x64.dll"
        $<TARGET_FILE_DIR:app>
    )

    add_custom_command(TARGET app      
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        "${OPENSSL_BIN_DIR}/libssl-1_1-x64.dll"
        $<TARGET_FILE_DIR:app>
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