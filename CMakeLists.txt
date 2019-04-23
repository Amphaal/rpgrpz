project(RPGRPZ)
cmake_minimum_required(VERSION 3.13.3)

#version
SET(CMAKE_PROJECT_VERSION_MAJOR 0)
SET(CMAKE_PROJECT_VERSION_MINOR 8)
SET(CMAKE_PROJECT_VERSION_PATCH 7)


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
    SET(MAINTENANCE_TOOL_LOCATION "C:/Qt/MaintenanceTool")
	
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
SET(Qt5_DIR "C:/Qtd/5.12.2/msvc2017_64/lib/cmake/Qt5")

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
    SET(CMAKE_PREFIX_PATH "D:/gstreamer/1.0/x86_64")
    find_package(PkgConfig) 

    #required modules
    pkg_check_modules(GST REQUIRED 
        gstreamer-1.0
        gobject-2.0
    )
    
    #get the DLL lib location
    pkg_get_variable(GSTREAMER_DLL_LOCATION 
        "${CMAKE_PREFIX_PATH}/lib/pkgconfig/gstreamer-1.0.pc" 
        toolsdir)

    #linking
    link_directories(${GST_LIBRARY_DIRS})
    include_directories(${GST_INCLUDE_DIRS}) 
    list(APPEND EXT_LIBS 
        ${GST_LIBRARIES}
    )

#################################
# mandatory explicit generation #
#################################

#private libs
file(GLOB_RECURSE INT_LIBS
    "libs/*.c"
    "libs/*.cpp"
)

#QThread derivatives
list(APPEND SOURCES
    src/network/uPnP/uPnPThread.h
)

#functionnal components
list(APPEND SOURCES
    src/network/rpz/_any/JSONSocket.cpp
    src/network/audio/_any/AudioBase.hpp
    src/network/audio/client/AudioClient.cpp
    src/network/audio/server/AudioServer.cpp
    src/network/rpz/client/RPZClient.cpp
    src/network/rpz/server/RPZServer.cpp
    src/network/uPnP/uPnPRequester.cpp
    src/network/ConnectivityHelper.cpp
)

#ui components
list(APPEND SOURCES
    
    src/shared/map/MapHint.cpp
    src/shared/map/MapHintViewBinder.cpp

    src/ui/map/base/AssetsNavigator.cpp
    src/ui/map/base/AssetsPreviewer.cpp
    src/ui/map/base/AnimationTimeLine.hpp
    src/ui/map/MapTools.cpp
    src/ui/map/MapView.cpp
    src/ui/map/MapLayoutManager.cpp
    src/ui/map/AssetsManager.cpp

    src/ui/others/RPZStatusBar.hpp
    src/ui/others/UpdaterUIIntegrator.hpp

    src/ui/communication/ChatEdit.cpp
    src/ui/communication/logs/base/LogContainer.cpp
    src/ui/communication/logs/base/LogScrollView.cpp
    src/ui/communication/logs/UsersLog.hpp
    src/ui/communication/ChatWidget.cpp
    src/ui/communication/ConnectWidget.cpp

    src/ui/communication/audio/AudioStreamToolbar.hpp
    src/ui/communication/audio/PlaylistToolbar.hpp
    src/ui/communication/audio/PlaylistController.cpp
    src/ui/communication/audio/AudioStreamController.cpp
)

#main
list(APPEND SOURCES 
    src/ui/MainWindow.cpp
    src/main.cpp
)

#########################
# Executable Generation #
#########################

# Create executable
add_executable(${PROJECT_NAME} ${BUNDLE_TYPE} ${SOURCES} ${INT_LIBS}) 

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
include(Windeployqt)
windeployqt(${PROJECT_NAME} ${CMAKE_CURRENT_BINARY_DIR}/bin/)

#copy gstreamer .dlls
foreach(_gstLib IN ITEMS ${GST_LIBRARIES})
    
    #searched expression
    file(GLOB _se LIST_DIRECTORIES OFF
        "${GSTREAMER_DLL_LOCATION}/${_gstLib}*.dll"
    )
    get_filename_component(_se_dll ${_se} NAME)

    #copy lib
    add_custom_command(TARGET ${PROJECT_NAME} COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        ${_se} $<TARGET_FILE_DIR:${PROJECT_NAME}>/${_se_dll}
    )

endforeach()

######################
# add Publish target #
######################

include(CMakePublish)

#include(PrintAllVariables)