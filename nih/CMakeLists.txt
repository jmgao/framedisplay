cmake_minimum_required(VERSION 2.8.9)

project(mbaacc_extract)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fno-strict-aliasing")

if(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc")
endif()

include_directories(../deps/gsl-lite/include)

add_library(libmbaacc STATIC mbaacc_pack.cpp)
add_executable(mbaacc_extract mbaacc_extract.cpp)
target_link_libraries(mbaacc_extract libmbaacc)
