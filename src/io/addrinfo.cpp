#include <io/addrinfo.hpp>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <utility>

namespace io {
Addrinfo::Addrinfo(addrinfo *raw) : addrinfo_(raw) {}

Addrinfo::Addrinfo(Addrinfo &&other) : addrinfo_(nullptr) {
  std::swap(addrinfo_, other.addrinfo_);
}

Addrinfo &Addrinfo::operator=(Addrinfo &&other) {
  if (this == &other) {
    return *this;
  }
  FreeIfNonEmpty();
  std::swap(addrinfo_, other.addrinfo_);
  return *this;
}

Addrinfo::~Addrinfo() { FreeIfNonEmpty(); }

addrinfo *Addrinfo::Get() const { return addrinfo_; }

bool Addrinfo::IsEmpty() const { return addrinfo_ == nullptr; }

void Addrinfo::FreeIfNonEmpty() {
  if (addrinfo_ != nullptr) {
    freeaddrinfo(addrinfo_);
    addrinfo_ = nullptr;
  }
}
}  // namespace io
