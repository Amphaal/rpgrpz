project(RPGRPZ)
cmake_minimum_required(VERSION 3.13.3)

########################
## VERSION.H HANDLING ##
########################

#app dest path
SET(APP_PUBLISHER "LVWL")
SET(APP_ROOT_URL "https://zonme.to2x.ovh")
SET(APP_MAIN_URL ${APP_ROOT_URL}/rpgrpz)
SET(APP_PATCHNOTE_URL "https://github.com/Amphaal/rpgrpz/releases")
SET(PEM_CERT_NAME "cacert.pem")

#version
SET(CMAKE_PROJECT_VERSION_MAJOR 0)
SET(CMAKE_PROJECT_VERSION_MINOR 6)
SET(CMAKE_PROJECT_VERSION_PATCH 1)


#set default maintenance tool path
SET(MAINTENANCE_TOOL_LOCATION "")
SET(IS_DEBUG_APP OFF)

IF(${CMAKE_BINARY_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR}/buildDebug)

    SET(IS_DEBUG_APP ON)

    if (WIN32)
        SET(MAINTENANCE_TOOL_LOCATION "C:/Qt/MaintenanceTool")
    endif(WIN32)
	
ENDIF()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/build_templates/version.h.in 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h
)

##################
## Enable Conan ##
##################

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
include(${CMAKE_BINARY_DIR}/conan_paths.cmake)
conan_basic_setup(KEEP_RPATHS)

################################
## CPP Compiler Configuration ##
################################

SET(CMAKE_CXX_STANDARD 17)

if(MSVC)
    add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
endif(MSVC)

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

if (WIN32)
    SET(BUNDLE_TYPE WIN32) #set bundle type
    SET(RPGRPZ_ICO ${CMAKE_CURRENT_SOURCE_DIR}/resources/resources.rc) #add icon
endif (WIN32)

#default source contains
SET(SOURCES
    ${RPGRPZ_ICO}
    resources/resources.qrc
)

####################
# Code compilation #
####################

# Define required linked QT libs
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network Svg)
SET(EXT_LIBS 
    Qt5::Widgets 
    Qt5::Core
    Qt5::Network
    Qt5::Svg
)

#private libs
file(GLOB_RECURSE INT_LIBS
    "libs/*.c"
    "libs/*.cpp"
)

##
# mandatory explicit generation
##

#QThread derivatives
list(APPEND SOURCES
    src/network/rpz/client/RPZCThread.h
    src/network/rpz/server/RPZSThread.h
    src/network/audio/client/ACThread.h
    src/network/audio/server/ASThread.h
    src/network/uPnP/uPnPThread.h
)

#functionnal components
list(APPEND SOURCES
    src/network/rpz/_any/JSONSocket.cpp
    src/network/audio/client/AudioClient.cpp
    src/network/audio/server/AudioServer.cpp
    src/network/rpz/client/RPZClient.cpp
    src/network/rpz/server/RPZServer.cpp
    src/network/uPnP/uPnPRequester.cpp
    src/network/ConnectivityHelper.cpp
)

#ui components
list(APPEND SOURCES
    src/ui/components/MapView.cpp
    src/ui/components/ChatEdit.cpp
    src/ui/components/LogView.cpp
    src/ui/components/LogScrollView.cpp
    src/ui/components/ChatWidget.cpp
    src/ui/components/ConnectWidget.cpp
    src/ui/components/AudioStreamController.cpp
    src/ui/components/AudioStreamNotifier.cpp
    src/ui/components/AssetsManager.cpp
)

#main
list(APPEND SOURCES
    src/main.cpp
)

#########################
# Executable Generation #
#########################

# Create executable
add_executable(${PROJECT_NAME} ${BUNDLE_TYPE} ${SOURCES} ${INT_LIBS}) 

if (WIN32)    
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/ignore:4099 /ignore:4267 /ignore:4244 /ignore:4715")
endif (WIN32)

# Bind ext libs
conan_target_link_libraries(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} ${EXT_LIBS})

#download cacert for windows mostly
##too heavy, prefer specific intermediate local 
#SET(CACERT_URL_SOURCE "https://curl.haxx.se/ca/cacert.pem")
##DST Root CA X3 only, from Firefox's "https://ccadb-public.secure.force.com/mozilla/IncludedCACertificateReport"
SET(CACERT_URL_SOURCE "https://crt.sh/?d=0687260331A72403D909F105E69BCF0D32E1BD2493FFC6D9206D11BCD6770739")

if (WIN32)    
    SET(PEM_CERT_DEST ${CMAKE_BINARY_DIR}/bin/${PEM_CERT_NAME})
endif (WIN32)

file(DOWNLOAD ${CACERT_URL_SOURCE} ${PEM_CERT_DEST})

######################
# add Publish target #
######################

include(${CMAKE_CURRENT_SOURCE_DIR}/CMakePublish.cmake)