project(RPGRPZ)
cmake_minimum_required(VERSION 3.13.3)

#version
SET(CMAKE_PROJECT_VERSION_MAJOR 0)
SET(CMAKE_PROJECT_VERSION_MINOR 15)
SET(CMAKE_PROJECT_VERSION_PATCH 1)


#default configuration
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

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
IF(${CMAKE_BINARY_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR}/buildDebug)

    #define debug and tool location
    SET(IS_DEBUG_APP ON)
    SET(MAINTENANCE_TOOL_LOCATION "C:/Program Files/RPGRPZ/maintenancetool")
	
ENDIF()

#config file to source code
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/build_templates/version.h.in 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h
)

#download cacert file to source code
file(DOWNLOAD 
    "https://crt.sh/?d=0687260331A72403D909F105E69BCF0D32E1BD2493FFC6D9206D11BCD6770739" 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/${PEM_CERT_NAME}
)
SET(CACERT src/${PEM_CERT_NAME})
get_filename_component(CACERT_FP ${CACERT} ABSOLUTE)

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
# miniupnpc #
#############

find_package(MiniUPnPc REQUIRED)

include_directories("${MINIUPNP_INCLUDE_DIR}")

list(APPEND EXT_LIBS
    ${MINIUPNP_LIBRARY}
    ws2_32
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

#############
# Gstreamer #
#############

# #pkgConfig setup
# SET(PKG_CONFIG_EXECUTABLE "pkg-config.exe")
# SET(GSTREAMER_LOCATION "C:/msys64/mingw64")

# include(GStreamerHelper)
# LinkGStreamer()

# #append librairies for later
# list(APPEND EXT_LIBS 
#     ${GST_STATIC_LIBRARIES}
#     ${GST_PLUGINS_BASE_STATIC_LIBRARIES}
# )
    
#################################
# mandatory explicit generation #
#################################

file(GLOB_RECURSE CPP_FILES src/*.cpp)
file(GLOB_RECURSE HPP_FILES src/*.hpp)

list(APPEND SOURCES
    ${CPP_FILES}
    ${HPP_FILES}
)

#########################
# Executable Generation #
#########################

# Create executable
add_executable(${PROJECT_NAME} ${BUNDLE_TYPE} ${SOURCES}) 

target_link_libraries(${PROJECT_NAME} ${EXT_LIBS})

##################
# LIBRARIES COPY #
##################

#misc
add_custom_command(TARGET ${PROJECT_NAME}
    #CA CERT
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CACERT_FP} $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

#using windeployqt to import all the .dll and qt requirements based on used components
IF(NOT EXISTS "${CMAKE_BINARY_DIR}/bin/Qt5Cored.dll")
    include(Windeployqt)
    windeployqt(${PROJECT_NAME} "${CMAKE_CURRENT_BINARY_DIR}/bin")
ENDIF()

# #copy gstreamer .dlls
# set(_gstreamer_plugins "all")
# DeployGStreamer(${PROJECT_NAME} "${_gstreamer_plugins}")
    

######################
# add Publish target #
######################

#include(CMakePublish)

#include(PrintAllVariables)