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

```bash
mkdir build && cd build
cmake ..
make
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