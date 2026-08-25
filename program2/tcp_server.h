#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include <common/unique_fd.h>

namespace program2 {

class TcpServer {
 public:
  using MessageHandler = std::function<void(const std::string& message)>;

  explicit TcpServer(std::uint16_t port);

  TcpServer(const TcpServer&) = delete;
  TcpServer& operator=(const TcpServer&) = delete;

  std::uint16_t port() const { return port_; }

  void Run(const MessageHandler& handler, const std::atomic<bool>& stop);

 private:
  struct Connection {
    common::FdWrapped fd;
    std::string peer;
    std::string pending;
  };

  void AcceptClient();
  bool ReadFromClient(Connection& connection, const MessageHandler& handler);
  void DisconnectAll();

  common::FdWrapped listen_fd_;
  std::uint16_t port_ = 0;
  std::vector<Connection> connections_;
};

}  // namespace program2
