
#include "zen/range.hpp"

#include "gtest/gtest.h"

// NOTE This must be manually tested
// TEST(RangeTest, ErrorsWhenPassingInLValueContainer) {
//   std::vector v1 { 10, 11, 12 };
//   zen::zip(std::vector { 1, 2, 3 }, std::vector { 6, 7, 8 });
//   zen::zip(v1, std::vector { 6, 7, 8 });
//   zen::zip(std::vector { 6, 7, 8 }, v1);
// }

TEST(RangeTest, CanZipMixed) {
  std::vector v1 { 1, 2, 3 };
  std::vector v2 { 5, 6, 7 };
  int i = 1;
  int j = 5;
  for (auto [x1, x2]: zen::zip(v1, zen::make_iterator_range(v2))) {
    ASSERT_EQ(x1, i++);
    ASSERT_EQ(x2, j++);
  }
}

TEST(RangeTest, CanZipConst) {
  std::vector<int> v1 {};
  static_assert(std::is_same_v<decltype(*std::as_const(v1).begin()), const int&>);
  auto result = zen::make_iterator_range(std::as_const(v1));
  static_assert(std::is_same_v<decltype(*result.begin()), const int&>);
}

TEST(RangeTest, CanIterConst) {
  std::vector<int> v1 { 1, 2, 3 };
  int i = 1;
  for (auto x: zen::make_iterator_range(std::as_const(v1))) {
    ASSERT_EQ(x, i++);
  }
}
