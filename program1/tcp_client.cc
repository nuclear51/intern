#include "tcp_client.h"

#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstddef>
#include <memory>
#include <utility>
#include <string_view>

#include <common/unique_fd.h>

namespace program1 {

namespace {

constexpr int kConnectTimeoutMs = 2000;
constexpr std::size_t kSendAttempts = 2;

using AddrInfoWrapped = std::unique_ptr<addrinfo, decltype(&freeaddrinfo)>;

AddrInfoWrapped ResolveHost(const std::string& host, std::uint16_t port) {
  addrinfo hints{};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  addrinfo* addresses = nullptr;
  const auto service = std::to_string(port);
  if (getaddrinfo(host.c_str(), service.c_str(), &hints, &addresses) != 0) {
    return AddrInfoWrapped(nullptr, &freeaddrinfo);
  }
  return AddrInfoWrapped(addresses, &freeaddrinfo);
}

bool WaitForConnect(int fd) {
  pollfd descriptor{};
  descriptor.fd = fd;
  descriptor.events = POLLOUT;
  if (poll(&descriptor, 1, kConnectTimeoutMs) != 1) {
    return false;
  }
  int error = 0;
  auto length = static_cast<socklen_t>(sizeof(error));
  if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &length) != 0) {
    return false;
  }
  return error == 0;
}

bool RestoreBlockingMode(int fd) {
  const auto flags = fcntl(fd, F_GETFL, 0);
  return flags >= 0 && fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) == 0;
}

}  // namespace

TcpClient::TcpClient(std::string host, std::uint16_t port)
    : host_(std::move(host)), port_(port) {}

TcpClient::~TcpClient() { Disconnect(); }

bool TcpClient::Send(std::string_view message) {
  std::string frame(message);
  // Разделяем сообщения через newline
  frame += '\n';

  for (std::size_t attempt = 0; attempt < kSendAttempts; ++attempt) {
    if (!fd_.IsValid() || !IsPeerAlive()) {
      Disconnect();
      if (!Connect()) {
        continue;
      }
    }

    if (SendAll(frame)) {
      return true;
    }

    Disconnect();
  }
  return false;
}

bool TcpClient::Connect() {
  const AddrInfoWrapped addresses = ResolveHost(host_, port_);
  for (const auto* entry = addresses.get(); entry != nullptr;
       entry = entry->ai_next) {
    common::FdWrapped candidate(socket(
        entry->ai_family, entry->ai_socktype | SOCK_NONBLOCK | SOCK_CLOEXEC,
        entry->ai_protocol));
    if (!candidate.IsValid()) {
      continue;
    }
    const int result =
        connect(candidate.fd(), entry->ai_addr, entry->ai_addrlen);
    const bool connected = (result == 0 || (errno == EINPROGRESS &&
                                            WaitForConnect(candidate.fd()))) &&
                           RestoreBlockingMode(candidate.fd());
    if (connected) {
      fd_ = std::move(candidate);
      return true;
    }
  }
  return false;
}

void TcpClient::Disconnect() {
  if (fd_.IsValid()) {
    shutdown(fd_.fd(), SHUT_RDWR);
    fd_.Reset();
  }
}

bool TcpClient::IsPeerAlive() {
  pollfd descriptor{};
  descriptor.fd = fd_.fd();
  descriptor.events = POLLIN;
  if (poll(&descriptor, 1, 0) <= 0) {
    return true;
  }
  if ((descriptor.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
    return false;
  }
  // Отправляем байт, проверяем соединение
  char probe = 0;
  const auto received = recv(fd_.fd(), &probe, 1, MSG_PEEK | MSG_DONTWAIT);
  if (received == 0) {
    return false;
  }
  if (received < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
    return false;
  }
  return true;
}

bool TcpClient::SendAll(std::string_view data) {
  std::size_t total_sent = 0;
  while (total_sent < data.size()) {
    const auto sent = send(fd_.fd(), data.data() + total_sent,
                           data.size() - total_sent, MSG_NOSIGNAL);
    if (sent < 0) {
      if (errno == EINTR) {
        continue;
      }
      return false;
    }
    total_sent += static_cast<std::size_t>(sent);
  }
  return true;
}

}  // namespace program1
