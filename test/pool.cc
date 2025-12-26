
#include "zen/pool.hpp"

#include "gtest/gtest.h"

TEST(PoolTest, TestConstructInt) {
  zen::pool<int> p;
  auto one = p.construct<int>(1);
  auto two = p.construct<int>(2);
  auto three = p.construct<int>(3);
  ASSERT_EQ(*one, 1);
  ASSERT_EQ(*two, 2);
  ASSERT_EQ(*three, 3);
}

TEST(PoolTest, TestConstructNextChunk) {
  zen::pool<int, 2> p;
  auto one = p.construct<int>(1);
  auto two = p.construct<int>(2);
  auto three = p.construct<int>(3);
  auto four = p.construct<int>(4);
  auto five = p.construct<int>(5);
  ASSERT_EQ(*one, 1);
  ASSERT_EQ(*two, 2);
  ASSERT_EQ(*three, 3);
  ASSERT_EQ(*four, 4);
  ASSERT_EQ(*five, 5);
}
