#include <libstring/libstring.h>

#include <gtest/gtest.h>

#include <string>

namespace libstring {
namespace {

TEST(SortDescendingReplaceEvenTest, WorksCorrectly) {
  std::string value = "1234";

  SortDescendingReplaceEven(value);

  EXPECT_EQ(value, "KB3KB1");
}

TEST(SortDescendingReplaceEvenTest, EmptyString) {
  std::string value = "";

  SortDescendingReplaceEven(value);

  EXPECT_EQ(value, "");
}

TEST(SumDigitsTest, SumsDigits) { EXPECT_EQ(SumDigits("1234"), 10); }

TEST(SumDigitsTest, IgnoresLetters) { EXPECT_EQ(SumDigits("KB3KB1"), 4); }

TEST(IsAcceptedSumTest, AcceptsCorrectValue) {
  EXPECT_TRUE(IsAcceptedSum("128"));
}

TEST(IsAcceptedSumTest, RejectsIncorrectValues) {
  EXPECT_FALSE(IsAcceptedSum("32"));
  EXPECT_FALSE(IsAcceptedSum("100"));
  EXPECT_FALSE(IsAcceptedSum("abc"));
}

}  // namespace
}  // namespace libstring