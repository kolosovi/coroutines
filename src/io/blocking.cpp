#include <io/blocking.hpp>

#include <io/addrinfo.hpp>

#include <iostream>
#include <string>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <netdb.h>

#include <fmt/core.h>

namespace {

static const int kDomain = PF_INET6;
static const int kSocketType = SOCK_STREAM;
static const int kProtocol = 0;
static const int kInvalidSocketDescriptor = -1;
static const std::string kHTTPServiceName{"http"};

void ThrowErrorWithErrno(const std::string &message) {
  int saved_errno = errno;
  throw std::runtime_error{fmt::format("{}: errno {}", message, saved_errno)};
}

io::Addrinfo ResolveHost(const std::string &host) {
  struct addrinfo getaddrinfo_hints;
  getaddrinfo_hints.ai_flags = 0;
  getaddrinfo_hints.ai_family = kDomain;
  getaddrinfo_hints.ai_socktype = kSocketType;
  getaddrinfo_hints.ai_protocol = kProtocol;
  getaddrinfo_hints.ai_addrlen = 0;
  getaddrinfo_hints.ai_addr = nullptr;
  getaddrinfo_hints.ai_canonname = nullptr;
  getaddrinfo_hints.ai_next = nullptr;
  addrinfo *an_addrinfo = nullptr;
  auto getaddrinfo_code = getaddrinfo(host.c_str(), kHTTPServiceName.c_str(),
                                      &getaddrinfo_hints, &an_addrinfo);
  if (getaddrinfo_code) {
    throw std::runtime_error{
        fmt::format("failed to resolve hostname {}: error_code {} ({})", host,
                    getaddrinfo_code, gai_strerror(getaddrinfo_code))};
  }
  if (!an_addrinfo) {
    throw std::runtime_error{
        fmt::format("got empty addrinfo when resolving host {}", host)};
  }
  if (an_addrinfo->ai_addr == nullptr) {
    throw std::runtime_error{
        fmt::format("addrinfo for host {} contains empty sockaddr", host)};
  }
  return io::Addrinfo(an_addrinfo);
}

}  // namespace

namespace io {

int BlockingDownload(const std::string host, const std::string path) {
  std::cout << "fetching " << path << " from " << host << "\n";
  auto address = ResolveHost(host);
  auto raw_addrinfo = address.Get();
  auto a_socket = socket(raw_addrinfo->ai_family, raw_addrinfo->ai_socktype,
                         raw_addrinfo->ai_protocol);
  if (a_socket == kInvalidSocketDescriptor) {
    ThrowErrorWithErrno("failed to create a socket descriptor");
  }
  auto connect_code =
      connect(a_socket, raw_addrinfo->ai_addr, raw_addrinfo->ai_addrlen);
  if (connect_code) {
    ThrowErrorWithErrno("connect failed");
  }
  return a_socket;
}

}  // namespace io
