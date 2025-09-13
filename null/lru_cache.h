#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <list>
#include <iostream>
#include <stdexcept>
#include <unordered_map>

template <typename K, typename V>
class lru_cache
{
  using value_type = std::pair<K, V>;
  using list_iterator = typename std::list<value_type>::iterator;

public:
  explicit lru_cache (size_t capacity) : capacity_ (capacity)
  {
    if (capacity_ == 0)
      throw std::invalid_argument ("LRUCache capacity must be positive.");
  }

  V *
  get (const K &key)
  {
    auto it = map_.find (key);
    if (it == map_.end ())
      return nullptr;

    list_.splice (list_.begin (), list_, it->second);
    return &it->second->second;
  }

  void
  put (const K &key, V value)
  {
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

  void
  print () const
  {
    std::cout << "LRU Cache Content (MRU -> LRU): ";
    for (const auto &pair : list_)
      std::cout << "[" << pair.first << ":" << pair.second << "] ";
    std::cout << std::endl;
  }

private:
  size_t capacity_;
  std::list<value_type> list_;
  std::unordered_map<K, list_iterator> map_;
};

#endif // LRU_CACHE_H
