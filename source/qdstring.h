#ifndef QDSTRING_H
#define QDSTRING_H

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

int asprintf(char **str, const char *format, ...) {
  va_list args;
  va_start(args, format);

  // Determine the length of the formatted string
  int length = vsnprintf(NULL, 0, format, args);
  if (length < 0) {
    va_end(args);
    return -1;
  }

  // Allocate memory for the string
  *str = (char *)malloc((length + 1) * sizeof(char));
  if (*str == NULL) {
    va_end(args);
    return -1;
  }

  // Format the string
  length = vsnprintf(*str, length + 1, format, args);
  va_end(args);

  if (length < 0) {
    free(*str);
    return -1;
  }

  return length;
}

char *trimWhitespace(char *str) {
  char *end;

  // Trim leading whitespace
  while (isspace((unsigned char)*str))
    str++;

  if (*str == '\0') // All whitespace
    return str;

  // Trim trailing whitespace
  end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end))
    end--;

  // Null-terminate the trimmed string
  *(end + 1) = '\0';

  return str;
}

#ifdef __cplusplus
}
#endif

#endif
