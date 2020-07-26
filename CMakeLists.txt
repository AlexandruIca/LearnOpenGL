cmake_minimum_required(VERSION 3.10)

project(LearnOpenGL LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR} ${CMAKE_PREFIX_PATH})

include(cmake/StandardProjectSettings.cmake)
include(cmake/CompilerWarnings.cmake)
include(cmake/Sanitizers.cmake)

add_library(project_options INTERFACE)
add_library(project::options ALIAS project_options)
target_compile_features(project_options INTERFACE cxx_std_17)

add_library(project_warnings INTERFACE)
add_library(project::warnings ALIAS project_warnings)
set_project_warnings(project_warnings)

enable_sanitizers(project_options)

find_package(SDL2 REQUIRED)
find_package(spdlog REQUIRED)
find_package(glad REQUIRED)
find_package(glm REQUIRED)
find_package(stb REQUIRED)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/)
