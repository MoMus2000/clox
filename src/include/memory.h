#ifndef clox_memory_h
#define clox_memory_h

#include "common.h"


#define GROW_CAPACITY(capacity)\
  ((capacity) < 8 ? 8: (capacity) * 2)

#define GROW_ARRAY(type, pointer, oldCapacity, newCapacity) \
  (type*)reallocate(pointer, sizeof(type) * (oldCapacity), sizeof(type) * (newCapacity))

#define FREE_ARRAY(type, pointer, oldCapacity) \
  (type*)reallocate(pointer, sizeof(type) * (oldCapacity), 0)

#define ALLOCATE(type, count) \
  (type*)reallocate(NULL, 0, sizeof(type) * count)


void* reallocate(void* pointer, size_t oldSize, size_t newSize);

#endif
