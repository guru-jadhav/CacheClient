#include "../include/TCPClient.h"
#include "../include/Exceptions.h"
#include "../include/RESPParser.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>

TCPClient::TCPClient(const std::string &_domain, const std::uint16_t _port)
    : domain(_domain), port(_port) {}

bool TCPClient::connect() {
    return tryConnect();
}

bool TCPClient::tryConnect() {
    struct addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *res = nullptr;

    int status = getaddrinfo(domain.c_str(), std::to_string(port).c_str(), &hints, &res);
    if (status != 0) {
        throw NetworkException("DNS resolution failed for: " + domain);
    }

    for (struct addrinfo *p = res; p != nullptr; p = p->ai_next) {
        clientFd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (clientFd < 0)
            continue;
        if (::connect(clientFd, p->ai_addr, p->ai_addrlen) == 0) {
            freeaddrinfo(res);
            return true;
        }
        ::close(clientFd);
        clientFd = -1;
    }

    freeaddrinfo(res);
    throw NetworkException("failed to connect to: " + domain + ":" + std::to_string(port));
}

bool TCPClient::tryReconnect() {
    try {
        close();
        return tryConnect();
    } catch (...) {
        return false;
    }
}

std::string TCPClient::RECV() {
    char buffer[4096];
    std::string response;

    while (true) {
        int bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);

        if (bytesRead == 0) {
            close();
            throw NetworkException("server closed the connection");
        }
        if (bytesRead == -1) {
            close();
            throw NetworkException("network error while receiving from CacheCore");
        }

        buffer[bytesRead] = '\0';
        response += std::string(buffer, bytesRead);

        if (RESPParser::isComplete(response)) {
            return response;
        }
    }
}

std::string TCPClient::sendInternal(const std::string &raw) {
    if (clientFd < 0) {
        throw NetworkException("client is not connected to CacheCore");
    }

    size_t totalSent = 0;
    while (totalSent < raw.size()) {
        ssize_t bytesSent = send(clientFd, raw.c_str() + totalSent, raw.size() - totalSent, 0);
        if (bytesSent == -1) {
            close();
            throw NetworkException("network error while sending to CacheCore");
        }
        totalSent += bytesSent;
    }
    return RECV();
}

std::string TCPClient::SEND(const std::string &raw) {
    try {
        return sendInternal(raw);
    } catch (const NetworkException &e) {
        // Attempt to auto-reconnect once and retry
        if (tryReconnect()) {
            return sendInternal(raw);
        }
        throw;
    }
}

TCPClient::~TCPClient() {
    close();
}

void TCPClient::close() {
    if (clientFd != -1) {
        ::close(clientFd);
        clientFd = -1;
    }
}
