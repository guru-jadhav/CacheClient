# CacheClient

A multi-language client library ecosystem for [CacheCore](https://github.com/guru-jadhav/CacheCore) — a Redis-inspired in-memory key-value store built from scratch in C++17.

CacheClient handles TCP connection management, RESP protocol encoding/decoding, multi-DB routing, and complex type serialization — so callers work with native types, not raw protocol bytes.

---

## Repository Structure

```
CacheClient/
├── CPP/        # C++17 client library (synchronous v1, connection pooling)
└── Java/       # Java client library (coming soon)
```

Each subdirectory is a self-contained project with its own build system, tests, and documentation.

---

## Language Clients

### C++ — `CPP/`

> **Status:** In active development

**Target:** C++17 · CMake · Linux

**Planned features:**
- Synchronous (blocking) API — v1
- RESP encoder / decoder
- Persistent TCP connection with auto-reconnect
- Connection pooling
- Domain-name based server resolution
- Full coverage of CacheCore commands: `SET`, `GET`, `SETRAW`, `GETRAW`, `DEL`, `EXISTS`, `INCR`, `EXPIRE`, `CLEAR`, `PING`

See [`CPP/README.md`](CPP/README.md) for build instructions, API reference, and usage examples.

---

### Java — `Java/`

> **Status:** Planned

See [`Java/README.md`](Java/README.md) for status updates.

---

## Protocol Compatibility

CacheClient targets **CacheCore's custom RESP dialect**:

- Every command includes a **DB index as the first argument** — stateless multi-DB routing, no `SELECT` command needed.
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

- [ ] C++ v1 — synchronous API
- [ ] Java v1 — synchronous API