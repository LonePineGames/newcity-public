#ifndef POOL_H
#define POOL_H

#include "cup.hpp"
#include "serialize.hpp"
#include "string_proxy.hpp"

#include <vector>
#include "spdlog/spdlog.h"

using namespace std;

template <typename T>
class Pool {

  public:
    static Pool* newPool(item expectedSize);
    item create();
    item create(bool clear);
    T* get(item ndx);
    void free(item ndx);
    void settle(item ndx);
    void defragment(const char* type);
    int size();
    int count();
    void clear();
    void reserve(int amount);
    void read(FileBuffer* file, int version);
    void write(FileBuffer* file);

//  private:
    Cup<T> array;
    vector<item> gaps;
    vector<item> settling;
};

template <typename T>
Pool<T>* Pool<T>::newPool(item expectedSize) {
  Pool<T>* pool = new Pool();
  pool->array.clear();
  pool->array.reserve(expectedSize);
  pool->gaps.clear();
  pool->settling.clear();
  return pool;
}

template <typename T>
void Pool<T>::defragment(const char* type) {
  if (this->gaps.size() == 0) return;

  reverse(this->gaps.begin(), this->gaps.end());
  sort(this->gaps.begin(), this->gaps.end());//, greater<item>());
  item ndx = this->gaps.back();
  item num = 0;
  while (ndx > 100 && ndx == this->array.size() && this->gaps.size() > 1) {
    this->gaps.pop_back();
    this->array.pop_back();
    ndx = this->gaps.back();
    num ++;
  }
  reverse(this->gaps.begin(), this->gaps.end());

  /*
  if (num > 0) {
    SPDLOG_INFO("Collected {} garbage {}, final size {},\n{} exist,"
        " {} settling, {} free, gaps {}-{}",
        num, type, this->array.size(), this->count(),
        this->settling.size(), this->gaps.size(), this->gaps.back(), ndx);
  }
  */
}

template <typename T>
item Pool<T>::create(bool clear) {
  if (this->gaps.size() > 0) {
    item ndx = gaps.back();
    this->gaps.pop_back();
    if (clear) memset_s(this->get(ndx), 0, sizeof(T));
    return ndx;

  } else {
    T elem;
    memset_s(&elem, 0, sizeof(T));
    this->array.push_back(elem);
    return this->array.size();
  }
}

template <typename T>
item Pool<T>::create() {
  return create(true);
}

template <typename T>
T* Pool<T>::get(item ndx) {
  return this->array.get(ndx-1);
}

template <typename T>
void Pool<T>::free(item ndx) {
  this->gaps.push_back(ndx);
  for (int i = this->settling.size()-1; i >= 0; i--) {
    if (this->settling[i] == ndx) {
      this->settling.erase(this->settling.begin()+i);
    }
  }
}

template <typename T>
void Pool<T>::settle(item ndx) {
  this->settling.push_back(ndx);
}

template <typename T>
int Pool<T>::size() {
  return this->array.size();
}

template <typename T>
void Pool<T>::reserve(int amount) {
  this->array.reserve(amount);
}

template <typename T>
void Pool<T>::clear() {
  this->array.clear();
  this->gaps.clear();
  this->settling.clear();
}

template <typename T>
int Pool<T>::count() {
  return this->array.size() - this->gaps.size() - this->settling.size();
}

template <typename T>
void Pool<T>::write(FileBuffer* file) {
  vector<item> allGaps = this->gaps;
  allGaps.insert(allGaps.begin(), this->settling.begin(), this->settling.end());
  fwrite_item_vector(file, &allGaps);
  fwrite_int(file, this->array.size());
}

template <typename T>
void Pool<T>::read(FileBuffer* file, int version) {
  this->array.clear();
  fread_item_vector(file, &this->gaps, version);
  int size = fread_int(file);
  this->array.resize(size);
}

#endif
