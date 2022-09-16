#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

namespace io {

class Addrinfo {
 public:
  Addrinfo() = delete;
  Addrinfo(const Addrinfo &) = delete;
  Addrinfo &operator=(const Addrinfo &) = delete;

  explicit Addrinfo(addrinfo *);
  Addrinfo &operator=(Addrinfo &&);
  Addrinfo(Addrinfo &&);
  ~Addrinfo();

  addrinfo *Get() const;
  bool IsEmpty() const;

 private:
  addrinfo *addrinfo_;

  void FreeIfNonEmpty();
};

}  // namespace io
