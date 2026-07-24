#pragma once
#include <string>
#include <cstdint>

/**
 * @brief Simple TCP socket client wrapper supporting blocking send and frame-aware receive.
 */
class TCPClient {
  std::string domain;
  std::uint16_t port;
  int clientFd = -1;

  bool tryConnect();
  std::string RECV();

public:
  TCPClient() = default;
  TCPClient(const std::string &_domain, const std::uint16_t _port);

  /**
   * @brief Resolves target address and connects to CacheCore server.
   * @return bool - true if connection was established
   */
  bool connect();

  /**
   * @brief Sends raw bytes over TCP and blocks until the full RESP frame response is received.
   * @return std::string - Raw RESP response string
   */
  std::string SEND(const std::string &raw);
};