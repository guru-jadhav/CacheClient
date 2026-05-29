#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <type_traits>
#include "TypeSerializer.h"
#include "TCPClient.h"

enum class ClientError {
    NETWORK_ERR,
    UNSUPPORTED_TYPE,
    SERIALIZE_ERR,
    DESERIALIZE_ERR
};

class CacheClient {
    std::string domain;
    std::uint16_t port;
    TCPClient client;
    // TODO: add TCPClient instance here
    // TODO: add RESPParser instance here

public:

    CacheClient(const std::string& _domain, const std::uint16_t _port) : domain(_domain), port(_port) {
        client = TCPClient(_domain, _port);
    }
    
    /**
    * @brief method to check if the TCP connection between the CacheClient & CacheCore is working.
    * @note checks DNS resolution + TCP connection
    * @return bool - true/false
    */
    bool connect(){
        return client.connect();
    };

    std::string PING();
    bool DEL(const unsigned int DB, const std::string& _key);
    bool EXISTS(const unsigned int DB, const std::string& _key);
    bool CLEAR(const unsigned int DB);
    bool EXPIRE(const unsigned int DB, const std::string& _key, const size_t duration);
    long long INCR(const unsigned int DB, const std::string& _key);

    /**
    * @brief Stores a value in CacheCore. Accepts any supported STL container,
    *        std::string, or arithmetic primitive. Serializes client-side before sending.
    *
    * @note Primitives: "N:type\x1FM:value"
    * @note Containers: "N:container\x1FM:type\x1Flen:v1\x1Flen:v2..."
    * @note Returns std::nullopt on unsupported type or network error.
    *
    * TODO: RESPParser::encode() -> TCPClient::send() -> return ACK
    */
    template<typename T>
    std::optional<bool> SET(const unsigned int DB, const std::string& _key, const T& _value, const bool _willExpire = true) {

        if constexpr (std::is_arithmetic_v<T>) {
            if (TypeName<T>::name() == "unknown") return std::nullopt;
            std::string formatted = Serializer::serializePrimitive(_value);
            // TODO: RESPParser::encode(DB, "SET", _key, formatted, _willExpire)
            // TODO: TCPClient::send(encoded)

        } else {
            if (ContainerName<T>::name() == "unknown") return std::nullopt;
            if (TypeName<typename T::value_type>::name() == "unknown") return std::nullopt;
            std::string formatted = Serializer::serializeContainer(_value);
            // TODO: RESPParser::encode(DB, "SET", _key, formatted, _willExpire)
            // TODO: TCPClient::send(encoded)
        }

        return std::nullopt;
    }

    /**
    * @brief Retrieves a value from CacheCore and deserializes it into T.
    *        Caller specifies the expected type: cache.GET<std::vector<int>>(0, "key")
    *
    * TODO: TCPClient::send(encoded) -> RESPParser::decode() -> deserialize -> return T
    */
    template<typename T>
    std::optional<T> GET(const unsigned int DB, const std::string& _key);
};