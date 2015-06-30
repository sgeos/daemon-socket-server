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

bool isCommand(const char *pCommand, const char *pBuffer)
{
  bool success = 0 == strncmp(pBuffer, pCommand, strlen(pCommand));
  return success;
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
  if (isCommand("quit", messageBuffer) || isCommand("exit", messageBuffer) || isCommand("q", messageBuffer)) {
    done = true;
  }
  return done;
}

