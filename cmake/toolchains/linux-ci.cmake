#We want Release with debug infos
SET(CMAKE_BUILD_TYPE Release)

list(APPEND CMAKE_PREFIX_PATH 
    usr/
    usr64/
)

SET (CMAKE_C_COMPILER             "clang")
SET (CMAKE_CXX_COMPILER             "clang++")