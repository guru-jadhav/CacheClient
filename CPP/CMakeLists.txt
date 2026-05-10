cmake_minimum_required(VERSION 3.20)

project(CacheClient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCES src/*.cpp)

add_library(CacheClient ${SOURCES})

target_include_directories(CacheClient PRIVATE ${PROJECT_SOURCE_DIR}/include)