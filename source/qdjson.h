#ifndef QDJSON_H
#define QDJSON_H

#include <stdlib.h>
#include <string.h>

#include "qdstring.h"

#ifdef __cplusplus
extern "C" {
#endif

int16_t findValueForKeyJson(char *json, const char *key, char **out) {
  const char *keyStart = strstr(json, key);

  if (keyStart != NULL) {
    const char *valueStart = strchr(keyStart, ':');
    if (valueStart != NULL) {
      valueStart++; // Move past the ':' character

      while (*valueStart == ' ' || *valueStart == '\t' || *valueStart == '\n') {
        valueStart++; // Skip any whitespace characters
      }

      if (*valueStart == '"' || '`') {
        // String value enclosed in double quotes
        const char *valueEnd = valueStart + 1;
        while (*valueEnd != '\0') {
          if (*valueEnd == '\\') {
            // Handle escape sequence
            switch (valueEnd[1]) {
            case '\"': // Double quote
              valueEnd++;
              break;
            case '\\': // Backslash
              valueEnd++;
              break;
            case 'b': // Backspace
              valueEnd++;
              break;
            case 'f': // Form feed
              valueEnd++;
              break;
            case 'n': // Newline
              valueEnd++;
              break;
            case 'r': // Carriage return
              valueEnd++;
              break;
            case 't': // Tab
              valueEnd++;
              break;
            case 'u': // Unicode escape sequence (not handled in this example)
              valueEnd += 5;
              break;
            default:
              break;
            }
          } else if (*valueEnd == '"') {
            break; // End of string value
          }
          valueEnd++;
        }

        int16_t length = valueEnd - valueStart - 1;
        asprintf(out, "%.*s", length, valueStart + 1);
        return length;
      } else {
        // Other value types (array, object, number, boolean, null)
        const char *valueEnd = valueStart;
        while (*valueEnd != ',' && *valueEnd != '}' && *valueEnd != ']' &&
               *valueEnd != '\0') {
          valueEnd++;
        }
        int length = valueEnd - valueStart;
        asprintf(out, "%.*s", length, valueStart + 1);
        return length;
      }
    }
  }
  return -1;
}

int16_t buildCompletionJson(char **bodyString, char *model, char *msg, char *temp) {
  float fTemp = atof(temp);

  int16_t parsedBytes = asprintf(bodyString,
    "{"
    "\"model\": \"%s\","
    "\"messages\": [{\"role\": \"user\", \"content\": \"%s\"}],"
    "\"temperature\": %0.2f"
    "}",
    model, msg, fTemp
  );

  return parsedBytes;
}

#ifdef __cplusplus
}
#endif

#endif
