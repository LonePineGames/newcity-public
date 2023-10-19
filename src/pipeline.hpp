#pragma once

#include "cup.hpp"
#include "error.hpp"

#include <atomic>

template <typename T>
class Pipeline {
  public:
    void push(T t);
    bool pop(T* t);
    void resetToSize(uint32_t size);
    uint32_t itemsInPipeline();
  private:
    Cup<T> data;
    std::atomic<uint32_t> pushCursor;
    std::atomic<uint32_t> popCursor;
};

template <typename T>
void Pipeline<T>::push(T t) {
  if (pushCursor - popCursor > data.size()) {
    handleError("Pipeline Full");
  }
  item cursor = pushCursor ++;
  cursor %= data.size();
  data.set(cursor, t);
}

template <typename T>
bool Pipeline<T>::pop(T* t) {
  if (popCursor < pushCursor) {
    item cursor = popCursor++;
    cursor %= data.size();
    *t = data[cursor];
    return true;
  } else {
    return false;
  }
}

template <typename T>
void Pipeline<T>::resetToSize(uint32_t size) {
  pushCursor = 0;
  popCursor = 0;
  data.clear();
  data.resize(size);
}

template <typename T>
uint32_t Pipeline<T>::itemsInPipeline() {
  return pushCursor - popCursor;
}

