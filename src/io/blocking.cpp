#include <io/blocking.hpp>

#include <io/addrinfo.hpp>

#include <iostream>
#include <string>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <fmt/core.h>

namespace {

static const int kAddressFamily = AF_INET6;
// PF_UNSPEC is another option
static const int kSocketType = SOCK_STREAM;
static const int kInvalidCode = -1;
static const int kRecvBufSize = 1024;
static const std::string kHTTPServiceName{"http"};

void ThrowErrorWithErrno(const std::string &message) {
  int saved_errno = errno;
  auto *description = std::strerror(saved_errno);
  throw std::runtime_error{
      fmt::format("{}: errno {} ({})", message, saved_errno, description)};
}

io::Addrinfo ResolveHost(const std::string &host) {
  struct addrinfo getaddrinfo_hints;
  getaddrinfo_hints.ai_flags = 0;
  getaddrinfo_hints.ai_family = kAddressFamily;
  getaddrinfo_hints.ai_socktype = kSocketType;
  getaddrinfo_hints.ai_protocol = 0;
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

void PrintAddresses(addrinfo *addrinfo_ptr) {
  for (auto *cur = addrinfo_ptr; cur != nullptr; cur = cur->ai_next) {
    char address_str[60];
    for (int i = 0; i < 60; ++i) {
      address_str[i] = 0;
    }
    void *addr;
    if (cur->ai_family == AF_INET) {
      sockaddr_in *ipv4 = reinterpret_cast<sockaddr_in *>(cur->ai_addr);
      addr = static_cast<void *>(&(ipv4->sin_addr));
    } else if (cur->ai_family == AF_INET6) {
      sockaddr_in6 *ipv6 = reinterpret_cast<sockaddr_in6 *>(cur->ai_addr);
      addr = static_cast<void *>(&(ipv6->sin6_addr));
    }
    inet_ntop(cur->ai_family, addr, address_str, sizeof(address_str));
    std::cout << fmt::format("address {}\n", address_str);
  }
}

void SendRequest(int socket, const std::string &host, const std::string &path) {
  auto req = fmt::format(
      "GET {} HTTP/1.1\r\nHost: {}\r\nUser-Agent: "
      "kolosovi/coroutines\r\nConnection: close\r\nAccept: */*\r\n\r\n",
      path, host);
  std::cout << req << std::endl;
  auto *buf = req.c_str();
  auto bytes_to_send = req.size();
  while (bytes_to_send > 0) {
    auto bytes_sent = send(socket, buf, bytes_to_send, 0);
    if (bytes_sent == kInvalidCode) {
      ThrowErrorWithErrno(fmt::format("cannot send HTTP request to {}", path));
    }
    bytes_to_send -= bytes_sent;
  }
}

int ReadResponse(int socket, const std::string &path) {
  std::unique_ptr<char> buf(new char[kRecvBufSize]);
  auto *raw_buf = buf.get();
  int count = 0;
  while (true) {
    auto bytes_received = recv(socket, raw_buf, kRecvBufSize, 0);
    if (bytes_received == kInvalidCode) {
      ThrowErrorWithErrno(
          fmt::format("cannot receive HTTP response for {}", path));
    }
    count += bytes_received;
    if (bytes_received == 0) {
      break;
    }
    for (int i = 0; i < bytes_received; i++) {
      std::cout << raw_buf[i];
    }
  }
  std::cout << std::endl;
  return count;
}

}  // namespace

namespace io {

int BlockingDownload(const std::string host, const std::string path) {
  std::cout << "fetching " << path << " from " << host << "\n";
  auto address = ResolveHost(host);
  auto raw_addrinfo = address.Get();
  PrintAddresses(raw_addrinfo);
  auto a_socket = socket(raw_addrinfo->ai_family, raw_addrinfo->ai_socktype,
                         raw_addrinfo->ai_protocol);
  if (a_socket == kInvalidCode) {
    ThrowErrorWithErrno("failed to create a socket descriptor");
  }
  auto connect_code =
      connect(a_socket, raw_addrinfo->ai_addr, raw_addrinfo->ai_addrlen);
  if (connect_code) {
    ThrowErrorWithErrno("connect failed");
  }
  SendRequest(a_socket, host, path);
  auto bytes_received = ReadResponse(a_socket, path);
  std::cout << "bytes received: " << bytes_received << std::endl;
  return bytes_received;
}

}  // namespace io
