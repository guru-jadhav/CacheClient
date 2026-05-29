#include <cstdint>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
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

public:

    TCPClient();
    TCPClient(const std::string& _domain, const std::uint16_t _port) : domain(_domain), port(_port) {}
    
    bool connect(){
        return tryConnect();
    }
};