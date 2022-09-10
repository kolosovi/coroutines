#pragma once

#include <functional>
#include <optional>
#include <memory>
#include <new>
#include <stdexcept>
#include <sstream>
#include <cstdarg>
#include <cstdint>
#include <string>

namespace coroutine {
    template<typename Y>
    class Coroutine;
}

namespace {
    using coroutine_stack_t = uint64_t;

    static coroutine_stack_t *current_coroutine_stack_top = nullptr;

    extern "C" void switch_stack(coroutine_stack_t **old_stack_top, coroutine_stack_t **new_stack_top);

    template<typename Y>
    static coroutine::Coroutine<Y> *current_coroutine = nullptr;

    template<typename Y>
    static void CallCurrentCoroutine() {
        current_coroutine<Y>->Call();
    }
}

namespace coroutine {

enum class Status {
    kInvalid = 0,
    kSuspended,  // Was suspended or hasn't ran yet
    kRunning,  // Was resumed and is running
    kNormal,  // Resumed someone else
    kDead  // Is dead (i.e. reached termination)
};

std::string ToString(Status status) {
    switch (status) {
        case Status::kInvalid:
            return "invalid";
        case Status::kSuspended:
            return "suspended";
        case Status::kRunning:
            return "running";
        case Status::kNormal:
            return "normal";
        case Status::kDead:
            return "dead";
    }
    return "";
}

class StatusViolationError : public std::logic_error {
public:
    explicit StatusViolationError(const std::string& what_arg)
        : logic_error(what_arg)
    {}
    explicit StatusViolationError(const char* what_arg)
        : logic_error(what_arg)
    {}

    static StatusViolationError New(Status actual, std::initializer_list<Status> expected) {
        std::stringstream msg;
        msg << "got coroutine status " << ToString(actual) << ", expected one of";
        bool is_first = true;
        for (auto status : expected) {
            if (!is_first) {
                msg << ",";
            }
            is_first = false;
            msg << " " << ToString(status);
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
        : status(Status::kSuspended)
        , bound_fn_(bound_fn)
        , stack_(new (kStackAlignmentBytes) coroutine_stack_t[kStackLength])
        , stack_top_(stack_.get() + kStackLength)
        , caller_stack_top_ptr_(nullptr)
    {
        for (int i = 0; i < kInitialStackOffset; i++) {
            *(--stack_top_) = 0;
        }
        stack_top_[kLinkRegisterOffset] = reinterpret_cast<coroutine_stack_t>(CallCurrentCoroutine<Y>);
    }

    void Resume(coroutine_stack_t **caller_stack_top_ptr) {
        if (status != Status::kSuspended) {
            throw StatusViolationError::New(status, {Status::kSuspended});
        }
        caller_stack_top_ptr_ = caller_stack_top_ptr;
        status = Status::kRunning;
        current_coroutine<Y> = this;
        switch_stack(caller_stack_top_ptr_, &stack_top_);
    }

    void Yield(std::optional<Y> yield_value) {
        yield(Status::kSuspended, yield_value);
    }

    void Call() {
        bound_fn_();
        yield(Status::kDead, std::optional<Y>{});
    }

    std::optional<Y> yield_value;
    Status status;

private:
    std::function<void()> bound_fn_;
    std::unique_ptr<coroutine_stack_t[]> stack_;
    // stack_top_ always points at topmost (i.e. lowest-address) full byte of
    // the stack. Since initially stack is empty, it means that the initial
    // value of stack_top_ is past the allocated memory region.
    coroutine_stack_t *stack_top_;
    coroutine_stack_t **caller_stack_top_ptr_;
    // ARM 64-bit requires stack to be 16 byte aligned. See
    // https://github.com/ARM-software/abi-aa/blob/main/aapcs64/aapcs64.rst#6221universal-stack-constraints
    static const std::align_val_t kStackAlignmentBytes = std::align_val_t(16);
    static const int kStackLength = (32 * 1024) / sizeof(coroutine_stack_t);
    static const int kInitialStackOffset = 0x100 / sizeof(coroutine_stack_t);
    static const int kLinkRegisterOffset = 0x98 / sizeof(coroutine_stack_t);

    void yield(Status new_status, std::optional<Y> yield_value) {
        if (status != Status::kRunning) {
            throw StatusViolationError::New(status, {Status::kRunning});
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
    current_coroutine<Y>->Yield(std::optional{yield_value});
}

}
