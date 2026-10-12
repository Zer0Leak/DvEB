cmake_minimum_required (VERSION 2.6)
project (bigdata CXX)

#SET(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_BUILD_TYPE Release)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++1y -march=x86-64")

#SET (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb3" )
#SET (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g0" )

SET (CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb3" )
SET (CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -g0" )

message("CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
message("CMAKE_CXX_FLAGS_DEBUG is ${CMAKE_CXX_FLAGS_DEBUG}")
message("CMAKE_CXX_FLAGS_RELEASE is ${CMAKE_CXX_FLAGS_RELEASE}")

ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)

include_directories("include")

file(GLOB bigdata_SRC
    "include/bigdata/*.h"
    "include/bigdata/*.hh"

    "include/bigdata/veb/*.h"
    "include/bigdata/veb/*.hh"

    "src/veb/*.cpp"
    "src/veb/*.cc"
    "src/veb/*.cxx"

    "src/*.cpp"
    "src/*.cc"
    "src/*.cxx"
)

include_directories(src/util)
include_directories(src/veb)
include_directories(/usr/local/include/coap)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../build)

add_library(bigdata SHARED ${bigdata_SRC})
# TARGET_LINK_LIBRARIES(DvEB gmp boost_system boost_thread boost_program_options pthread)
