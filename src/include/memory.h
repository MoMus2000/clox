#ifndef clox_memory_h
#define clox_memory_h

#include "common.h"
#include "object.h"


#define GROW_CAPACITY(capacity)\
  ((capacity) < 8 ? 8: (capacity) * 2)

#define GROW_ARRAY(type, pointer, oldCapacity, newCapacity) \
  (type*)reallocate(pointer, sizeof(type) * (oldCapacity), sizeof(type) * (newCapacity))

#define FREE_ARRAY(type, pointer, oldCapacity) \
  (type*)reallocate(pointer, sizeof(type) * (oldCapacity), 0)

#define ALLOCATE(type, count) \
  (type*)reallocate(NULL, 0, sizeof(type) * (count))

#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0)

void* reallocate(void* pointer, size_t oldSize, size_t newSize);

static void freeObject(Obj* object) {
  switch (object->type) {
    case OBJ_STRING: {
      ObjString* string = (ObjString*)object;
      FREE_ARRAY(char, string->chars, string->length + 1);
      FREE(ObjString, object);
      break;
    }
  }
}
void freeObjects();

#endif
