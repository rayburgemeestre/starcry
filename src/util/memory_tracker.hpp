#pragma once

#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>

#include <boost/uuid/uuid.hpp>             // for uuid
#include <boost/uuid/uuid_generators.hpp>  // for uuid_generators
#include <boost/uuid/uuid_io.hpp>          // for to_string

class memory_tracker;

class tracked_memory {
public:
  static tracked_memory& instance() {
    static tracked_memory instance;
    return instance;
  }

  void register_tracker(const memory_tracker* tracker, const std::string& name) {
    std::unique_lock<std::mutex> lock(mutex_);
    trackers_[tracker] = name;
    memory_usage_[name] = 0;
    // // temporary
    // lock.unlock();
    // print_report();
  }

  void unregister_tracker(const memory_tracker* tracker) {
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = trackers_.find(tracker);
    if (it != trackers_.end()) {
      memory_usage_.erase(it->second);
      trackers_.erase(it);
      // // temporary
      // lock.unlock();
      // print_report();
    }
  }

  // Update memory usage
  void update_memory_usage(const memory_tracker* tracker, std::size_t memory, bool silent = false) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (trackers_.find(tracker) != trackers_.end()) {
      memory_usage_[trackers_[tracker]] += memory;
      // temporary
      // if (silent) return;
      // lock.unlock();
      // print_report();
    }
  }

  void print_report() const {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t cumulative_bytes = 0;
    for (const auto& pair : memory_usage_) {
      cumulative_bytes += pair.second;
      std::cout << pair.first << ": " << pair.second
                << " bytes (cumulative: " << (cumulative_bytes / 1024. / 1024. / 1024.) << " GB)" << std::endl;
    }
    std::cout << "---" << std::endl;
  }

private:
  mutable std::mutex mutex_;
  std::unordered_map<const memory_tracker*, std::string> trackers_;
  std::unordered_map<std::string, std::size_t> memory_usage_;

  tracked_memory() = default;
  tracked_memory(const tracked_memory&) = delete;
  tracked_memory& operator=(const tracked_memory&) = delete;
};

inline std::string make_name_unique(const std::string& name) {
  boost::uuids::random_generator generator;
  boost::uuids::uuid uuid1 = generator();
  std::string uuid_str = to_string(uuid1);

  std::ostringstream ss;
  ss << std::this_thread::get_id();
  std::string id_str = ss.str();

  return name + "-" + uuid_str + "-thread-id-" + id_str;
}

class memory_tracker {
public:
  memory_tracker(const std::string& name, std::size_t initial_memory = 0)
      : name_(make_name_unique(name)), memory_(initial_memory) {
    tracked_memory::instance().register_tracker(this, name_);
    tracked_memory::instance().update_memory_usage(this, memory_);
  }

  ~memory_tracker() {
    tracked_memory::instance().unregister_tracker(this);
  }

  void update_memory(std::size_t memory, bool silent = false) {
    tracked_memory::instance().update_memory_usage(this, memory - memory_, silent);
    memory_ = memory;
  }

private:
  std::string name_;
  std::size_t memory_;
};