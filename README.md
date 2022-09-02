An attempt at implementing coroutines for educational purposes.

*Warning*: it's not intended for production use.

This implementation is not cross-platform:
- It only targets 64-bit Arm. I'm building it on an Apple M1 machine, so it might not even work with other 64-bit CPUs.
- It targets clang ABI, so probably won't work with gcc.
