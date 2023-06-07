#ifndef QDHTTP_H
#define QDHTTP_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qdstring.h"

#ifdef __cplusplus
extern "C" {
#endif

void charToUint8(const char* src, uint8_t* destiny) {
  size_t length = strlen(src);

  for (size_t i = 0; i < length; i++) {
    destiny[i] = (uint8_t)src[i];
  }
}

int16_t buildPostRequest(uint8_t* sendBuffer, char* host, char* path, char* body) {
  int16_t bodyLen = strlen(body);
  char contLen[10];
  snprintf(contLen, 10, "%d", (int)bodyLen);

  char* reqBody = NULL;
  int16_t msgLength = asprintf(&reqBody,
    "POST %s HTTP/1.1\r\n"
    "User-Agent: mTCP HTGet\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: %s\r\n"
    "Host: %s\r\n"
    "Connection: close\r\n\r\n"
    "%s",
    path,
    contLen,
    host,
    body
  );

  charToUint8(reqBody, sendBuffer);
  free(reqBody);

  return msgLength;
}

int16_t buildAuthPostRequest(uint8_t* sendBuffer, char* host, char* path, char* body, char* auth) {
  int16_t bodyLen = strlen(body);
  char contLen[10];
  snprintf(contLen, 10, "%d", (int)bodyLen);

  char* reqBody = NULL;
  int16_t msgLength = asprintf(&reqBody,
    "POST %s HTTP/1.1\r\n"
    "User-Agent: mTCP HTGet\r\n"
    "Authorization: Bearer %s\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: %s\r\n"
    "Host: %s\r\n"
    "Connection: close\r\n\r\n"
    "%s",
    path,
    auth,
    contLen,
    host,
    body
  );

  charToUint8(reqBody, sendBuffer);
  free(reqBody);

  return msgLength;
}

int8_t extractResponse(char *response, char **line, char **headers, char **body) {
  char *lineEnd = strchr(response, '\n');
  if (lineEnd == NULL)
    return -1;

  size_t lineLength = lineEnd - response;
  *line = (char *)malloc((lineLength + 1) * sizeof(char));
  snprintf(*line, lineLength + 1, "%s", response);

  // Find the end of the headers section
  char *headersEnd = strstr(lineEnd, "\r\n\r\n");
  if (headersEnd == NULL) {
    free(*line);
    return -1;
  }

  char *headersStart = lineEnd + 1;
  size_t headersLength = headersEnd - headersStart;
  *headers = (char *)malloc((headersLength + 1) * sizeof(char));
  snprintf(*headers, headersLength + 1, "%s", headersStart);

  size_t bodyLength = strlen(headersEnd + 4);
  *body = (char *)malloc((bodyLength + 1) * sizeof(char));
  snprintf(*body, bodyLength + 1, "%s", headersEnd + 4);

  return 0;
}

#ifdef __cplusplus
}
#endif

#endif
