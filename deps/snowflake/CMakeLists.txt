add_library(snowflake INTERFACE)
target_include_directories(snowflake INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(snowflake INTERFACE snowflake.cpp)