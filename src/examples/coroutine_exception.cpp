#include <stdexcept>

#include <coroutine/api.hpp>

void CoroutineBody() { throw std::runtime_error{"sike"}; }

int main() {
  auto bad_coro = coroutine::Create<int>(std::function<void()>(CoroutineBody));
  coroutine::Resume(&bad_coro);
}
