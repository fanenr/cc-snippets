#ifndef CONCURRENT_LRU_CACHE_H
#define CONCURRENT_LRU_CACHE_H

#include <list>
#include <mutex>
#include <stdexcept>
#include <unordered_map>

template <typename K, typename V>
class concurrent_lru_cache
{
  using value_type = std::pair<K, V>;
  using list_iterator = typename std::list<value_type>::iterator;

public:
  using size_type = size_t;

  explicit concurrent_lru_cache (size_type capacity) : capacity_ (capacity)
  {
    if (capacity_ == 0)
      throw std::invalid_argument ("LRUCache capacity must be positive.");
  }

  V *
  get (const K &key)
  {
    std::lock_guard<std::mutex> lock (mutex_);

    auto it = map_.find (key);
    if (it == map_.end ())
      return nullptr;

    list_.splice (list_.begin (), list_, it->second);
    return &it->second->second;
  }

  void
  put (const K &key, V value)
  {
    std::lock_guard<std::mutex> lock (mutex_);

    auto it = map_.find (key);
    if (it != map_.end ())
      {
	auto list_it = it->second;
	list_it->second = std::move (value);
	list_.splice (list_.begin (), list_, list_it);
	return;
      }

    if (list_.size () >= capacity_)
      {
	map_.erase (list_.back ().first);
	list_.pop_back ();
      }

    list_.push_front ({ key, std::move (value) });
    map_[key] = list_.begin ();
  }

private:
  size_type capacity_;
  std::list<value_type> list_;
  std::unordered_map<K, list_iterator> map_;
  std::mutex mutex_;
};

#endif // CONCURRENT_LRU_CACHE_H
