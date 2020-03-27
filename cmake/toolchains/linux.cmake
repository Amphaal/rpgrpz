#We want Release with debug infos
SET(CMAKE_BUILD_TYPE RelWithDebInfo)

SET (CMAKE_C_COMPILER             "clang")
SET (CMAKE_C_FLAGS                "-Wall -Wno-unused-parameter")
SET (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")

SET (CMAKE_CXX_COMPILER             "clang++")
SET (CMAKE_CXX_FLAGS                "-Wall -Wno-unused-parameter")
SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")