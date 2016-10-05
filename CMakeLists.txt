cmake_minimum_required(VERSION 2.8.9)

project(framedisplay)
set(SRCS
  src/abk_framedata.cpp
  src/abk_framedisplay.cpp
  src/abk_images.cpp
  src/abk_packfiles.cpp
  src/clone.cpp
  src/framedisplay.cpp
  src/mbaa_arx.cpp
  src/mbaacc_cg.cpp
  src/mbaacc_character.cpp
  src/mbaacc_framedata.cpp
  src/mbaacc_framedisplay.cpp
  src/mbaacc_pack.cpp
  src/mbaa_cg2.cpp
  src/mbaa_character.cpp
  src/mbaa_framedata.cpp
  src/mbaa_framedisplay.cpp
  src/mbaa_iso.cpp
  src/mbaa_tileimage.cpp
  src/ougon_data.cpp
  src/ougon_framedata.cpp
  src/ougon_framedisplay.cpp
  src/render.cpp
  src/texture.cpp
  src/touhou_framedata.cpp
  src/touhou_framedisplay.cpp
  src/touhou_images.cpp
  src/touhou_packfiles.cpp
)

if(WIN32)
  list(APPEND SRCS src/windows_gui.cpp)
else()
  list(APPEND SRCS src/simple_gui.cpp)
endif()

find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})

if(NOT WIN32)
  find_package(SDL REQUIRED)
  include_directories(${SDL_INCLUDE_DIRS})
  link_directories(${SDL_LIBRARY_DIRS})
endif()

# Vendored libpng/zlib
add_subdirectory(deps)
include_directories(
  ${CMAKE_BINARY_DIR}/deps/libpng
  ${CMAKE_SOURCE_DIR}/deps/libpng
  ${CMAKE_BINARY_DIR}/deps/zlib
  ${CMAKE_SOURCE_DIR}/deps/zlib
)

add_executable(framedisplay ${SRCS})

target_link_libraries(framedisplay
  ${OPENGL_LIBRARIES}
  png_static
  zlibstatic
)

if(NOT WIN32)
  target_link_libraries(framedisplay SDL)
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc")
  target_link_libraries(framedisplay opengl32 comctl32 ole32)
endif()
