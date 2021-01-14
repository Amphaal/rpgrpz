#policies first
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0076 NEW)
endif()

add_library(snowflake INTERFACE)
target_include_directories(snowflake INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(snowflake INTERFACE snowflake.cpp)