#include "../include/TypeSerializer.h"

std::string lenPrefix(const std::string &s) {
    return std::to_string(s.size()) + ':' + s;
}

std::vector<std::string> parseBlocks(const std::string &raw) {
    std::vector<std::string> blocks;
    size_t i = 0;

    while (i < raw.size()) {
        size_t colonPos = raw.find(':', i);
        if (colonPos == std::string::npos)
            break;

        size_t blockLen = std::stoull(raw.substr(i, colonPos - i));
        i = colonPos + 1;

        if (i + blockLen > raw.size())
            break;
        blocks.push_back(raw.substr(i, blockLen));
        i += blockLen;

        if (i < raw.size() && raw[i] == DELIM)
            i++;
    }

    return blocks;
}
