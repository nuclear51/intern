#pragma once

#include <unistd.h>

#include <utility>

namespace common {

class FdWrapped {
 public:
  FdWrapped() = default;
  explicit FdWrapped(int fd) : fd_(fd) {}
  ~FdWrapped() { Reset(); }

  FdWrapped(FdWrapped&& other) noexcept : fd_(other.Release()) {}
  FdWrapped& operator=(FdWrapped&& other) noexcept {
    if (this != &other) {
      Reset(other.Release());
    }
    return *this;
  }

  FdWrapped(const FdWrapped&) = delete;
  FdWrapped& operator=(const FdWrapped&) = delete;

  int fd() const { return fd_; }
  bool IsValid() const { return fd_ >= 0; }

  void Reset(int fd = kInvalidFd) {
    if (fd_ >= 0) {
      close(fd_);
    }
    fd_ = fd;
  }

  int Release() { return std::exchange(fd_, kInvalidFd); }

 private:
  static constexpr int kInvalidFd = -1;

  int fd_ = kInvalidFd;
};

}  // namespace common
