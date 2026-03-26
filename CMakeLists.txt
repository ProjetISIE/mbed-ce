cmake_minimum_required(VERSION 3.19)
cmake_policy(VERSION 3.19...3.22)

set(MBED_APP_JSON_PATH mbed_app.json5)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
# include(mbed-os/tools/cmake/mbed_toolchain_setup.cmake)

project(
  mbedCeSynth
  VERSION 0.1.0
  LANGUAGES C CXX ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Mbed OS is designed for C++17 and C11 with GNU extensions
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

find_package(doctest REQUIRED) # Testing framework
include(mbed_project_setup)
add_subdirectory(mbed-os)
add_executable(${PROJECT_NAME} main.cpp)

target_compile_options(${PROJECT_NAME} PRIVATE 
    -Wall -Wextra -Wpedantic -Werror
    -Wshadow -Wno-error=shadow
    -Wconversion -Wno-error=conversion
    -Wfloat-conversion -Wno-error=float-conversion
)
target_link_libraries(${PROJECT_NAME} mbed-os)
mbed_set_post_build(${PROJECT_NAME})
