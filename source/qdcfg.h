#ifndef QDCFG_H
#define QDCFG_H

#include <stdlib.h>
#include <string.h>

#include "qdmap.h"
#include "qdstring.h"

#ifdef __cplusplus
extern "C" {
#endif

int parseConfigFile(const char *filename, Map *map) {
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    return -1;
  }

  char line[100];
  while (fgets(line, sizeof(line), file) != NULL) {
    char *separator = strchr(line, '=');
    if (separator != NULL) {
      *separator = '\0';
      char *key = trimWhitespace(line);
      char *value = trimWhitespace(separator + 1);

      // Skip empty lines or lines starting with '#'
      if (*key == '\0' || *key == '#')
        continue;

      if(strlen(key) > 0 && strlen(value) > 0)
        addToMap(map, key, value);
    }
  }

  fclose(file);
  return 0;
}

#ifdef __cplusplus
}
#endif

#endif
