cmake_minimum_required(VERSION 3.16)
project(retro_os_cpp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
add_compile_options(-Wall -Wextra)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
pkg_check_modules(SDL2_TTF REQUIRED IMPORTED_TARGET SDL2_ttf)

file(GLOB SOURCES CONFIGURE_DEPENDS src/*.cpp src/apps/*.cpp)
add_executable(retroos ${SOURCES})
target_link_libraries(retroos PRIVATE PkgConfig::SDL2 PkgConfig::SDL2_TTF)
