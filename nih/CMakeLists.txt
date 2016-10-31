cmake_minimum_required(VERSION 2.8.9)

project(mbaacc_extract)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++14 -fno-strict-aliasing")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color -g")
if(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -D__USE_MINGW_ANSI_STDIO=1")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc")
endif()

include_directories(../deps/gsl-lite/include)

add_library(mbaacc_pack STATIC mbaacc_pack.cpp)
add_executable(mbaacc_extract mbaacc_extract.cpp)
target_link_libraries(mbaacc_extract mbaacc_pack)
