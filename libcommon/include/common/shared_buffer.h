#pragma once

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <optional>
#include <string>
#include <utility>

namespace common {

class SharedBuffer {
 public:
  static constexpr std::size_t kDefaultCapacity = 64;

  explicit SharedBuffer(std::size_t capacity = kDefaultCapacity)
      : capacity_(capacity) {}

  SharedBuffer(const SharedBuffer&) = delete;
  SharedBuffer& operator=(const SharedBuffer&) = delete;

  void Put(std::string data) {
    {
      std::unique_lock<std::mutex> lock(mutex_);
      not_full_.wait(lock,
                     [this] { return items_.size() < capacity_ || closed_; });
      if (closed_) {
        return;
      }
      items_.push_back(std::move(data));
    }
    not_empty_.notify_one();
  }

  std::optional<std::string> Take() {
    std::unique_lock<std::mutex> lock(mutex_);
    not_empty_.wait(lock, [this] { return !items_.empty() || closed_; });
    if (items_.empty()) {
      return std::nullopt;
    }
    std::string taken = std::move(items_.front());
    items_.pop_front();
    lock.unlock();
    not_full_.notify_one();
    return taken;
  }

  void Close() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      closed_ = true;
    }
    not_empty_.notify_all();
    not_full_.notify_all();
  }

 private:
  const std::size_t capacity_;
  std::mutex mutex_;
  std::condition_variable not_empty_;
  std::condition_variable not_full_;
  std::deque<std::string> items_;
  bool closed_ = false;
};

}  // namespace common
