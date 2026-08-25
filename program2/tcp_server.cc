#include "tcp_server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstddef>
#include <cstring>
#include <format>
#include <print>
#include <stdexcept>
#include <utility>

#include <common/unique_fd.h>

namespace program2 {

namespace {

constexpr int kPollTimeoutMs = 500;
constexpr std::size_t kReceiveChunkBytes = 4096;
constexpr std::size_t kMaxPendingBytes = 64 * 1024;
constexpr std::size_t kMaxClients = 64;

constexpr short kErrorEvents = POLLERR | POLLHUP | POLLNVAL;

std::string FormatPeer(const sockaddr_in& address) {
  char text[INET_ADDRSTRLEN]{};
  inet_ntop(AF_INET, &address.sin_addr, text, sizeof(text));
  return std::format("{}:{}", text, ntohs(address.sin_port));
}

}  // namespace

// Исключение здесь - чтобы не создавался битый обьект
TcpServer::TcpServer(std::uint16_t port) {
  listen_fd_.Reset(socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0));
  if (!listen_fd_.IsValid()) {
    throw std::runtime_error(
        std::format("Failed to create a socket: {}", std::strerror(errno)));
  }

  const int enable = 1;
  setsockopt(listen_fd_.fd(), SOL_SOCKET, SO_REUSEADDR, &enable,
             sizeof(enable));

  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  address.sin_port = htons(port);

  if (bind(listen_fd_.fd(), reinterpret_cast<const sockaddr*>(&address),
           sizeof(address)) != 0 ||
      listen(listen_fd_.fd(), SOMAXCONN) != 0) {
    throw std::runtime_error(std::format("Failed to listen on port {}: {}",
                                         port, std::strerror(errno)));
  }

  sockaddr_in bound{};
  auto length = static_cast<socklen_t>(sizeof(bound));
  if (getsockname(listen_fd_.fd(), reinterpret_cast<sockaddr*>(&bound),
                  &length) != 0) {
    throw std::runtime_error(std::format("Failed to query the bound port: {}",
                                         std::strerror(errno)));
  }
  port_ = ntohs(bound.sin_port);
}

void TcpServer::Run(const MessageHandler& handler,
                    const std::atomic<bool>& stop) {
  // Работаем пока внешний stop не запрошен
  std::vector<pollfd> poll_set;
  while (!stop.load()) {
    // Собираем подключения и poll'им
    poll_set.clear();
    poll_set.push_back(pollfd{listen_fd_.fd(), POLLIN, 0});
    for (const auto& connection : connections_) {
      poll_set.push_back(pollfd{connection.fd.fd(), POLLIN, 0});
    }

    const int ready = poll(poll_set.data(), poll_set.size(), kPollTimeoutMs);
    if (ready < 0) {
      if (errno == EINTR) {
        continue;
      }
      std::println(stderr, "poll failed: {}", std::strerror(errno));
      break;
    }
    if (ready == 0) {
      continue;
    }

    const auto listening_socket = poll_set[0];

    if ((listening_socket.revents & POLLIN) != 0) {
      AcceptClient();
    } else if ((listening_socket.revents & kErrorEvents) != 0) {
      std::println(stderr, "The listening socket failed; shutting down.");
      break;
    }

    // Обслуживаем клиентов
    const auto polled_count = poll_set.size() - 1;
    for (auto index = polled_count; index-- > 0;) {
      const short revents = poll_set[index + 1].revents;
      Connection& connection = connections_[index];
      bool keep = true;
      if ((revents & POLLIN) != 0) {
        keep = ReadFromClient(connection, handler);
      } else if ((revents & kErrorEvents) != 0) {
        keep = false;
      }
      if (!keep) {
        std::println("Client {} disconnected.", connection.peer);
        shutdown(connection.fd.fd(), SHUT_RDWR);
        connections_.erase(connections_.begin() +
                           static_cast<std::ptrdiff_t>(index));
      }
    }
  }
  DisconnectAll();
}

void TcpServer::AcceptClient() {
  sockaddr_in address{};
  socklen_t length = sizeof(address);
  common::FdWrapped fd(accept4(listen_fd_.fd(),
                               reinterpret_cast<sockaddr*>(&address), &length,
                               SOCK_CLOEXEC));
  if (!fd.IsValid()) {
    return;
  }
  std::string peer = FormatPeer(address);
  if (connections_.size() >= kMaxClients) {
    std::println(stderr, "Refusing client {}: the connection limit is reached.",
                 peer);
    return;
  }
  std::println("Client {} connected.", peer);
  connections_.push_back(Connection{std::move(fd), std::move(peer), {}});
}

bool TcpServer::ReadFromClient(Connection& connection,
                               const MessageHandler& handler) {
  char chunk[kReceiveChunkBytes];
  const auto received = recv(connection.fd.fd(), chunk, sizeof(chunk), 0);
  if (received == 0) {
    return false;
  }
  if (received < 0) {
    return errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK;
  }
  connection.pending.append(chunk, static_cast<std::size_t>(received));
  std::size_t line_end = 0;
  while ((line_end = connection.pending.find('\n')) != std::string::npos) {
    handler(connection.pending.substr(0, line_end));
    connection.pending.erase(0, line_end + 1);
  }
  if (connection.pending.size() > kMaxPendingBytes) {
    std::println(stderr, "Dropping client {}: it sent an oversized message.",
                 connection.peer);
    return false;
  }
  return true;
}

void TcpServer::DisconnectAll() {
  for (const Connection& connection : connections_) {
    shutdown(connection.fd.fd(), SHUT_RDWR);
  }
  connections_.clear();
}

}  // namespace program2
