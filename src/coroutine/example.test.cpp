#include <gtest/gtest.h>

#include <coroutine/example.hpp>

TEST(Call, 1) {
  bool result = false;
  result = Func1(12, [](int D) -> bool { return D < 0; });
  EXPECT_FALSE(result);
  result = Func1(12, [](int D) -> bool { return D > 0; });
  EXPECT_TRUE(result);
}
