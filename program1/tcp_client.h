#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include <common/unique_fd.h>

namespace program1 {

class TcpClient {
 public:
  TcpClient(std::string host, std::uint16_t port);
  ~TcpClient();

  TcpClient(const TcpClient&) = delete;
  TcpClient& operator=(const TcpClient&) = delete;

  bool Send(std::string_view message);

 private:
  bool Connect();
  void Disconnect();
  bool IsPeerAlive();
  bool SendAll(std::string_view data);

  std::string host_;
  std::uint16_t port_;
  common::FdWrapped fd_;
};

}  // namespace program1
