#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include "../include/Exceptions.h"

class TCPClient {
    std::string domain;
    std::uint16_t port;
    int clientFd = -1;
    
    bool tryConnect(){
        struct addrinfo hints{};
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        struct addrinfo* res = nullptr;

        int status = getaddrinfo(domain.c_str(), std::to_string(port).c_str(), &hints, &res);
        if(status != 0){
            return false;
        }

        for (struct addrinfo *p = res; p != nullptr; p = p->ai_next) {
            clientFd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (clientFd < 0)
                continue;
            if (::connect(clientFd, p->ai_addr, p->ai_addrlen) == 0) {
                freeaddrinfo(res);
                return true;
            }
            close(clientFd);
            clientFd = -1;
        }

        return false;
    };

    std::optional<std::string> RECV(){
        
        char buffer[4096];
        std::string response;
        while(true){
            int bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
        
            if(bytesRead == -1){
                return std::nullopt;
            }
            
            buffer[bytesRead] = '\0';
            response += std::string(buffer, bytesRead);
        
            // but what if server gets disconnected ? and we need to parse resp cmd out of this 
            // only then we know that we can return from this response
            
        }
        return "";
    };

public:

    TCPClient();
    TCPClient(const std::string& _domain, const std::uint16_t _port) : domain(_domain), port(_port) {}
    
    bool connect(){
        return tryConnect();
    }


    std::optional<std::string> SEND(const std::string& raw){
        u_int32_t totalSent = 0;
        while(totalSent != raw.size()){
            int bytesSent = send(clientFd, raw.c_str() + totalSent, raw.size() - totalSent, 0);
            if(bytesSent == -1){
                // return std::nullopt;
                throw NetworkException("network erroe while sending to CacheCore");
            }
            totalSent += bytesSent;
        }
        
        return RECV();
    };
};