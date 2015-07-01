#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "utility.h"
#include "log.h"

bool isCommand(const char *pCommand, const char *pBuffer)
{
  bool success = 0 == strncmp(pBuffer, pCommand, strlen(pCommand));
  return success;
}

void stripNewlines(char *pBuffer, int pLength)
{
  for (int i = pLength; 0 <= i; i--) {
    if ('\0' == pBuffer[i] || '\n' == pBuffer[i]) {
      pBuffer[i] = '\0';
    }
    else {
      return;
    }
  }
}

bool bufferFreeInner(buffer_t **pBuffer)
{
  bool success;
  if (NULL != *pBuffer) {
    if (NULL != (*pBuffer)->buffer) {
      free((*pBuffer)->buffer);
      infof("Freed inner buffer of size %d.", (*pBuffer)->size);
      (*pBuffer)->size = 0;
      success = true;
    }
    else {
      warningf("No inner buffer of size %d to free.", (*pBuffer)->size);
      success = false;
    }
  }
  else {
    warningf("No inner buffer of size %d to free.", (*pBuffer)->size);
    success = false;
  }
  return success;
}

bool bufferAllocateInner(buffer_t **pBuffer, int pBufferSize)
{
  bool success;
  if (NULL == *pBuffer) {
    error("Can not allocat inner buffer in null struct.");
    success = false;
  }
  else {
    if (NULL != (*pBuffer)->buffer) {
      bufferFreeInner(pBuffer);
    }
    (*pBuffer)->buffer = calloc(pBufferSize, sizeof(char));
    if (NULL == (*pBuffer)->buffer) {
      errorf("Failed to allocate buffer of size %d.", pBufferSize);
      (*pBuffer)->size = 0;
      success = false;
    }
    else {
      infof("Allocated inner buffer of size %d.", pBufferSize);
      (*pBuffer)->size = pBufferSize;
      success = true;
    }
  }
  return success;
}

bool bufferClear(buffer_t **pBuffer)
{
  memset((*pBuffer)->buffer, 0, (*pBuffer)->size * sizeof(char));
  bool success = true;
  return success;
}

bool bufferAllocate(buffer_t **pBuffer, int pBufferSize)
{
  if (NULL != *pBuffer) {
    bufferFree(pBuffer);
  }

  bool success;
  *pBuffer = calloc(pBufferSize, sizeof(buffer_t));
  if (NULL == *pBuffer) {
    error("Failed to allocate buffer struct.");
    success = false;
  }
  else {
    success = bufferAllocateInner(pBuffer, pBufferSize);
  }
  return success;
}

bool bufferGrow(buffer_t **pBuffer, int pBufferSize)
{
  bool success = true;
  if ((*pBuffer)->size < pBufferSize) {
    success = success && bufferFreeInner(pBuffer);
    success = success && bufferAllocateInner(pBuffer, pBufferSize);
  }
  return success;
}

bool bufferFree(buffer_t **pBuffer)
{
  bool success;
  if (NULL != *pBuffer) {
    if (NULL != (*pBuffer)->buffer) {
      bufferFreeInner(pBuffer);
    }
    free(*pBuffer);
    info("Freed buffer struct.");
    success = true;
  }
  else {
    warning("No buffer struct to free.");
    success = false;
  }
  return success;
}

