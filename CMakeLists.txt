cmake_minimum_required(VERSION 3.22.1)
project(archiver)

set(CMAKE_CXX_STANDARD 23)
# set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)

add_executable(archiver main.cpp)

add_compile_definitions(LOCAL)
