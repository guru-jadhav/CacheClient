#pragma once
#include "Exceptions.h"
#include <optional>
#include <string>
#include <sys/types.h>
#include <vector>

static const std::string CRLF = "\r\n";
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



class RESPParser{
    static std::string lenPrefix(std::string s){
        return "$" + std::to_string(s.size()) + CRLF + s + CRLF;
    }
    
    static bool handleSimpleString(const std::string& resp) {
        // +OK\r\n or +PONG\r\n — complete when we find \r\n
        return resp.find(CRLF) != std::string::npos;
    }

    static bool handleError(const std::string& resp) {
        // -ERR <msg>\r\n — complete when we find \r\n
        return resp.find(CRLF) != std::string::npos;
    }

    static bool handleInteger(const std::string& resp) {
        // :<value>\r\n — complete when we find \r\n
        return resp.find(CRLF) != std::string::npos;
    }

    static bool handleBulk(const std::string& resp) {
        // $-1\r\n — nullbulk
        // $<len>\r\n<data>\r\n — complete when we have header + len bytes + trailing \r\n

        size_t crlfPos = resp.find(CRLF);
        if (crlfPos == std::string::npos) return false;

        // extract length from "$<len>"
        int len = std::stoi(resp.substr(1, crlfPos - 1));
        
        // nullbulk
        if (len == -1) return true;

        // header + data + trailing \r\n
        size_t expectedSize = crlfPos + 2 + len + 2;
        return resp.size() >= expectedSize;
    }

public:
    static std::string encode(const RESPRequest& req) {
        std::vector<std::string> elements;

        elements.push_back(std::to_string(req.dbIndex));
        elements.push_back(req.cmd);

        if (req.key){
            elements.push_back(*req.key);
        }
        if (req.value){
            elements.push_back(*req.value);
        }
        if (req.expires){
            elements.push_back(*req.expires ? "1" : "0");
        } 

        std::string encoded = "*" + std::to_string(elements.size()) + CRLF;
        for (const auto& e : elements){
            encoded += lenPrefix(e);
        } 
        return encoded;
    }

    static RESPResponse decode(const std::string& rawResponse) {
        if (rawResponse.empty()) {
            throw DeserializeException("empty response from server");
        }

        RESPResponse result;
        size_t crlfPos = rawResponse.find(CRLF);

        if (crlfPos == std::string::npos) {
            throw DeserializeException("malformed RESP response — no CRLF found");
        }

        switch (rawResponse[0]) {
            case '+': {
                // +OK\r\n or +PONG\r\n
                result.value = rawResponse.substr(1, crlfPos - 1);
                break;
            }
            case '-': {
                // -ERR <msg>\r\n
                result.isError = true;
                result.value = rawResponse.substr(1, crlfPos - 1);
                break;
            }
            case ':': {
                // :<value>\r\n
                result.value = rawResponse.substr(1, crlfPos - 1);
                break;
            }
            case '$': {
                int len = std::stoi(rawResponse.substr(1, crlfPos - 1));
                if (len == -1) {
                    // $-1\r\n — key not found
                    result.isNull = true;
                } else {
                    // $<len>\r\n<data>\r\n
                    result.value = rawResponse.substr(crlfPos + 2, len);
                }
                break;
            }
            default:
                throw DeserializeException("unknown RESP type: " + std::string(1, rawResponse[0]));
        }

        return result;
    }

    static bool isComplete(const std::string& rawResponse) {
        if (rawResponse.empty()) {
            return false;
        }

        switch (rawResponse[0]) {
            case '+': 
                return handleSimpleString(rawResponse);
            case '-': 
                return handleError(rawResponse);
            case ':': 
                return handleInteger(rawResponse);
            case '$': 
                return handleBulk(rawResponse);
            default:  
                return false;
        }
    }
};
