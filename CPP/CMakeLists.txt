cmake_minimum_required(VERSION 3.20)

project(CacheClient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCES src/*.cpp)

add_library(CacheClient ${SOURCES})
target_include_directories(CacheClient PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Target Installation & Exports Configuration
install(TARGETS CacheClient
    EXPORT CacheClientTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install public headers
install(DIRECTORY include/ DESTINATION include)

# Install target export definitions
install(EXPORT CacheClientTargets
    FILE CacheClientTargets.cmake
    NAMESPACE CacheClient::
    DESTINATION lib/cmake/CacheClient
)

# Install config helper file
install(FILES cmake/CacheClientConfig.cmake
    DESTINATION lib/cmake/CacheClient
)

# E2E Integration Test Executable
add_executable(test_client test/test_client.cpp)
target_link_libraries(test_client PRIVATE CacheClient)