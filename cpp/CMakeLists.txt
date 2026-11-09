cmake_minimum_required(VERSION 3.16)
project(retro_os_cpp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
add_compile_options(-Wall -Wextra)

# Hardening, so the binary doesn't depend on what the distro's compiler enables by
# default. Each flag is only used if the toolchain accepts it.
include(CheckCXXCompilerFlag)
include(CheckPIESupported)
check_pie_supported(LANGUAGES CXX)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
foreach(flag -fstack-protector-strong -fstack-clash-protection -fcf-protection)
  string(MAKE_C_IDENTIFIER "HAVE${flag}" have)
  check_cxx_compiler_flag(${flag} ${have})
  if(${have})
    add_compile_options(${flag})
  endif()
endforeach()
add_link_options(-Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack)

# ASan + UBSan build for testing: cmake -DRETRO_SANITIZE=ON -DCMAKE_BUILD_TYPE=Debug
option(RETRO_SANITIZE "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
if(RETRO_SANITIZE)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
elseif(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  # _FORTIFY_SOURCE needs optimisation and doesn't mix with the sanitizers.
  add_compile_options(-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2)
endif()

find_package(PkgConfig REQUIRED)
# Paint hands dropped .bmp files to SDL's BMP loader. Refuse SDL releases older than
# 2.0.20, which have known memory-safety bugs on that path (e.g. CVE-2021-33657).
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2>=2.0.20)
pkg_check_modules(SDL2_TTF REQUIRED IMPORTED_TARGET SDL2_ttf)

file(GLOB SOURCES CONFIGURE_DEPENDS src/*.cpp src/apps/*.cpp)
add_executable(retroos ${SOURCES})
target_link_libraries(retroos PRIVATE PkgConfig::SDL2 PkgConfig::SDL2_TTF)
