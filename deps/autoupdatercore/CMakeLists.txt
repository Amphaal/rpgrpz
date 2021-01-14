#policies first
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0076 NEW)
endif()

add_library(autoupdatercore INTERFACE)
target_include_directories(autoupdatercore INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(autoupdatercore INTERFACE 
    simplescheduler.cpp
    updater_p.cpp
    updater.cpp
)