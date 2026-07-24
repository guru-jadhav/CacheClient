#pragma once
#include <optional>
#include <string>

struct RESPRequest {
    std::string cmd;
    int dbIndex = -1;
    std::optional<std::string> key = std::nullopt;
    std::optional<std::string> value = std::nullopt;
    std::optional<bool> expires = std::nullopt;
};

struct RESPResponse {
    bool isError = false;
    bool isNull = false;
    std::string value;
};

class RESPParser {
    static std::string lenPrefix(std::string s);
    static bool handleSimpleString(const std::string& resp);
    static bool handleError(const std::string& resp);
    static bool handleInteger(const std::string& resp);
    static bool handleBulk(const std::string& resp);

public:
    static std::string encode(const RESPRequest& req);
    static RESPResponse decode(const std::string& rawResponse);
    static bool isComplete(const std::string& rawResponse);
};
