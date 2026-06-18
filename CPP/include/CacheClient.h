#pragma once
#include <cstdint>
#include <string>
#include <type_traits>
#include <optional>
#include "TypeSerializer.h"
#include "TCPClient.h"
#include "RESPParser.h"
#include "Exceptions.h"

class CacheClient {
    std::string domain;
    std::uint16_t port;
    TCPClient client;

public:

    CacheClient(const std::string& _domain, const std::uint16_t _port) : domain(_domain), port(_port), client(_domain, _port) {}

    /**
     * @brief Checks if the client is connected to CacheCore.
     * @return bool - true if connected successfully
     */
    bool connect() {
        return client.connect();
    }

    /**
     * @brief Pings the CacheCore server to verify connection health.
     * @return std::string - "PONG" on success
     */
    std::string PING() {
        RESPRequest req;
        req.cmd = "PING";
        req.dbIndex = 0;
        std::string raw = client.SEND(RESPParser::encode(req));
        RESPResponse resp = RESPParser::decode(raw);
        if (resp.isError) {
            throw DeserializeException(resp.value);
        }
        return resp.value;
    }

    /**
     * @brief Deletes a key from the specified database.
     * @return bool - true if the key was deleted, false if it did not exist
     */
    bool DEL(const unsigned int DB, const std::string& _key) {
        RESPRequest req;
        req.cmd = "DEL";
        req.dbIndex = DB;
        req.key = _key;
        std::string raw = client.SEND(RESPParser::encode(req));
        RESPResponse resp = RESPParser::decode(raw);
        if (resp.isError) {
            throw DeserializeException(resp.value);
        }
        return resp.value == "1";
    }

    /**
     * @brief Checks if a key exists in the specified database.
     * @return bool - true if the key exists, false otherwise
     */
    bool EXISTS(const unsigned int DB, const std::string& _key) {
        RESPRequest req;
        req.cmd = "EXISTS";
        req.dbIndex = DB;
        req.key = _key;
        std::string raw = client.SEND(RESPParser::encode(req));
        RESPResponse resp = RESPParser::decode(raw);
        if (resp.isError) {
            throw DeserializeException(resp.value);
        }
        return resp.value == "1";
    }

    /**
     * @brief Clears all keys in the specified database.
     * @return bool - true on successful clear
     */
    bool CLEAR(const unsigned int DB) {
        RESPRequest req;
        req.cmd = "CLEAR";
        req.dbIndex = DB;
        std::string raw = client.SEND(RESPParser::encode(req));
        RESPResponse resp = RESPParser::decode(raw);
        if (resp.isError) {
            throw DeserializeException(resp.value);
        }
        return true;
    }

    /**
     * @brief Sets a time-to-live timeout (in seconds) on a key.
     * @return bool - true on success
     */
    bool EXPIRE(const unsigned int DB, const std::string& _key, const size_t duration) {
        RESPRequest req;
        req.cmd = "EXPIRE";
        req.dbIndex = DB;
        req.key = _key;
        req.value = std::to_string(duration);
        std::string raw = client.SEND(RESPParser::encode(req));
        RESPResponse resp = RESPParser::decode(raw);
        if (resp.isError) {
            throw DeserializeException(resp.value);
        }
        return true;
    }

    /**
     * @brief Atomically increments the integer value of a key.
     * @return long long - the value after the increment
     */
    long long INCR(const unsigned int DB, const std::string& _key) {
        RESPRequest req;
        req.cmd = "INCR";
        req.dbIndex = DB;
        req.key = _key;
        std::string raw = client.SEND(RESPParser::encode(req));
        RESPResponse resp = RESPParser::decode(raw);
        if (resp.isError) {
            throw DeserializeException(resp.value);
        }
        return std::stoll(resp.value);
    }

    /**
     * @brief Stores a raw, unserialized string value in CacheCore.
     * @return bool - true on success
     */
    bool SETRAW(const unsigned int DB, const std::string& _key, const std::string& _value, const bool _willExpire = true) {
        RESPRequest req;
        req.cmd      = "SET";
        req.dbIndex  = DB;
        req.key      = _key;
        req.value    = _value;
        req.expires  = _willExpire;

        std::string raw = client.SEND(RESPParser::encode(req));
        RESPResponse resp = RESPParser::decode(raw);
        if (resp.isError) {
            throw DeserializeException(resp.value);
        }
        return resp.value == "1";
    }

    /**
     * @brief Retrieves a raw, unserialized string value from CacheCore.
     * @return std::optional<std::string> - raw string value, or std::nullopt if key does not exist
     */
    std::optional<std::string> GETRAW(const unsigned int DB, const std::string& _key) {
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

        return resp.value;
    }

    /**
    * @brief Stores a value in CacheCore. Accepts any supported STL container,
    *        std::string, or arithmetic primitive. Serializes client-side before sending.
    *
    * @note Primitives: "N:type\x1FM:value"
    * @note Containers: "N:container\x1FM:type\x1Flen:v1\x1Flen:v2..."
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
    * @brief Retrieves a value from CacheCore and deserializes it into T.
    *        Caller specifies the expected type: cache.GET<std::vector<int>>(0, "key")
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