#include "../include/RESPParser.h"
#include "../include/Exceptions.h"
#include <vector>

static const std::string CRLF = "\r\n";

std::string RESPParser::lenPrefix(std::string s) {
    return "$" + std::to_string(s.size()) + CRLF + s + CRLF;
}

bool RESPParser::handleSimpleString(const std::string& resp) {
    return resp.find(CRLF) != std::string::npos;
}

bool RESPParser::handleError(const std::string& resp) {
    return resp.find(CRLF) != std::string::npos;
}

bool RESPParser::handleInteger(const std::string& resp) {
    return resp.find(CRLF) != std::string::npos;
}

bool RESPParser::handleBulk(const std::string& resp) {
    size_t crlfPos = resp.find(CRLF);
    if (crlfPos == std::string::npos) return false;

    int len = std::stoi(resp.substr(1, crlfPos - 1));
    if (len == -1) return true;

    size_t expectedSize = crlfPos + 2 + len + 2;
    return resp.size() >= expectedSize;
}

std::string RESPParser::encode(const RESPRequest& req) {
    std::vector<std::string> elements;

    elements.push_back(std::to_string(req.dbIndex));
    elements.push_back(req.cmd);

    if (req.key) {
        elements.push_back(*req.key);
    }
    if (req.value) {
        elements.push_back(*req.value);
    }
    if (req.expires) {
        elements.push_back(*req.expires ? "1" : "0");
    }

    std::string encoded = "*" + std::to_string(elements.size()) + CRLF;
    for (const auto& e : elements) {
        encoded += lenPrefix(e);
    }
    return encoded;
}

RESPResponse RESPParser::decode(const std::string& rawResponse) {
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
            result.value = rawResponse.substr(1, crlfPos - 1);
            break;
        }
        case '-': {
            result.isError = true;
            result.value = rawResponse.substr(1, crlfPos - 1);
            break;
        }
        case ':': {
            result.value = rawResponse.substr(1, crlfPos - 1);
            break;
        }
        case '$': {
            int len = std::stoi(rawResponse.substr(1, crlfPos - 1));
            if (len == -1) {
                result.isNull = true;
            } else {
                result.value = rawResponse.substr(crlfPos + 2, len);
            }
            break;
        }
        default:
            throw DeserializeException("unknown RESP type: " + std::string(1, rawResponse[0]));
    }

    return result;
}

bool RESPParser::isComplete(const std::string& rawResponse) {
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
