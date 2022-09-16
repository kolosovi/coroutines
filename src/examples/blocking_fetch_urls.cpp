#include <iostream>
#include <string>
#include <array>

#include <examples/urls.hpp>
#include <io/blocking.hpp>

int main() {
  int total_byte_count = 0;
  for (auto path : kPaths) {
    total_byte_count += io::BlockingDownload(kHost, path);
  }
  std::cout << total_byte_count << "\n";
  return 0;
}
