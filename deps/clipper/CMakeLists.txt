#policies first
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0076 NEW)
endif()

add_library(clipper INTERFACE)
target_include_directories(clipper INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(clipper INTERFACE clipper.cpp)