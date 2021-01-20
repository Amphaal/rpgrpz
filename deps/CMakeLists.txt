#policies first
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0076 NEW)
endif()

#autoupdatercore
add_library(autoupdatercore INTERFACE)

# link deps
target_link_libraries(autoupdatercore INTERFACE
    Qt5::CorePrivate
    Advapi32 
    Ole32 
    Shell32
)

# source
SET(autoupdatercore_PATH QtAutoUpdater/src/autoupdatercore)

# sources
target_sources(autoupdatercore INTERFACE
    ${autoupdatercore_PATH}/simplescheduler.cpp
    ${autoupdatercore_PATH}/updaterbackend.cpp
    ${autoupdatercore_PATH}/updater.cpp
    ${autoupdatercore_PATH}/updateinstaller.cpp
    ${autoupdatercore_PATH}/updateinfo.cpp
    ${autoupdatercore_PATH}/processbackend.cpp
    ${autoupdatercore_PATH}/adminauthoriser_dummy.cpp
)

add_compile_definitions(QT_STATIC)

# headers
file(GLOB_RECURSE autoupdatercore_HEADERS ${autoupdatercore_PATH}/*.h)
SET(autoupdatercore_DEST_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/QtAutoUpdaterCore)

file(COPY ${autoupdatercore_HEADERS} DESTINATION ${autoupdatercore_DEST_HEADERS})

target_include_directories(autoupdatercore 
    INTERFACE 
        ${CMAKE_CURRENT_BINARY_DIR}
        ${autoupdatercore_DEST_HEADERS}
)

# plugins
file(GLOB_RECURSE autoupdatercore_plugins_HEADERS QtAutoUpdater/src/plugins/updaters/qtifw/*.h)

target_sources(autoupdatercore INTERFACE
    QtAutoUpdater/src/plugins/updaters/qtifw/qtifwupdaterbackend.cpp
    QtAutoUpdater/src/plugins/updaters/qtifw/qtifwupdaterplugin.cpp
)

target_include_directories(autoupdatercore 
    INTERFACE 
        QtAutoUpdater/src/plugins/updaters/qtifw
)