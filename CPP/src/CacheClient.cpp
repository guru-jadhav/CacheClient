#include "../include/CacheClient.h"

CacheClient::CacheClient(const std::string& _domain, const std::uint16_t _port)
    : domain(_domain), port(_port), client(_domain, _port) {}

bool CacheClient::connect() {
    return client.connect();
}

std::string CacheClient::PING() {
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

bool CacheClient::DEL(const unsigned int DB, const std::string& _key) {
    RESPRequest req;
    req.cmd = "DEL";
    req.dbIndex = DB;
    req.key = _key;
    std::string raw = client.SEND(RESPParser::encode(req));
    RESPResponse resp = RESPParser::decode(raw);
    if (resp.isError) {
        throw DeserializeException(resp.value);
    }
    return resp.value == "OK" || resp.value == "1";
}

bool CacheClient::EXISTS(const unsigned int DB, const std::string& _key) {
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

bool CacheClient::CLEAR(const unsigned int DB) {
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

bool CacheClient::EXPIRE(const unsigned int DB, const std::string& _key, const size_t duration) {
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

long long CacheClient::INCR(const unsigned int DB, const std::string& _key) {
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

bool CacheClient::SETRAW(const unsigned int DB, const std::string& _key, const std::string& _value, const bool _willExpire) {
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

std::optional<std::string> CacheClient::GETRAW(const unsigned int DB, const std::string& _key) {
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
