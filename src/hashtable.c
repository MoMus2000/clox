#include "hashtable.h"

unsigned int simpleHash(const char* str) {
    unsigned int hash = 5381;
    while (*str) {
        hash = ((hash << 5) + hash) + (unsigned char)(*str++);
    }
    return hash;
}

void growCapacity(HashMap* map) {
  int newCapacity = map->capacity * 2;

  HashEntry** entries = calloc(newCapacity, sizeof(HashEntry*));

  for(int i=0; i<map->capacity; i++){
    HashEntry* entry = map->entries[i];
    if(entry != NULL){
      int newHash = simpleHash(entry->key) % newCapacity;
      while(entries[newHash] != NULL){
        newHash = (newHash + 1) % newCapacity;
      }
      entries[newHash] = entry;
    }
  }
  
  free(map->entries);
  map->entries = entries;
  map->capacity = newCapacity;
}

void addKey(char* key, void* value, HashMap* map){
  if(((float)map->count / map->capacity) > 0.30f){
    growCapacity(map);
  }

  HashEntry* entry = malloc(sizeof(HashEntry));

  int hash = simpleHash(key) % map->capacity;

  if(map->entries[hash] == NULL) {
    entry->key = malloc(strlen(key)+1);
    strcpy(entry->key, key);
    entry->value = value;
    map->entries[hash] = entry;
  } else {
    int idx = hash;
    while(map->entries[idx]!= NULL){
      idx = (idx + 1) % map->capacity;
    }
    entry->key = malloc(strlen(key)+1);
    strcpy(entry->key, key);
    entry->value = value;
    map->entries[idx] = entry;
  }
  map->count ++;
}


HashEntry* getEntry(char* key, HashMap* map) {
  int capacity = map->capacity;
  int hash = simpleHash(key) % capacity;
  int start = hash;

  while(map->entries[hash] != NULL) {
    if (strcmp(map->entries[hash]->key, key) == 0){
      return map->entries[hash];
    }
    hash = (hash + 1) % capacity;
    if (hash == start) break;  // full circle, not found
  }

  return NULL;
}

