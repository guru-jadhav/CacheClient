# CacheClient — Java

A lightweight, blocking Java client library for CacheCore.

Supports dual-stack connection fallback, custom RESP wire framing, automatic serialization/deserialization of standard Java primitives and collections, and raw string bypasses.

---

## Installation & Build

Build the project and run tests using Maven:

```bash
# Compile source files
mvn compile

# Run the integration test suite
mvn test

# Package the library into a jar
mvn clean package
```

To include this library in a Maven project, add the JitPack repository and the dependency configuration to your `pom.xml`:

```xml
<repositories>
    <repository>
        <id>jitpack.io</id>
        <url>https://jitpack.io</url>
    </repository>
</repositories>

<dependencies>
    <dependency>
        <groupId>com.github.guru-jadhav</groupId>
        <artifactId>CacheClient</artifactId>
        <version>v1.0.0</version>
    </dependency>
</dependencies>
```

For Gradle projects, add the repository and dependency configuration to your `build.gradle`:

```groovy
repositories {
    mavenCentral()
    maven { url 'https://jitpack.io' }
}

dependencies {
    implementation 'com.github.guru-jadhav:CacheClient:v1.0.0'
}
```

For other build tools and configurations, see the [JitPack CacheClient Project Page](https://jitpack.io/#guru-jadhav/CacheClient).

---

## API Documentation

### Connection Setup

```java
import com.gurujadhav.cacheclient.CacheClient;

CacheClient cache = new CacheClient("localhost", 6948);
if (!cache.connect()) {
    // handle connection failure
}
```

---

### Command Reference & Examples

#### `PING`
*   **Description**: Sends a ping to verify connection health.
*   **Signature**: `String PING()`
*   **Example**:
    ```java
    String pong = cache.PING(); // returns "PONG"
    ```

#### `SET` (Typed)
*   **Description**: Serializes and stores native Java types (primitives and standard collection interfaces like `List`, `Set`, `Queue`, `Stack`) in the database.
*   **Signature**: `<T> boolean SET(int db, String key, T value, boolean willExpire)`
*   **Example**:
    ```java
    List<String> items = Arrays.asList("apple", "banana");
    boolean success = cache.SET(0, "grocery_list", items, false);
    ```

#### `GET` (Typed)
*   **Description**: Retrieves and deserializes a database value back into its native Java primitive or collection representation.
*   **Signatures**:
    *   `<T> Optional<T> GET(int db, String key, Class<T> type)` *(For primitives and String)*
    *   `<T> Optional<T> GET(int db, String key, Class<T> containerType, Class<?> elementType)` *(For collections)*
*   **Example**:
    ```java
    // Get primitive
    Optional<Integer> count = cache.GET(0, "user_count", Integer.class);

    // Get collection
    Optional<List> items = cache.GET(0, "grocery_list", List.class, String.class);
    if (items.isPresent()) {
        List<String> list = items.get();
    }
    ```

#### `SETRAW`
*   **Description**: Stores a raw, unserialized string directly. Useful for initializing numeric values for server-side operations (like `INCR`).
*   **Signature**: `boolean SETRAW(int db, String key, String value, boolean willExpire)`
*   **Example**:
    ```java
    boolean success = cache.SETRAW(0, "user_counter", "0", false);
    ```

#### `GETRAW`
*   **Description**: Retrieves the raw database string representation directly without attempting deserialization.
*   **Signature**: `Optional<String> GETRAW(int db, String key)`
*   **Example**:
    ```java
    Optional<String> count = cache.GETRAW(0, "user_counter"); // "0"
    ```

#### `DEL`
*   **Description**: Deletes a key from the database.
*   **Signature**: `boolean DEL(int db, String key)`
*   **Example**:
    ```java
    boolean deleted = cache.DEL(0, "grocery_list");
    ```

#### `EXISTS`
*   **Description**: Checks if a key exists and is not expired.
*   **Signature**: `boolean EXISTS(int db, String key)`
*   **Example**:
    ```java
    boolean exists = cache.EXISTS(0, "grocery_list");
    ```

#### `EXPIRE`
*   **Description**: Sets a Time-To-Live timeout (in seconds) on a key.
*   **Signature**: `boolean EXPIRE(int db, String key, int duration)`
*   **Example**:
    ```java
    boolean ok = cache.EXPIRE(0, "grocery_list", 300); // expires in 5 minutes
    ```

#### `INCR`
*   **Description**: Atomically increments the integer value of a key. If the key does not exist, it is initialized to `"1"`. The key must contain a raw, unserialized integer string.
*   **Signature**: `long INCR(int db, String key)`
*   **Example**:
    ```java
    cache.SETRAW(0, "hits", "10", false);
    long currentHits = cache.INCR(0, "hits"); // returns 11
    ```

#### `CLEAR`
*   **Description**: Clears all keys in the specified database.
*   **Signature**: `boolean CLEAR(int db)`
*   **Example**:
    ```java
    boolean ok = cache.CLEAR(0); // clears database 0
    ```


---

## Supported Serialization Types

The Java client automatically maps and serializes standard Java object wrappers and primitive types to align with the CacheCore server's wire format.

### Primitive Types
*   **`Integer` / `int`** (wire name: `"int"`)
*   **`Long` / `long`** (wire name: `"long long"`)
*   **`Float` / `float`** (wire name: `"float"`)
*   **`Double` / `double`** (wire name: `"double"`)
*   **`Boolean` / `boolean`** (wire name: `"bool"`, serialized as `"1"` or `"0"`)
*   **`Character` / `char`** (wire name: `"char"`)
*   **`String`** (wire name: `"string"`)

### Container Types
*   **`List<T>` / `ArrayList<T>`** (wire name: `"vector"`)
*   **`Set<T>` / `HashSet<T>`** (wire name: `"set"`)
*   **`Queue<T>` / `LinkedList<T>`** (wire name: `"queue"`)
*   **`Stack<T>`** (wire name: `"stack"`)
*   **`String`** (wire name: `"string"`, supports collection-style serialization)

> [!NOTE]
> For any collection container type, the element type `T` must be one of the supported Java primitive classes listed above. Nesting collections (e.g., `List<List<Integer>>`) is not supported by the wire format serializer.

---

## Exception Handling

All methods throw exceptions extending `CacheException` (which extends `RuntimeException` for clean caller interfaces):

*   **`NetworkException`**: Thrown on DNS resolution failures, socket timeouts, or server disconnects.
*   **`SerializeException`**: Thrown if type mapping or element conversions fail during serialization.
*   **`DeserializeException`**: Thrown if a received payload violates protocol framing, or if a raw payload cannot be successfully parsed into the requested type.