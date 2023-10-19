#pragma once

#include "spdlog/spdlog.h"
#include "../serialize.hpp"
#include "../util.hpp"

#include <unordered_map>
using namespace std;

template <typename V>
struct CacheEntry {
  uint64_t ndx;
  V entry;
};

template <typename K, typename V>
class Cache {

  public:
    Cache(uint64_t capacity, K (*keyCallback)(V proto)):
      cap(capacity), key(keyCallback) {}
    std::tuple<bool, V> get(V proto);
    void put(V val);
    bool erase(K k);
    bool erase(V proto);
    size_t size();
    void clear();
    void read(FileBuffer* file, int version,
        V (*readCallback)(FileBuffer* file, int version));
    void write(FileBuffer* file,
        void (*writeCallback)(FileBuffer* file, V v));
    void setCapacity(uint64_t capacity);
    void discardRandom();

  private:
    bool ageCheck(CacheEntry<V> e, K k, bool drop);
    uint64_t cap, ndx = 0;
    unordered_map<K, CacheEntry<V>> map;
    vector<K> array;
    K (*key)(V proto);
};

template <typename K, typename V>
bool Cache<K, V>::erase(K k) {
  auto result = map.find(k);
  if (result == map.end()) {
    return false;
  } else {
    CacheEntry<V> e = (*result).second;
    map.erase(result);
    if (array.size() > e.ndx) {
      array[e.ndx] = 0;
    }
    return true;
  }
}

template <typename K, typename V>
bool Cache<K, V>::ageCheck(CacheEntry<V> e, K k, bool drop) {
  if (ndx > cap && e.ndx < ndx - cap) {
    if (drop) erase(k);
    //SPDLOG_INFO("erased old cache entry {} {} {} {}", ndx, cap, e.ndx, k);
    return false;
  } else {
    return true;
  }
}

template <typename K, typename V>
std::tuple<bool, V> Cache<K, V>::get(V proto) {
  K k = key(proto);
  auto result = map.find(k);
  if (result == map.end()) {
    V v;
    return std::make_tuple(false, v);
  } else {
    CacheEntry<V> e = (*result).second;
    ageCheck(e, k, true);
    return std::make_tuple(true, e.entry);
  }
}

template <typename K, typename V>
void Cache<K, V>::put(V proto) {
  CacheEntry<V> e{ndx, proto};
  ndx ++;
  K k = key(proto);
  map[k] = e;

  if (array.size() <= e.ndx) {
    array.resize(e.ndx*1.5+10);
  }
  array[e.ndx] = k;
}

template <typename K, typename V>
bool Cache<K, V>::erase(V proto) {
  return erase(key(proto));
}

template <typename K, typename V>
void Cache<K, V>::discardRandom() {
  for (int i = 0; i < 10; i++) {
    K k = array[randItem(ndx)];
    if (k != 0) {
      erase(k);
      return;
    }
  }
}

template <typename K, typename V>
void Cache<K, V>::clear() {
  ndx = 0;
  map.clear();
  array.clear();
}

template <typename K, typename V>
void Cache<K, V>::setCapacity(uint64_t capacity) {
  cap = capacity;
}

template <typename K, typename V>
size_t Cache<K, V>::size() {
  return map.size();
}

template <typename K, typename V>
void Cache<K, V>::write(FileBuffer* file,
        void (*writeCallback)(FileBuffer* file, V v)) {

  FileBuffer* hold = makeFileBufferPtr();
  fwrite_uint64_t(file, ndx);
  fwrite_uint64_t(file, cap);

  int num = 0;
  for (auto it = map.begin(); it != map.end(); it++) {
    CacheEntry<V> e = it->second;
    if (ageCheck(e, it->first, false)) {
      fwrite_uint64_t(hold, e.ndx);
      writeCallback(hold, e.entry);
      num ++;
    }
  }

  SPDLOG_INFO("writing {} cached routes", num);
  fwrite_uint64_t(file, num);
  fwrite_buffer(file, hold);
  freeBuffer(hold);
}

template <typename K, typename V>
void Cache<K, V>::read(FileBuffer* file, int version,
        V (*readCallback)(FileBuffer* file, int version)) {

  fread_uint64_t(file); // ndx
  fread_uint64_t(file); // cap
  uint64_t num = fread_uint64_t(file);
  SPDLOG_INFO("reading {} cached routes", num);

  for (int i = 0; i < num; i ++) {
    fread_uint64_t(file); // e.ndx
    put(readCallback(file, version));
  }
}

