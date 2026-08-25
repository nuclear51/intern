#include <libstring/libstring.h>

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstdint>
#include <functional>
#include <system_error>
#include <utility>

namespace libstring {

namespace {

constexpr std::string_view kEvenDigitReplacement = "KB";
constexpr std::size_t kMinAcceptedLength = 3;
constexpr std::uint64_t kAcceptedDivisor = 32;

bool IsDigit(char symbol) {
  return std::isdigit(static_cast<unsigned char>(symbol)) != 0;
}

}  // namespace

// Функция 1: сортируем строку по убыванию и заменяем
// каждую чётную цифру на "KB"
void SortDescendingReplaceEven(std::string& digits) {
  std::sort(digits.begin(), digits.end(), std::greater<char>());
  std::string result;
  result.reserve(digits.size() * kEvenDigitReplacement.size());
  for (char symbol : digits) {
    if (IsDigit(symbol) && (symbol - '0') % 2 == 0) {
      result += kEvenDigitReplacement;
    } else {
      result += symbol;
    }
  }
  digits = std::move(result);
}

// Функция 2: считаем сумму всех цифр, оставшихся в строке
int SumDigits(std::string_view text) {
  int sum = 0;
  for (char symbol : text) {
    if (IsDigit(symbol)) {
      sum += symbol - '0';
    }
  }
  return sum;
}

// Функция 3: сумма должна состоять минимум из трёх символов
// и быть кратна 32
bool IsAcceptedSum(std::string_view sum_text) {
  if (sum_text.size() < kMinAcceptedLength) {
    return false;
  }
  std::uint64_t value = 0;
  const char* const begin = sum_text.data();
  const char* const end = begin + sum_text.size();
  const auto [parse_end, error] = std::from_chars(begin, end, value);
  if (error != std::errc() || parse_end != end) {
    return false;
  }
  return value % kAcceptedDivisor == 0;
}

}  // namespace libstring
