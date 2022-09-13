#include <coroutine/example.hpp>
#include <functional>

bool Func1(int Arg1, std::function<bool(int)> Arg2) { return Arg2(Arg1); }
