cmake_minimum_required(VERSION 3.19)
cmake_policy(VERSION 3.19...3.22)

set(MBED_TARGET
    LPC1768
    CACHE STRING "Mbed target name")
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_CROSSCOMPILING TRUE)
set(MBED_CREATE_PYTHON_VENV
    FALSE
    CACHE BOOL "Whether to create a python venv for mbed tools")

set(MBED_APP_JSON_PATH mbed_app.json5)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/mbed-os/tools/cmake)
list(APPEND CMAKE_MODULE_PATH
     ${CMAKE_CURRENT_SOURCE_DIR}/mbed-os/tools/cmake/upload_methods)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(mbed_toolchain_setup)

project(
  mbedCeSynth
  VERSION 0.1.0
  LANGUAGES C CXX ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

find_package(doctest REQUIRED)
include(mbed_project_setup)
include(UploadMethodManager)
add_subdirectory(mbed-os)
add_executable(${PROJECT_NAME} main.cpp Synth.cpp TH02Sensor.cpp
                               BME280Sensor.cpp)

target_compile_options(
  ${PROJECT_NAME}
  PRIVATE -Wall
          -Wextra
          -Wpedantic
          -Werror
          # Too much warning because of mbed code we don’t control -Wshadow
          # -Wno-error=shadow -Wconversion -Wno-error=conversion
          # -Wfloat-conversion -Wno-error=float-conversion
)
target_link_libraries(${PROJECT_NAME} mbed-os)
mbed_set_post_build(${PROJECT_NAME})
