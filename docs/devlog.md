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
