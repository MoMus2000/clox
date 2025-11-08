#ifndef clox_hashtable_h

#define clox_hashtable_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 2048

typedef struct {
  char* key;
  void* value;
} Entry;

typedef struct {
  Entry** entries;
  int count;
  int capacity;
} Map;

unsigned int simpleHash(const char* str);
void growCapacity(Map* map);
void addKey(char* key, void* value, Map* map);
Entry* getEntry(char* key, Map* map);
#endif
