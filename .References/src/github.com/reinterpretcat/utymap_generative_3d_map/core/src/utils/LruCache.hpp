#ifndef UTILS_LRUCACHE_HPP_DEFINED
#define UTILS_LRUCACHE_HPP_DEFINED

#include <stdexcept>
#include <list>
#include <map>

namespace utymap {
namespace utils {

template<typename Key, typename Value, typename Comparator = std::less<Key>>
class LruCache final {
  typedef typename std::pair<Key, Value> KeyValuePair;
  typedef typename std::list<KeyValuePair>::iterator ListIterator;
 public:

  LruCache(size_t maxSize) : maxSize_(maxSize) {
  }

  void put(const Key &key, Value &&value) {
    itemsList_.push_front(KeyValuePair(key, std::move(value)));

    auto it = itemsMap_.find(key);
    if (it!=itemsMap_.end()) {
      itemsList_.erase(it->second);
      itemsMap_.erase(it);
    }
    itemsMap_[key] = itemsList_.begin();

    if (itemsMap_.size() > maxSize_) {
      auto last = itemsList_.end();
      --last;
      itemsMap_.erase(last->first);
      itemsList_.pop_back();
    }
  }

  const Value &get(const Key &key) {
    auto it = itemsMap_.find(key);
    if (it==itemsMap_.end())
      throw std::range_error("There is no such key in cache.");

    itemsList_.splice(itemsList_.begin(), itemsList_, it->second);
    return it->second->second;
  }

  bool exists(const Key &key) const {
    return itemsMap_.find(key)!=itemsMap_.end();
  }

  size_t size() const {
    return itemsMap_.size();
  }

 private:
  std::list<KeyValuePair> itemsList_;
  std::map<Key, ListIterator, Comparator> itemsMap_;
  size_t maxSize_;
};

}
}
#endif // UTILS_LRUCACHE_HPP_DEFINED
