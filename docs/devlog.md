# Goals

## Adapting to address sanitizer

Goal: make it work correctly with address sanitizer. To do this, we should make address sanitizer aware of context switches.

### Status

I was thinking that exceptions trigger this behaviour, but it seems that my code is just broken with respect to exceptions, even if no sanitizers are used.

Until this is fixed, this goal is on hold.

## Handling exceptions

Throwing an exception from a coroutine and not catching it leads to a segfault.

I must understand how exceptions work and fix this.

The desired behaviour is for this piece of code to print `"SUCCESS"`:

```
try {
    // bad_coro throws runtime_error without handling it
    bad_coro.Resume();
} catch(std::runtime_error exc) {
    std::cout << "SUCCESS" << std::endl;
}
```

### Progress

Idea: Let's write a simple example (no gtest, no nothing) and explore it.
No sanitizers either, the less code the better.

Then look at compiler output closely and figure out what is missing.

The minumum viable example is at `src/examples/coroutine_exception.cpp`.

Result: Looking at compiler output wasn't fruitful. Tracing execution with
debugger ultimately wasn't fruitful, too. I got lost while tracing stack
unwinding during exception.

Ultimately at some point of the stack unwinding process program counter
acquires the value of 0, which leads to segfault.

Idea: Maybe I break the ABI in some way that breaks exceptions. Armv8-a ABI says
that I must maintain frame pointer (register x29) and the Apple also tells to
maintain the frame pointer [here](https://developer.apple.com/documentation/xcode/writing-arm64-code-for-apple-platforms).

Result: Initializing frame pointer to point to base of the stack fixed the
segfault. Now the exception properly bubbles up the stack. The only problem is
that it just aborts the process rather than propagating to the coroutine that
resumed the one that throwed the exception.
