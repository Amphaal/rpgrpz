#policies first
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0076 NEW)
endif()

#autoupdatercore
add_library(autoupdatercore INTERFACE)

# source
SET(autoupdatercore_PATH QtAutoUpdater/src/autoupdatercore)

# platform dependant
if(WIN32)
    SET(autoupdatercore_adminauthoriser_FILE "adminauthoriser_win.cpp")
elseif(APPLE)
    SET(autoupdatercore_adminauthoriser_FILE "adminauthoriser_mac.cpp")
endif()

# sources
target_sources(autoupdatercore INTERFACE
    ${autoupdatercore_PATH}/simplescheduler.cpp
    ${autoupdatercore_PATH}/updaterbackend.cpp
    ${autoupdatercore_PATH}/updater.cpp
    ${autoupdatercore_PATH}/updateinstaller.cpp
    ${autoupdatercore_PATH}/updateinfo.cpp
    ${autoupdatercore_PATH}/processbackend.cpp
    ${autoupdatercore_PATH}/${autoupdatercore_adminauthoriser_FILE}
)

# headers
file(GLOB_RECURSE autoupdatercore_HEADERS ${autoupdatercore_PATH}/*.h)
SET(autoupdatercore_DEST_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/QtAutoUpdaterCore)

file(COPY ${autoupdatercore_HEADERS} DESTINATION ${autoupdatercore_DEST_HEADERS})
target_include_directories(autoupdatercore INTERFACE ${CMAKE_CURRENT_BINARY_DIR})