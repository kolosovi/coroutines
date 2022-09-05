#include <gtest/gtest.h>

#include <functional>

#include <coroutine/api.hpp>

TEST(CreateAndResume, 1) {
    std::optional<int> yield_value{};
    auto coro = coroutine::Create<int, int, int>(
        std::function<void(int, int)>([](int lhs, int rhs) -> void {
            coroutine::Yield(lhs - rhs);
            ;
        }),
        5,
        3
    );
    EXPECT_EQ(coro.status, coroutine::Status::kSuspended);

    yield_value = coroutine::Resume(coro);
    EXPECT_EQ(coro.status, coroutine::Status::kSuspended);
    EXPECT_TRUE(bool(yield_value));
    EXPECT_EQ(*yield_value, 2);

    yield_value = coroutine::Resume(coro);
    EXPECT_EQ(coro.status, coroutine::Status::kDead);
    EXPECT_FALSE(bool(yield_value));
}
