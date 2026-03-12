cmake_minimum_required(VERSION 3.30)
project(
  project
  VERSION 0.0.0
  LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
find_package(doctest REQUIRED) # Testing framework
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Werror
)

set(name ${PROJECT_NAME})

add_executable(${name} ${name}.cpp)
target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# target_link_libraries(${name} Threads::Threads)

include(GNUInstallDirs)
install(TARGETS ${name} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
