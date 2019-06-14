project(RPGRPZ)
cmake_minimum_required(VERSION 3.13.3)

#version
SET(CMAKE_PROJECT_VERSION_MAJOR 0)
SET(CMAKE_PROJECT_VERSION_MINOR 14)
SET(CMAKE_PROJECT_VERSION_PATCH 2)


#default configuration
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)

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

##################
## Enable Conan ##
##################

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(KEEP_RPATHS)

################################
## CPP Compiler Configuration ##
################################

SET(CMAKE_CXX_STANDARD 17)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS) #upnpc
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

SET(BUNDLE_TYPE WIN32) #set bundle type
SET(RPGRPZ_ICO ${CMAKE_CURRENT_SOURCE_DIR}/resources/resources.rc) #add icon

#default source contains
SET(SOURCES
    ${RPGRPZ_ICO}
    resources/resources.qrc
)

###########
# QT libs #
###########

#search path
SET(Qt5_DIR "C:/Qtd/5.12.3/msvc2017_64/lib/cmake/Qt5")

#find package
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

#pkgConfig setup
SET(PKG_CONFIG_EXECUTABLE "C:\\msys64\\mingw64\\bin\\x86_64-w64-mingw32-pkg-config.exe")

#weak linking
SET(GSTREAMER_LOCATION "D:/gstreamer/1.0/x86_64")
include(GStreamerHelper)
LinkGStreamer()

#append librairies for later
list(APPEND EXT_LIBS 
    ${GST_STATIC_LIBRARIES}
    ${GST_PLUGINS_BASE_STATIC_LIBRARIES}
)
    
#################################
# mandatory explicit generation #
#################################

#private libs
file(GLOB_RECURSE INT_LIBS
    "libs/*"
)

list(APPEND SOURCES 
    src/helpers/_appContext.cpp
)

#QThread derivatives
list(APPEND SOURCES
    src/network/uPnP/uPnPThread.h
)

#functionnal components
list(APPEND SOURCES
    src/network/rpz/_any/JSONSocket.cpp
    src/network/rpz/client/RPZClient.cpp
    src/network/rpz/server/RPZServer.cpp

    src/network/stream/GStreamerClient.cpp

    src/network/uPnP/uPnPRequester.cpp
    src/network/ConnectivityHelper.cpp
)

#ui components
list(APPEND SOURCES
    src/shared/models/RPZAtom.cpp

    src/ui/map/graphics/MapViewGraphicsScene.hpp
    
    src/shared/commands/RPZActions.cpp
    src/shared/commands/AtomsContextualMenuHandler.cpp
    src/shared/commands/MessageInterpreter.cpp
    
    src/shared/atoms/AtomsHandler.cpp
    src/shared/atoms/AtomsStorage.cpp
    
    src/shared/atoms/ViewMapHint.cpp
    src/shared/atoms/TreeMapHint.cpp

    src/ui/map/base/AnimationTimeLine.hpp
    src/ui/map/MapTools.cpp
    src/ui/map/MapView.cpp
    src/ui/atoms/base/AtomEditorLineDescriptor.cpp
    src/ui/atoms/base/AtomSubEditor.cpp
    src/ui/atoms/editors/AtomSliderEditor.cpp
    src/ui/atoms/AtomEditor.cpp
    src/ui/layout/base/LayerSelector.cpp
    src/ui/layout/MapLayoutTree.cpp
    src/ui/layout/MapLayoutManager.cpp

    src/shared/database/base/JSONDatabase.cpp
    src/shared/database/AssetsDatabase.cpp
    src/shared/assets/AssetsDatabaseElement.cpp
    
    src/ui/assets/base/AssetsTreeViewModel.cpp
    src/ui/assets/base/AssetsTreeView.cpp
    src/ui/assets/AssetsManager.cpp

    src/ui/others/RestoringSplitter.cpp
    src/ui/others/statusbar/RPZStatusLabel.hpp
    src/ui/others/statusbar/RPZStatusBar.cpp
    src/ui/others/UpdaterUIIntegrator.hpp

    src/ui/communication/ChatEdit.cpp
    src/ui/communication/logs/base/LogContainer.cpp
    src/ui/communication/logs/base/LogScrollView.cpp
    src/ui/communication/logs/UsersLog.hpp
    src/ui/communication/ChatWidget.cpp
    src/ui/communication/ConnectWidget.cpp

    src/ui/communication/audio/PlaylistItem.hpp
    src/ui/communication/audio/Playlist.cpp
    src/ui/communication/audio/AudioStreamToolbar.hpp
    src/ui/communication/audio/PlaylistToolbar.hpp
    src/ui/communication/audio/PlaylistController.cpp
    src/ui/communication/audio/AudioStreamController.cpp
)

#main
list(APPEND SOURCES 
    src/ui/MainWindow.cpp
    tests/TestMainWindow.cpp
    src/main.cpp
)

#########################
# Executable Generation #
#########################

# Create executable
add_executable(${PROJECT_NAME} ${BUNDLE_TYPE} ${INT_LIBS} ${SOURCES}) 

# Bind ext libs
conan_target_link_libraries(${PROJECT_NAME})
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

#copy gstreamer .dlls
set(_gstreamer_plugins "all")
DeployGStreamer(${PROJECT_NAME} "${_gstreamer_plugins}")
    

######################
# add Publish target #
######################

include(CMakePublish)

#include(PrintAllVariables)