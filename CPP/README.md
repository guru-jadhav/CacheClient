# CacheClient — C++

C++17 synchronous client library for CacheCore.

## Structure

```
CPP/
├── include/
│   ├── CacheClient.h       # public API
│   ├── TypeSerializer.h    # template serialization for STL containers and primitives
│   ├── RESPParser.h        # RESP encode/decode (in progress)
│   └── TCPClient.h         # TCP connection management (in progress)
└── src/
    ├── CacheClient.cpp
    ├── TypeSerializer.cpp
    ├── RESPParser.cpp
    └── TCPClient.cpp
```

## Build

To compile the library locally and run the tests:
```bash
mkdir build && cd build
cmake ..
make
```

## Integration

### CMake FetchContent (Recommended)
You can include this library dynamically in your CMake project at build time. Add the following to your `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.20)
project(MyApp)

# 1. Import FetchContent
include(FetchContent)

# 2. Declare the dependency
FetchContent_Declare(
    CacheClient
    GIT_REPOSITORY https://github.com/guru-jadhav/CacheClient.git
    GIT_TAG        v1.0.0
    SOURCE_SUBDIR  CPP # Tells CMake the C++ project is in the CPP/ subdirectory
)

# 3. Make the library targets available
FetchContent_MakeAvailable(CacheClient)

# 4. Define your executable and link against it
add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE CacheClient)
```

### Method 2: System-wide Installation (find_package)
Alternatively, you can clone, compile, and install the library globally to your system:

```bash
git clone https://github.com/guru-jadhav/CacheClient.git
cd CacheClient/CPP
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
make
sudo make install
```

Once installed, locate and link it in your project's `CMakeLists.txt`:
```cmake
cmake_minimum_required(VERSION 3.20)
project(MyApp)

# 1. Locate the installed library configuration
find_package(CacheClient REQUIRED)

# 2. Define your executable and link against the CacheClient target
add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE CacheClient::CacheClient)
```

## API Documentation

### Connection Setup

```cpp
#include "CacheClient.h"

CacheClient cache("localhost", 6948);
if (!cache.connect()) {
    // handle connection failure
}
```

---

### Command Reference & Examples

#### `PING`
*   **Description**: Sends a ping to verify connection health.
*   **Signature**: `std::string PING()`
*   **Example**:
    ```cpp
    std::string pong = cache.PING(); // returns "PONG"
    ```

#### `SET` (Typed)
*   **Description**: Serializes and stores native C++ types (primitives, `std::vector`, `std::set`, `std::queue`, `std::stack`, etc.) in the database.
*   **Signature**: `template<typename T> bool SET(const unsigned int DB, const std::string& key, const T& value, const bool willExpire = true)`
*   **Example**:
    ```cpp
    std::vector<std::string> items = {"apple", "banana"};
    bool success = cache.SET(0, "grocery_list", items, false);
    ```

#### `GET` (Typed)
*   **Description**: Retrieves and deserializes a database value back into its native C++ representation.
*   **Signature**: `template<typename T> std::optional<T> GET(const unsigned int DB, const std::string& key)`
*   **Example**:
    ```cpp
    std::optional<std::vector<std::string>> items = cache.GET<std::vector<std::string>>(0, "grocery_list");
    if (items) {
        // use *items
    }
    ```

#### `SETRAW`
*   **Description**: Stores a raw, unserialized string directly. Used when initializing keys targeted by server-side commands (like `INCR`).
*   **Signature**: `bool SETRAW(const unsigned int DB, const std::string& key, const std::string& value, const bool willExpire = true)`
*   **Example**:
    ```cpp
    bool success = cache.SETRAW(0, "user_counter", "0", false);
    ```

#### `GETRAW`
*   **Description**: Retrieves the raw database string directly without deserializing it.
*   **Signature**: `std::optional<std::string> GETRAW(const unsigned int DB, const std::string& key)`
*   **Example**:
    ```cpp
    std::optional<std::string> count = cache.GETRAW(0, "user_counter"); // "0"
    ```

#### `DEL`
*   **Description**: Deletes a key from the database.
*   **Signature**: `bool DEL(const unsigned int DB, const std::string& key)`
*   **Example**:
    ```cpp
    bool deleted = cache.DEL(0, "grocery_list"); // returns true if deleted
    ```

#### `EXISTS`
*   **Description**: Checks key existence.
*   **Signature**: `bool EXISTS(const unsigned int DB, const std::string& key)`
*   **Example**:
    ```cpp
    bool exists = cache.EXISTS(0, "grocery_list");
    ```

#### `EXPIRE`
*   **Description**: Sets a Time-To-Live timeout (in seconds) on a key.
*   **Signature**: `bool EXPIRE(const unsigned int DB, const std::string& key, const size_t duration)`
*   **Example**:
    ```cpp
    bool ok = cache.EXPIRE(0, "grocery_list", 300); // expires in 5 minutes
    ```

#### `INCR`
*   **Description**: Atomically increments the integer value of a key. If the key does not exist, it is initialized to `"1"`. The key must contain a raw, unserialized integer string.
*   **Signature**: `long long INCR(const unsigned int DB, const std::string& key)`
*   **Example**:
    ```cpp
    // Initialize raw counter and increment it
    cache.SETRAW(0, "hits", "10", false);
    long long current_hits = cache.INCR(0, "hits"); // returns 11
    ```

#### `CLEAR`
*   **Description**: Clears all keys in the specified database.
*   **Signature**: `bool CLEAR(const unsigned int DB)`
*   **Example**:
    ```cpp
    bool ok = cache.CLEAR(0); // clears database 0
    ```

---

## Supported Serialization Types

The C++ client uses template metaprogramming to resolve type serialization at compile-time. Below is the list of supported types.

### Primitive Types
*   **`int`** (wire name: `"int"`)
*   **`long long`** (wire name: `"long long"`)
*   **`float`** (wire name: `"float"`)
*   **`double`** (wire name: `"double"`)
*   **`bool`** (wire name: `"bool"`, serialized as `"1"` or `"0"`)
*   **`char`** (wire name: `"char"`)
*   **`std::string`** (wire name: `"string"`)

### Container Types
*   **`std::vector<T>`** (wire name: `"vector"`)
*   **`std::list<T>`** (wire name: `"list"`)
*   **`std::set<T>`** (wire name: `"set"`)
*   **`std::multiset<T>`** (wire name: `"multiset"`)
*   **`std::unordered_set<T>`** (wire name: `"unordered_set"`)
*   **`std::unordered_multiset<T>`** (wire name: `"unordered_multiset"`)
*   **`std::queue<T>`** (wire name: `"queue"`)
*   **`std::stack<T>`** (wire name: `"stack"`)
*   **`std::string`** (wire name: `"string"`, supports collection-style serialization)

> [!NOTE]
> For any container type, the element type `T` must be one of the supported C++ primitive types listed above. Nesting containers (e.g., `std::vector<std::vector<int>>`) is not supported by the wire format serializer.