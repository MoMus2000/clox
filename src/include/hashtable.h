#ifndef clox_hashtable_h

#define clox_hashtable_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 2048

typedef struct {
  char* key;
  void* value;
} HashEntry;

typedef struct {
  HashEntry** entries;
  int count;
  int capacity;
} HashMap;

unsigned int simpleHash(const char* str);
void growCapacity(HashMap* map);
void addKey(char* key, void* value, HashMap* map);
HashEntry* getEntry(char* key, HashMap* map);
#endif
