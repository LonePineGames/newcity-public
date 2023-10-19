#pragma once

#include "../item.hpp"

struct Heap {
  item capacity = 0;
  item size = 0;
  item* heap = 0;
  item* into = 0;
  float* priority = 0;
};

Heap initHeap(item capacity);
Heap initRouterHeap(item size);
void reinitRouterHeap(Heap* heap, item size);
void freeHeap(Heap* h);
item heapPop(Heap* h);
void heapPriority(Heap* h, item d, float priority);

