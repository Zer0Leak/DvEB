cmake_minimum_required (VERSION 2.6)
project (DvEBserver CXX)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3 -g0 -std=c++1y -march=x86-64")

file(GLOB dveb_SRC
    "src/dveb/*.h"
    "src/dveb/*.hh"
    "src/dveb/*.cpp"
    "src/dveb/*.cc"
    "src/dveb/*.cxx"
)

include_directories(src)
include_directories(src/dveb)

add_executable(DvEBserver ${dveb_SRC} src/DvEBserver.cc)
TARGET_LINK_LIBRARIES(DvEBserver boost_system boost_thread boost_program_options pthread)

