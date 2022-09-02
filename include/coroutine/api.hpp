#pragma once

#include <functional>
#include <optional>
#include <memory>
#include <new>
#include <stdexcept>
#include <sstream>
#include <cstdarg>

namespace {
    static const int kStackSizeBytes = 32 * 1024;
    // ARM 64-bit requires stack to be 16 byte aligned. See
    // https://github.com/ARM-software/abi-aa/blob/main/aapcs64/aapcs64.rst#6221universal-stack-constraints
    static const std::align_val_t kStackAlignmentBytes = 16;

    template<typename Y>
    class Coroutine<Y>;

    template<typename Y>
    static Coroutine<Y> *current_coroutine = nullptr;

    static char *current_coroutine_stack_top = nullptr;

    extern "C" void switch_stack(char **old_stack_top, char **new_stack_top);
}

namespace coroutine {

enum class Status {
    kInvalid = 0,
    kSuspended,  // Was suspended or hasn't ran yet
    kRunning,  // Was resumed and is running
    kNormal,  // Resumed someone else
    kDead,  // Is dead (i.e. reached termination)
    kStatusCount  // always keep it last
};

class StatusViolationError : public std::logic_error {
public:
    explicit StatusViolationError(const string& what_arg)
        : logic_error(what_arg)
    {}
    explicit StatusViolationError(const char* what_arg)
        : logic_error(what_arg)
    {}

    static StatusViolationError New(Status actual, std::initializer_list<Status> expected) {
        std::stringstream msg;
        msg << "got coroutine status " << actual ", expected one of";
        bool is_first = true;
        for (auto status : expected) {
            if (!is_first) {
                msg << ",";
            }
            is_first = false;
            msg << " " << status;
        }
        return StatusViolationError(msg.str());
    }
};

class AttemptToYieldFromOutsideACoroutineError : public std::logic_error {
public:
    AttemptToYieldFromOutsideACoroutineError()
        : logic_error("attempt to yield from outside a coroutine")
    {}
};

// Represents a coroutine that yields values of type Y.
template<typename Y>
class Coroutine {
public:
    Coroutine(std::function<void()> bound_fn)
        : bound_fn_(bound_fn)
        , stack_(new (kStackAlignmentBytes) char[kStackSizeBytes])
        , stack_top_(stack_ + kStackSizeBytes)
        , status(Status::kSuspended)
    {
        // We should now initialize our memory.
    }

    void Resume(char **caller_stack_top_ptr) {
        if (status != Status::kSuspended) {
            throw StatusViolationError::New(status, {Status::kSuspended});
        }
        caller_stack_top_ptr_ = caller_stack_top_ptr;
        status = kRunning;
        current_coroutine<Y> = this;
        switch_stack(caller_stack_top_ptr_, &stack_top_);
    }

    void Yield(std::optional<Y> yield_value) {
        yield(Status::Status::kSuspended, yield_value);
    }

    std::optional<Y> yield_value;
    Status status;

private:
    std::function<void()> bound_fn_;
    std::unique_ptr<char[]> stack_;
    // Stack is Full and Descending on my machine, so stack_top_ points past the
    // first available (empty) byte.
    char *stack_top_;
    char **caller_stack_top_ptr_;

    void call() {
        bound_fn_();
        yield(Status::kDead, std::optional<Y>{});
    }

    void yield(Status new_status, std::optional<Y> yield_value) {
        if (status != kRunning) {
            throw StatusViolationError::New(status, {kRunning});
        }
        status = new_status;
        this->yield_value = yield_value;
        switch_stack(&stack_top_, caller_stack_top_ptr_);
    }
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
    coro.Resume(&current_coroutine_stack_top);
    return coro.yield_value;
}

// Suspends execution of coroutine. yield_value becomes the return value of the
// corresponding Resume.
template<typename Y>
void Yield(Y yield_value) {
    if (current_coroutine<Y> == nullptr) {
        throw AttemptToYieldFromOutsideACoroutineError();
    }
    current_coroutine<Y>->Yield();
}

}
