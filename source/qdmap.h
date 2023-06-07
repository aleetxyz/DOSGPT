#ifndef QDMAP_H
#define QDMAP_H

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  char *key;
  char *value;
} KeyValuePair;

typedef struct {
  KeyValuePair *entries;
  int count;
  int capacity;
} Map;

void initializeMap(Map *map) {
  map->entries = NULL;
  map->count = 0;
  map->capacity = 0;
}

int addToMap(Map *map, const char *key, const char *value) {
  KeyValuePair entry;
  entry.key = strdup(key);
  entry.value = strdup(value);

  // Check if capacity needs to be increased
  if (map->count >= map->capacity) {
    int newCapacity = (map->capacity == 0) ? 4 : map->capacity * 2;
    KeyValuePair *newEntries = (KeyValuePair*)realloc(map->entries, newCapacity * sizeof(KeyValuePair));
    if (newEntries == NULL) {
      return -1;
    }
    map->entries = newEntries;
    map->capacity = newCapacity;
  }

  map->entries[map->count] = entry;
  map->count++;

  return 0;
}

int hasValue(const Map *map, const char *key) {
  for (int i = 0; i < map->count; i++) {
    if (strcmp(map->entries[i].key, key) == 0) {
      return 0;
    }
  }
  return -1;
}

const char *getValue(const Map *map, const char *key) {
  for (int i = 0; i < map->count; i++) {
    if (strcmp(map->entries[i].key, key) == 0) {
      return map->entries[i].value;
    }
  }
  return NULL;
}

void freeMap(Map *map) {
  for (int i = 0; i < map->count; i++) {
    free(map->entries[i].key);
    free(map->entries[i].value);
  }
  free(map->entries);
  map->count = 0;
  map->capacity = 0;
}

#ifdef __cplusplus
}
#endif

#endif
