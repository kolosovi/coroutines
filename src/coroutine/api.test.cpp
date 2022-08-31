#include <gtest/gtest.h>

#include <functional>

#include <coroutine/api.hpp>

TEST(CreateAndCall, 1) {
    struct SideEffect {
        int value;
    };
    SideEffect side_effect{0};
    auto coro = coroutine::Create<void, int, int>(
        std::function<void(int, int)>([&side_effect](int lhs, int rhs) -> void {
            side_effect.value = lhs - rhs;
        }),
        5,
        3
    );
    coro.Call();
    EXPECT_EQ(side_effect.value, 2);
}
