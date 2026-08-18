# CacheClient

A multi-language client library ecosystem for [CacheCore](https://github.com/guru-jadhav/CacheCore) — a Redis-inspired in-memory key-value store built from scratch in C++17.

CacheClient handles TCP connection management, RESP protocol encoding/decoding, multi-DB routing, and complex type serialization — so callers work with native types, not raw protocol bytes.

---

## Repository Structure

```
CacheClient/
├── CPP/        # C++17 client library (synchronous, E2E integration tested)
└── Java/       # Java 11 client library (synchronous, JUnit 5 tested)
```

Each subdirectory is a self-contained project with its own build system, test suites, and documentation.

---

## Language Clients

### C++ Client (`CPP/`)

> **Status:** Complete (Stable)

**Target:** C++17 · CMake 3.20+ · Linux

**Supported Features:**
- Synchronous (blocking) API.
- Native RESP protocol encoder / decoder.
- Persistent TCP connection.
- Dual-stack DNS-name based server resolution (IPv4 & IPv6 fallback loop).
- Automatic serialization of C++ primitives and STL containers (`std::vector`, `std::list`, `std::set`, `std::queue`, `std::stack`).
- Raw string bypass APIs (`SETRAW`/`GETRAW`) for server-side atomic commands.
- Full CacheCore command coverage: `SET`, `GET`, `SETRAW`, `GETRAW`, `DEL`, `EXISTS`, `INCR`, `EXPIRE`, `CLEAR`, `PING`.

See [`CPP/README.md`](CPP/README.md) for build instructions, API references, and usage examples.

---

### Java Client (`Java/`)

> **Status:** Complete (Stable)

**Target:** Java 11+ · Maven 3.6+ · Linux

**Supported Features:**
- Synchronous (blocking) API.
- Persistent TCP socket connection.
- Dual-stack DNS-name based server resolution (`InetAddress` fallback loop).
- Automatic serialization of Java primitives and Collection interfaces (`List`, `Set`, `Queue`, `Stack`).
- Raw string bypass APIs (`SETRAW`/`GETRAW`) to initialize counters for server-side operations.
- Full CacheCore command coverage using C++ API naming parity (all-caps methods like `PING()`, `SET()`, `GET()`, `DEL()`, `EXISTS()`, `EXPIRE()`, `INCR()`, `CLEAR()`).
- Robust exception hierarchy mapping (`NetworkException`, `SerializeException`, `DeserializeException`).

See [`Java/README.md`](Java/README.md) for Maven setup, compiler targets, and usage examples.

---

## Protocol Compatibility

CacheClient targets **CacheCore's custom RESP dialect**:

- Every command includes a **DB index as the first argument** — stateless multi-DB routing, no stateful `SELECT` command needed.
- All values are stored as strings at the wire level — complex types are serialized client-side.
- Wire format follows standard RESP (Redis Serialization Protocol) framing.

Example wire frame (SET on DB 0):
```
*5\r\n$1\r\n0\r\n$3\r\nSET\r\n$3\r\nfoo\r\n$3\r\nbar\r\n$1\r\n1\r\n
```

---

## Related

- [CacheCore](https://github.com/guru-jadhav/CacheCore) — The server this library connects to. Start there for protocol spec, command reference, and wire format examples.

---

## Roadmap

- [x] C++ v1 — synchronous API (Complete)
- [x] Java v1 — synchronous API (Complete)