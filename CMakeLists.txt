cmake_minimum_required(VERSION 3.19)
cmake_policy(VERSION 3.19...3.22)
project(
  project
  VERSION 0.1.0
  LANGUAGES C CXX ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
find_package(doctest REQUIRED) # Testing framework
add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror)

set(MBED_APP_JSON_PATH mbed_app.json5)
include(mbed-os/tools/cmake/mbed_toolchain_setup.cmake)
include(mbed_project_setup)

add_subdirectory(mbed-os)
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} mbed-os) # Could link to mbed-baremetal
mbed_set_post_build(${PROJECT_NAME})
