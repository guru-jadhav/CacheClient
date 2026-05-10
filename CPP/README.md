# CacheClient — C++

C++17 synchronous client library for CacheCore.

## Structure
CPP/
├── include/
│   ├── CacheClient.h      # public API
│   ├── TypeSerializer.h   # template serialization for STL containers and primitives
│   ├── RESPParser.h       # RESP encode/decode (in progress)
│   └── TCPClient.h        # TCP connection management (in progress)
└── src/
    ├── CacheClient.cpp
    ├── TypeSerializer.cpp
    ├── RESPParser.cpp
    └── TCPClient.cpp

## Build
mkdir build && cd build
cmake ..
make

## Requirements
- C++17
- CMake 3.20+
- Linux