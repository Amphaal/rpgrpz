project(RPGRPZ)
cmake_minimum_required(VERSION 3.13.3)

#version
SET(CMAKE_PROJECT_VERSION_MAJOR 0)
SET(CMAKE_PROJECT_VERSION_MINOR 16)
SET(CMAKE_PROJECT_VERSION_PATCH 1)


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

    #define debug and tool location
    SET(IS_DEBUG_APP ON)
    SET(MAINTENANCE_TOOL_LOCATION "C:/Program Files/RPGRPZ/maintenancetool")
	
ENDIF()

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
add_definitions(-DWIN32_LEAN_AND_MEAN) #std::btye c++17 ambiguity

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

SET(BUNDLE_TYPE WIN32) #set bundle type
SET(RPGRPZ_ICO ${CMAKE_CURRENT_SOURCE_DIR}/resources/resources.rc) #add icon

#default source contains
SET(SOURCES
    ${RPGRPZ_ICO}
    resources/resources.qrc
)

#############
# Gstreamer #
#############

#pkgConfig setup
SET(PKG_CONFIG_EXECUTABLE "C:/msys64/mingw64/bin/x86_64-w64-mingw32-pkg-config.exe")
SET(W32_GSTREAMER_LOCATION $ENV{GSTREAMER_1_0_ROOT_X86_64})
string(REPLACE "\\" "/" GSTREAMER_LOCATION ${W32_GSTREAMER_LOCATION})

include(GStreamerHelper)
FindGStreamer()

list(APPEND EXT_LIBS
    PkgConfig::Gst
    PkgConfig::GstPluginsBase
)

########
# QT 5 #
########

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

# #accelerate builds times
include(cotire)
cotire(${PROJECT_NAME})

##################
# LIBRARIES COPY #
##################

#misc
add_custom_command(TARGET ${PROJECT_NAME}

    # get windeployqt tool from official snapshot
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "C:/Qt/5.13.0/msvc2017_64/bin/windeployqt.exe" $<TARGET_FILE_DIR:Qt5::Core>/../bin/windeployqt.exe
    
    #CA CERT
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CACERT_FP} $<TARGET_FILE_DIR:${PROJECT_NAME}> 

    #missing deps for gstreamer
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_LOCATION}/bin/avcodec-58.dll" $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_LOCATION}/bin/avfilter-7.dll" $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_LOCATION}/bin/avformat-58.dll" $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_LOCATION}/bin/avutil-56.dll" $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_LOCATION}/bin/bz2.dll" $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_LOCATION}/bin/libgmp-10.dll" $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_LOCATION}/bin/libgnutls-30.dll" $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_LOCATION}/bin/libhogweed-4.dll" $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_LOCATION}/bin/libnettle-6.dll" $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_LOCATION}/bin/libsoup-2.4-1.dll" $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_LOCATION}/bin/libtasn1-6.dll" $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_LOCATION}/bin/libwinpthread-1.dll" $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_LOCATION}/bin/libxml2-2.dll" $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_LOCATION}/bin/swresample-3.dll" $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_LOCATION}/lib/gio/modules/giognutls.dll" $<TARGET_FILE_DIR:${PROJECT_NAME}>/gio/giognutls.dll
)

#using windeployqt to import all the QT .dll requirements based on used components
IF(NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/bearer/)
    include(Windeployqt)
    windeployqt(${PROJECT_NAME})
endif()

#copy gstreamer .dlls
IF(NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gst-plugins/)
    #LIST(APPEND _gstreamer_plugins all)
    LIST(APPEND _gstreamer_plugins 
        gstaudioconvert
        gstaudioparsers
        gstaudioresample
        gstcoreelements
        gstisomp4
        gstlibav
        gstplayback
        gstsoup
        gsttypefindfunctions
        gstvolume
        gstwasapi
    )
    DeployGStreamer(${PROJECT_NAME} "${_gstreamer_plugins}")
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