#pragma once
#include <cstdint>
#include <string>
#include <type_traits>
#include <optional>
#include "TypeSerializer.h"
#include "TCPClient.h"
#include "RESPParser.h"
#include "Exceptions.h"

/**
 * @brief Main client class providing the public CacheCore API endpoints.
 * Handles DNS fallback resolution, RESP command serialization, type conversion,
 * and connection resource lifecycle management.
 */
class CacheClient {
    std::string domain;
    std::uint16_t port;
    TCPClient client;

public:

    /**
     * @brief Instantiates a new CacheClient for the specified domain and port.
     * Note: This does not automatically connect to the server. Call connect() to open the socket.
     * 
     * @param _domain The host domain or IP address of the CacheCore server.
     * @param _port The port number of the CacheCore server.
     */
    CacheClient(const std::string& _domain, const std::uint16_t _port);

    /**
     * @brief Destructor. Automatically calls close() to release system socket and port resources.
     */
    ~CacheClient();

    /**
     * @brief Explicitly closes the underlying socket connection.
     * Safe to call multiple times (idempotent).
     */
    void close();

    /**
     * @brief Resolves target address via DNS and opens a TCP socket connection.
     * Supports DNS lookup fallback loops.
     * 
     * @return bool - true if connection was established successfully
     * @throw NetworkException if DNS resolution fails or connection cannot be established
     */
    bool connect();

    /**
     * @brief Pings the CacheCore server to verify connection health.
     * 
     * @return std::string - "PONG" on success
     * @throw NetworkException if a network error occurs
     * @throw DeserializeException if the server returns an error response
     */
    std::string PING();

    /**
     * @brief Deletes a key from the specified database.
     * 
     * @param DB The database index (0, 1, or 2)
     * @param _key The key to delete
     * @return bool - true if the key was deleted, false if it did not exist
     * @throw NetworkException if a network error occurs
     * @throw DeserializeException if the server returns an error response
     */
    bool DEL(const unsigned int DB, const std::string& _key);

    /**
     * @brief Checks if a key exists in the specified database.
     * 
     * @param DB The database index (0, 1, or 2)
     * @param _key The key to check
     * @return bool - true if the key exists, false otherwise
     * @throw NetworkException if a network error occurs
     * @throw DeserializeException if the server returns an error response
     */
    bool EXISTS(const unsigned int DB, const std::string& _key);

    /**
     * @brief Clears all keys in the specified database.
     * 
     * @param DB The database index (0, 1, or 2) to clear
     * @return bool - true on successful clear
     * @throw NetworkException if a network error occurs
     * @throw DeserializeException if the server returns an error response
     */
    bool CLEAR(const unsigned int DB);

    /**
     * @brief Sets a time-to-live timeout (in seconds) on a key in the database.
     * Note: CacheCore clamping rules may apply to minimum TTL.
     * 
     * @param DB The database index (0, 1, or 2)
     * @param _key The key to expire
     * @param duration The TTL duration in seconds
     * @return bool - true on success
     * @throw NetworkException if a network error occurs
     * @throw DeserializeException if the server returns an error response
     */
    bool EXPIRE(const unsigned int DB, const std::string& _key, const size_t duration);

    /**
     * @brief Atomically increments the integer value of a key.
     * If the key does not exist, it is initialized to 1.
     * 
     * @param DB The database index (0, 1, or 2)
     * @param _key The key whose value to increment
     * @return long long - the value after the increment
     * @throw NetworkException if a network error occurs
     * @throw DeserializeException if the value cannot be parsed as an integer, or the server returns an error
     */
    long long INCR(const unsigned int DB, const std::string& _key);

    /**
     * @brief Stores a raw, unserialized string value in CacheCore.
     * 
     * @param DB The database index (0, 1, or 2)
     * @param _key The key to associate the raw value with
     * @param _value The raw string value to store
     * @param _willExpire If true, the key will be subject to eviction rules
     * @return bool - true on success
     * @throw NetworkException if a network error occurs
     * @throw DeserializeException if the server returns an error response
     */
    bool SETRAW(const unsigned int DB, const std::string& _key, const std::string& _value, const bool _willExpire = true);

    /**
     * @brief Retrieves a raw, unserialized string value from CacheCore.
     * 
     * @param DB The database index (0, 1, or 2)
     * @param _key The key to look up
     * @return std::optional<std::string> - raw string value, or std::nullopt if key does not exist
     * @throw NetworkException if a network error occurs
     * @throw DeserializeException if the server returns an error response
     */
    std::optional<std::string> GETRAW(const unsigned int DB, const std::string& _key);

    /**
     * @brief Serializes and stores a value (primitive or container) in CacheCore.
     * Accepts arithmetic primitives, std::string, and standard STL containers.
     * 
     * @tparam T The type of the value to store.
     * @param DB The database index (0, 1, or 2).
     * @param _key The key to associate the serialized value with.
     * @param _value The value to serialize and store.
     * @param _willExpire If true, the key will be subject to eviction rules.
     * @return bool - true on success.
     * @throw NetworkException if a network error occurs.
     * @throw DeserializeException if serialization fails or the server returns an error.
     */
    template<typename T>
    bool SET(const unsigned int DB, const std::string& _key, const T& _value, const bool _willExpire = true) {
        std::string formatted;
        if constexpr (std::is_arithmetic_v<T>) {
            static_assert(TypeName<T>::supported, "Unsupported primitive type");
            formatted = Serializer::serializePrimitive(_value);
        } else {
            static_assert(ContainerName<T>::supported, "Unsupported container type");
            formatted = Serializer::serializeContainer(_value);
        }

        RESPRequest req;
        req.cmd      = "SET";
        req.dbIndex  = DB;
        req.key      = _key;
        req.value    = formatted;
        req.expires  = _willExpire;

        std::string raw = client.SEND(RESPParser::encode(req));
        RESPResponse resp = RESPParser::decode(raw);
        if (resp.isError) {
            throw DeserializeException(resp.value);
        }
        return resp.value == "1";
    }

    /**
     * @brief Retrieves a value from CacheCore and deserializes it into type T.
     * Caller specifies the expected type, e.g., cache.GET<std::vector<int>>(0, "key").
     * 
     * @tparam T The target type for deserialization.
     * @param DB The database index (0, 1, or 2).
     * @param _key The key to look up.
     * @return std::optional<T> - deserialized value, or std::nullopt if the key does not exist.
     * @throw NetworkException if a network error occurs.
     * @throw DeserializeException if deserialization fails or the server returns an error.
     */
    template<typename T>
    std::optional<T> GET(const unsigned int DB, const std::string& _key) {
        RESPRequest req;
        req.cmd     = "GET";
        req.dbIndex = DB;
        req.key     = _key;

        std::string raw = client.SEND(RESPParser::encode(req));
        RESPResponse resp = RESPParser::decode(raw);

        if (resp.isError) {
            throw DeserializeException(resp.value);
        }
        if (resp.isNull)  {
            return std::nullopt;
        }

        if constexpr (std::is_arithmetic_v<T>) {
            auto result = Serializer::deserializePrimitive<T>(resp.value);
            if (!result) {
                throw DeserializeException("failed to deserialize primitive");
            }
            return *result;
        } else {
            auto result = Serializer::deserializeContainer<T>(resp.value);
            if (!result) {
                throw DeserializeException("failed to deserialize container");
            }
            return *result;
        }
    }
};