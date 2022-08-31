#pragma once

#include <functional>
#include <optional>

namespace coroutine {

// Represents a coroutine that yields values of type Y.
template<typename Y>
class Coroutine {
public:
    Coroutine(std::function<void()> bound_fn) : bound_fn_(bound_fn) {};
    void Call() {
        bound_fn_();
    };

private:
    std::function<void()> bound_fn_;
};

// Creates a coroutine.
template<typename Y, typename... Args>
Coroutine<Y> Create(std::function<void(Args...)> fn, Args... args) {
    return Coroutine<Y>(std::bind(fn, args...));
}

// Resumes execution of the coroutine and blocks until coroutine reaches a yield
// or terminates. Returns the value yielded by the coroutine. If coroutine
// terminated rather than yielded, the return value is empty.
template<typename Y>
std::optional<Y> Resume(Coroutine<Y> &coro) {
    coro.Call();
}

// Suspends execution of coroutine. yield_value becomes the return value of the
// corresponding Resume.
template<typename Y>
void Yield(Y yield_value);

}
