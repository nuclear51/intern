#include <common/parse.h>
#include <common/shared_buffer.h>
#include <libstring/libstring.h>

#include <cstdint>
#include <iostream>
#include <optional>
#include <print>
#include <string>
#include <string_view>
#include <thread>

#include "util.h"
#include "tcp_client.h"

namespace {

constexpr std::string_view kDefaultHost = "127.0.0.1";
constexpr std::uint16_t kDefaultPort = 5555;

void ProcessingLoop(common::SharedBuffer& buffer, program1::TcpClient& client) {
  while (true) {
    const auto data = buffer.Take();
    if (!data.has_value()) {
      break;
    }
    const auto sum_text = std::to_string(libstring::SumDigits(*data));
    std::println("Processed string: {}", *data);
    std::println("Sum of digits: {}", sum_text);
    if (client.Send(sum_text)) {
      std::println("Sum {} was sent to program 2.", sum_text);
    } else {
      std::println(
          stderr,
          "Warning: program 2 is unreachable, sum {} was not delivered.",
          sum_text);
    }
  }
}

void InputLoop(common::SharedBuffer& buffer) {
  std::string line;
  while (true) {
    std::println("Enter a string of up to 64 digits (Ctrl+D to exit):");
    if (!std::getline(std::cin, line)) {
      break;
    }
    if (!program1::IsValidInput(line)) {
      std::println(stderr,
                   "Error: the input must be a non-empty string of at most 64 "
                   "decimal digits.");
      continue;
    }
    libstring::SortDescendingReplaceEven(line);
    buffer.Put(line);
  }
}

}  // namespace

int main(int argc, char* argv[]) {
  std::string host(kDefaultHost);
  std::uint16_t port = kDefaultPort;
  if (argc > 1) {
    host = argv[1];
  }
  if (argc > 2) {
    const auto parsed = common::ParsePort(argv[2]);
    if (!parsed.has_value()) {
      std::println(stderr, "Usage: {} [host] [port]", argv[0]);
      return 1;
    }
    port = *parsed;
  }

  common::SharedBuffer buffer;
  program1::TcpClient client(host, port);
  std::thread processing_thread(ProcessingLoop, std::ref(buffer),
                                std::ref(client));

  InputLoop(buffer);

  buffer.Close();
  processing_thread.join();
  return 0;
}
