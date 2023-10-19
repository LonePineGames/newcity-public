#pragma once

#include "error.hpp"
#include "string_proxy.hpp"
#include "serialize.hpp"

const int cupMinAlloc = 10;
const float cupGrowthRate = 1.5f;

template <typename T>
class Cup {
  public:

    static Cup* newCup(int expectedSize);
    vector<T> toVector();
    void fromVector(vector<T> vec);
    T* get(int ndx);
    void set(int ndx, T value);
    T operator[](int ndx);
    T at(int ndx);
    T back();
    T* next();
    void push_back(T t);
    void insertAt(int ndx, T t);
    void pop_back();
    void remove(int ndx);
    void removeByValue(T t);
    bool contains(T t);
    void concat(Cup* other);

    int size();
    void clear();
    void empty();
    void copy(Cup<T>* dest);
    void reserve(int amount);
    void resize(int amount);
    void ensureSize(int amount);
    void read(FileBuffer* file, int version);
    void write(FileBuffer* file);

  private:
    T* data = 0;
    int capacity = 0;
    int num = 0;
};

template <typename T>
Cup<T>* Cup<T>::newCup(int expectedSize) {
  Cup<T>* cup = new Cup();
  cup->reserve(expectedSize);
  return cup;
}

template <typename T>
vector<T> Cup<T>::toVector() {
  vector<T> result;
  result.reserve(num);
  for (int i = 0; i < num; i++) {
    result.push_back(data[i]);
  }
  return result;
}

template <typename T>
void Cup<T>::fromVector(vector<T> vec) {
  resize(vec.size());
  for (int i = 0; i < num; i++) {
    data[i] = vec[i];
  }
}

template <typename T>
T* Cup<T>::get(int ndx) {
  return &data[ndx];
}

template <typename T>
void Cup<T>::set(int ndx, T value) {
  data[ndx] = value;
}

template <typename T>
T Cup<T>::operator[](int ndx) {
  return data[ndx];
}

template <typename T>
T Cup<T>::at(int ndx) {
  return data[ndx];
}

template <typename T>
T Cup<T>::back() {
  return data[num-1];
}


template <typename T>
void Cup<T>::push_back(T t) {
  ensureSize(num+1);
  data[num-1] = t;
}

template <typename T>
void Cup<T>::insertAt(int ndx, T t) {
  ensureSize(num+1);
  for (int i = num-1; i > ndx; i --) {
    data[i] = data[i-1];
  }

  data[ndx] = t;
}

template <typename T>
T* Cup<T>::next() {
  ensureSize(num+1);
  return &data[num-1];
}

template <typename T>
void Cup<T>::pop_back() {
  num --;
}

template <typename T>
void Cup<T>::removeByValue(T t) {
  int offset = 0;
  for (int i = 0; i < num; i ++) {
    if (data[i] == t) {
      offset ++;
    } else if (offset > 0) {
      data[i-offset] = data[i];
    }
  }
  num -= offset;
}

template <typename T>
bool Cup<T>::contains(T t) {
  int offset = 0;
  for (int i = 0; i < num; i ++) {
    if (data[i] == t) {
      return true;
    }
  }
  return false;
}

template <typename T>
void Cup<T>::remove(int ndx) {
  for (int i = ndx; i < num-1; i ++) {
    data[i] = data[i+1];
  }
  num --;
}

template <typename T>
int Cup<T>::size() {
  return num;
}

template <typename T>
void Cup<T>::clear() {
  if (data != 0) {
    free(data);
  }
  data = 0;
  capacity = 0;
  num = 0;
}

template <typename T>
void Cup<T>::empty() {
  num = 0;
}

template <typename T>
void Cup<T>::copy(Cup<T>* dest) {
  dest->resize(num);
  dest->num = num;
  //dest->data = (T*) malloc(num * sizeof(T));
  if (data != 0) {
    memcpy_s (dest->data, data, sizeof(T)*num);
  }
}

template <typename T>
void Cup<T>::reserve(int amount) {
  if (amount > capacity) {
    if (capacity == 0 || data == 0) {
      data = (T*) malloc(amount * sizeof(T));
    } else {
      data = (T*) realloc(data, amount * sizeof(T));
    }
    memset_s(&data[capacity], 0, (amount-capacity)*sizeof(T));
    capacity = amount;
  }
}

template <typename T>
void Cup<T>::resize(int amount) {
  reserve(amount);
  num = amount;
}

template <typename T>
void Cup<T>::ensureSize(int amount) {
  if (amount >= capacity) {
    int newCap = capacity;
    if (newCap < cupMinAlloc) newCap = cupMinAlloc;
    while (newCap <= amount) newCap *= cupGrowthRate;
    reserve(newCap);
  }

  if (num < amount) {
    num = amount;
  }
}

template <typename T>
void Cup<T>::concat(Cup* other) {
  int oNum = other->size();
  reserve(num + oNum);
  memcpy_s(&data[num], other->data, oNum*sizeof(T));
  num += oNum;
}

template <typename T>
void Cup<T>::read(FileBuffer* file, int version) {
  clear();
  num = fread_int(file);
  if (num > 1000000 || num < 0) {
    handleError("Corrupt file");
  }
  reserve(num);
  fread(data, sizeof(T), num, file);
}

template <typename T>
void Cup<T>::write(FileBuffer* file) {
  fwrite_int(file, num);
  fwrite(data, sizeof(T), num, file);
}

