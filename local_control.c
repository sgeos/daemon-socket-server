#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
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

bool localControl(int pSocket, fd_set *pSocketSet, int pMaxSocket, int pBufferSize)
{
  UNUSED(pSocket);
  UNUSED(pSocketSet);
  UNUSED(pMaxSocket);

  bool done = false;
  char messageBuffer[pBufferSize];
  memset(&messageBuffer, 0, pBufferSize * sizeof(char));

  fgets(messageBuffer, pBufferSize * sizeof(char), stdin);
  stripNewlines(messageBuffer, strlen(messageBuffer));
  if (isCommand("quit", messageBuffer) || isCommand("exit", messageBuffer) || isCommand("q", messageBuffer)) {
    noticef("Exiting program with local control command \"%s\".", messageBuffer);
    done = true;
  }
  else {
    noticef("Unknown local control command \"%s\".", messageBuffer);
  }
  return done;
}

