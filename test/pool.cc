
#include "zen/bump_ptr_pool.hpp"

#include "gtest/gtest.h"

TEST(BumpPtrPoolTest, TestConstructInt) {
  zen::bump_ptr_pool p;
  auto one = construct<int>(p, 1);
  auto two = construct<int>(p, 2);
  auto three = construct<int>(p, 3);
  ASSERT_NE(one, nullptr);
  ASSERT_NE(two, nullptr);
  ASSERT_NE(three, nullptr);
  ASSERT_EQ(*one, 1);
  ASSERT_EQ(*two, 2);
  ASSERT_EQ(*three, 3);
}

TEST(BumpPtrPoolTest, TestRunsDestructor) {

  int count = 0;

  struct Foo {
    int value;
    int& count;

    ~Foo() {
      count++;
    }
  };

  {
    zen::bump_ptr_pool p;
    auto f1 = zen::construct<Foo>(p, 1, count);
    auto f2 = zen::construct<Foo>(p, 2, count);
    auto f3 = zen::construct<Foo>(p, 3, count);

    ASSERT_EQ(f1->value, 1);
    ASSERT_EQ(f2->value, 2);
    ASSERT_EQ(f3->value, 3);
    ASSERT_EQ(count, 0);
  }

  ASSERT_EQ(count, 3);
}

TEST(GrowingBumpPtrPoolTest, TestConstructNextChunk) {
  zen::growing_bump_ptr_pool p { 2 };
  auto one = zen::construct<int>(p, 1);
  auto two = construct<int>(p, 2);
  auto three = construct<int>(p, 3);
  auto four = construct<int>(p, 4);
  auto five = construct<int>(p, 5);
  ASSERT_NE(one, nullptr);
  ASSERT_NE(two, nullptr);
  ASSERT_NE(three, nullptr);
  ASSERT_NE(four, nullptr);
  ASSERT_NE(five, nullptr);
  ASSERT_EQ(*one, 1);
  ASSERT_EQ(*two, 2);
  ASSERT_EQ(*three, 3);
  ASSERT_EQ(*four, 4);
  ASSERT_EQ(*five, 5);
}
