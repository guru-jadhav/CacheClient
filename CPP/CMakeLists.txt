cmake_minimum_required(VERSION 3.20)

project(CacheClient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCES src/*.cpp)

add_library(CacheClient ${SOURCES})
target_include_directories(CacheClient PUBLIC ${PROJECT_SOURCE_DIR}/include)

# E2E Integration Test Executable
add_executable(test_client test/test_client.cpp)
target_link_libraries(test_client PRIVATE CacheClient)