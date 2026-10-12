cmake_minimum_required (VERSION 2.6)
project (test CXX)

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

include_directories("../bigdata/include")

file(GLOB test_SRC
    "include/*.h"
    "include/*.hh"

    "src/*.cpp"
    "src/*.cc"
    "src/*.cxx"
)

cmake_policy(SET CMP0015 NEW)

include_directories(src)
include_directories(/usr/local/include/coap)
link_directories(./)
# link_directories(../build-bigdata-Desktop_Qt_5_5_1_GCC_64bit-Debug)
link_directories(../build)
#link_directories(/usr/local/lib)


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../build)

add_executable(test ${test_SRC})
TARGET_LINK_LIBRARIES(test bigdata boost_log boost_log_setup gmp gmpxx boost_unit_test_framework boost_system boost_thread boost_program_options pthread coap-1)


