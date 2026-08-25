#pragma once

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <ranges>
#include <string_view>

namespace program1 {

inline constexpr std::size_t kMaxInputLength = 64;

inline bool IsValidInput(std::string_view line) {
  if (line.empty() || line.size() > kMaxInputLength) {
    return false;
  }
  return std::ranges::all_of(line, [](char symbol) {
    return std::isdigit(static_cast<unsigned char>(symbol)) != 0;
  });
}

}  // namespace program1
