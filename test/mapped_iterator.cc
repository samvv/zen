
#include <iterator>
#include <unordered_map>

#include "zen/mapped_iterator.hpp"
#include "zen/iterator_range.hpp"

#include "gtest/gtest.h"

// This provides prettier error messages than just a static_assert
template<std::input_iterator T>
struct EnsureInputIterator {};

EnsureInputIterator<zen::mapped_iterator<std::vector<int>::iterator, bool(*)(int)>> test;

TEST(MappedIteratorTest, CanMapOverPair) {
  std::unordered_map<int, int> m { { 1, 2 }, { 3, 4 } };
  auto range = zen::make_iterator_range(m.begin(), m.end()).map_second();
  auto iter = zen::map(m.begin(), [](auto p) { return p.second; });
  ASSERT_TRUE(*iter++ < 5);
  ASSERT_TRUE(*iter++ < 5);
}
