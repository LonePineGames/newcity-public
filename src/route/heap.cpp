#include "heap.hpp"

#include <stdlib.h>
#include <float.h>

const int heapOutdegree = 16;

Heap initHeap(item capacity) {
  Heap result;
  result.capacity = capacity;
  result.size = 0;
  result.heap = (item*) calloc(capacity, sizeof(item));
  result.into = (item*) calloc(capacity, sizeof(item));
  result.priority = (float*) calloc(capacity, sizeof(float));
  return result;
}

void reinitHeap(Heap* heap, item capacity) {
  if (heap->capacity >= capacity) return;

  if (heap->heap != 0) {
    free(heap->heap);
    free(heap->into);
    free(heap->priority);
  }

  capacity *= 2;
  heap->capacity = capacity;
  heap->size = 0;
  heap->heap = (item*) calloc(capacity, sizeof(item));
  heap->into = (item*) calloc(capacity, sizeof(item));
  heap->priority = (float*) calloc(capacity, sizeof(float));
}

Heap initRouterHeap(item size) {
  Heap result = initHeap(size+1);
  result.size = size;

  for (int i = 0; i <= size; i++) {
    result.priority[i] = FLT_MAX;
    result.heap[i] = i;
    result.into[i] = i;
  }

  return result;
}

void reinitRouterHeap(Heap* heap, item size) {
  reinitHeap(heap, size+1);
  heap->size = size;

  for (int i = 0; i < heap->capacity; i++) {
    heap->priority[i] = FLT_MAX;
    heap->heap[i] = i;
    heap->into[i] = i;
  }
}

void freeHeap(Heap* h) {
  free(h->heap);
  free(h->priority);
  free(h->into);
}

void heapSwap(Heap* h, item i0, item i1) {
  item heap0 = h->heap[i1];
  item heap1 = h->heap[i0];
  h->heap[i0] = heap0;
  h->into[heap0] = i0;
  h->heap[i1] = heap1;
  h->into[heap1] = i1;
}

void heapify(Heap* h, item i) {
  while(i*heapOutdegree < h->size) {
    item ci0 = i*heapOutdegree;
    item ci = ci0;
    item c0 = h->heap[ci0];
    float d0 = h->priority[c0];
    float d = d0;
    item max = (ci0 + heapOutdegree >= h->size) ?
      h->size - ci0 - 1 : heapOutdegree;

    for (int k = 1; k <= max; k++) {
      item ci1 = ci0 + k;
      item c1 = h->heap[ci1];
      float d1 = h->priority[c1];
      if (d1 < d) {
        d = d1;
        ci = ci1;
      }
    }

    if (d >= h->priority[h->heap[i]]) {
      break;
    }

    if (ci == i) break;

    heapSwap(h, i, ci);
    i = ci;
  }
}

item heapPop(Heap* h) {
  if (h->size <= 0) {
    return -1;

  } else {
    item result = h->heap[0];
    h->size--;

    if (h->size > 0) {
      heapSwap(h, 0, h->size+1);
      h->heap[h->size+1] = -1;
      h->into[result] = -1;
      heapify(h, 0);

    } else {
      h->heap[h->size+1] = -1;
      h->into[result] = -1;
    }

    return result;
  }
}

void heapPriority(Heap* h, item d, float priority) {
  h->priority[d] = priority;
  item ci = h->into[d];

  while (ci > 0) {
    item pi = (ci-1)/heapOutdegree;

    if (h->priority[h->heap[pi]] < h->priority[h->heap[ci]]) {
      break;
    }

    heapSwap(h, pi, ci);
    ci = pi;
  }
}

