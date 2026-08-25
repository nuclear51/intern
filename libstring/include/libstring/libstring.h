#pragma once

#include <string>
#include <string_view>

namespace libstring {

void SortDescendingReplaceEven(std::string& digits);

int SumDigits(std::string_view text);

bool IsAcceptedSum(std::string_view sum_text);

}  // namespace libstring
