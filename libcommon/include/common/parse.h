#pragma once

#include <cstdint>

#include <charconv>
#include <concepts>
#include <optional>
#include <string_view>
#include <system_error>

namespace common {

template <std::integral T>
  requires(!std::same_as<T, bool>)
inline std::optional<T> ParseInteger(std::string_view text) {
  T value{};

  const auto* const begin = text.data();
  const auto* const end = begin + text.size();

  const auto [parse_end, error] = std::from_chars(begin, end, value);

  if (error != std::errc{} || parse_end != end) {
    return std::nullopt;
  }

  return value;
}

inline std::optional<std::uint16_t> ParsePort(std::string_view text) {
  const auto port = ParseInteger<std::uint16_t>(text);

  // Порт не может быть 0
  if (!port || *port == 0) {
    return std::nullopt;
  }

  return port;
}

}  // namespace common