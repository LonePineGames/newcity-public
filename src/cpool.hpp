#pragma once

#include "item.hpp"
#include "cup.hpp"
#include "serialize.hpp"
#include "string_proxy.hpp"
using namespace std;

template <typename T>
class CPool {
  public:
    static CPool* newPool(item expectedSize);
    item create();
    T* get(item ndx);
    void free(item ndx);
    int size();
    int count();
    void clear();
    void empty();
    void reserve(int amount);
    void read(FileBuffer* file, int version);
    void write(FileBuffer* file);
  private:
    Cup<T> array;
    Cup<item> gaps;
};

template <typename T>
CPool<T>* CPool<T>::newPool(item expectedSize) {
  CPool<T>* pool = new CPool();
  pool->array.empty();
  pool->array.reserve(expectedSize);
  pool->gaps.empty();
  return pool;
}

template <typename T>
item CPool<T>::create() {
  if (this->gaps.size() > 0) {
    item ndx = gaps.back();
    this->gaps.pop_back();
    memset_s(this->get(ndx), 0, sizeof(T));
    return ndx;
  } else {
    T elem;
    memset_s(&elem, 0, sizeof(T));
    this->array.push_back(elem);
    return this->array.size();
  }
}

template <typename T>
T* CPool<T>::get(item ndx) {
  return this->array.get(ndx-1);
}

template <typename T>
void CPool<T>::free(item ndx) {
  this->gaps.push_back(ndx);
}

template <typename T>
int CPool<T>::size() {
  return this->array.size();
}

template <typename T>
void CPool<T>::reserve(int amount) {
  this->array.reserve(amount);
}

template <typename T>
void CPool<T>::clear() {
  this->array.clear();
  this->gaps.clear();
}

template <typename T>
void CPool<T>::empty() {
  this->array.empty();
  this->gaps.empty();
}

template <typename T>
int CPool<T>::count() {
  return this->array.size() - this->gaps.size();
}

template <typename T>
void CPool<T>::write(FileBuffer* file) {
  this->gaps.write(file);
  fwrite_int(file, this->array.size());
  printf("write cpool: %d,%d\n",
      (int)this->array.size(), (int)this->gaps.size());
}

template <typename T>
void CPool<T>::read(FileBuffer* file, int version) {
  this->array.empty();
  this->gaps.read(file, version);
  int size = fread_int(file);
  this->array.resize(size);
  printf("read cpool: %d,%d\n",
      size, (int)this->gaps.size());
}

