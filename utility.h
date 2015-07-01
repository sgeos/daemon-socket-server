#ifndef UTILITY_H
#define UTILITY_H

#include <stdbool.h>

#define UNUSED(x) (void)(x)

typedef struct buffer_t {
  char *buffer;
  int size;
} buffer_t;

bool isCommand(const char *pCommand, const char *pBuffer);
void stripNewlines(char *pBuffer, int pLength);
bool bufferClear(buffer_t **pBuffer);
bool bufferAllocate(buffer_t **pBuffer, int pBufferSize);
bool bufferGrow(buffer_t **pBuffer, int pBufferSize);
bool bufferFree(buffer_t **pBuffer);

#endif // UTILITY_H

