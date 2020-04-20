#We want Release with debug infos
SET (CMAKE_BUILD_TYPE Release)

SET (CMAKE_C_COMPILER   "clang")
SET (CMAKE_CXX_COMPILER "clang++")

SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
