#include <common/parse.h>
#include <libstring/libstring.h>

#include <atomic>
#include <csignal>
#include <cstdint>
#include <exception>
#include <optional>
#include <print>
#include <string>

#include "tcp_server.h"

namespace {

constexpr std::uint16_t kDefaultPort = 5555;

std::atomic<bool> stop_requested{false};

void HandleSignal([[maybe_unused]] int signal_number) {
  stop_requested.store(true);
}

void ReportSum(const std::string& sum_text) {
  if (libstring::IsAcceptedSum(sum_text)) {
    std::println(
        "Accepted: sum {} is longer than 2 characters and divisible by 32!",
        sum_text);
  } else {
    std::println("Not accepted: sum '{}' does not satisfy the criteria.",
                 sum_text);
  }
}

}  // namespace

int main(int argc, char* argv[]) {
  std::uint16_t port = kDefaultPort;
  if (argc > 1) {
    const std::optional<std::uint16_t> parsed = common::ParsePort(argv[1]);
    if (!parsed.has_value()) {
      std::println(stderr, "Usage: {} [port]", argv[0]);
      return 1;
    }
    port = *parsed;
  }

  std::signal(SIGINT, HandleSignal);
  std::signal(SIGTERM, HandleSignal);

  try {
    program2::TcpServer server(port);
    std::println("Program 2 is listening on port {}. Waiting for program 1...",
                 server.port());
    server.Run(ReportSum, stop_requested);
  } catch (const std::exception& error) {
    std::println(stderr, "Fatal error: {}", error.what());
    return 1;
  }
  return 0;
}
