#pragma once
#include <exception>
#include <string>


class CacheException : public std::exception{
    std::string message = "oops something went wrong";
public:
    const char * what() const noexcept override {
        return message.c_str();
    };

    CacheException(const std::string& _message) : message(_message) {};
};

class NetworkException : public CacheException {
public:
    NetworkException(const std::string& _message): CacheException(_message) {};
};

class SerializeException : public CacheException {
public:
    SerializeException(const std::string& _message) : CacheException(_message) {};
};

class DeserializeException : public CacheException {
public:
    DeserializeException(const std::string& _message) : CacheException(_message) {};
};