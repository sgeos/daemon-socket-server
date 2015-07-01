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

buffer_t *gMessageBuffer = NULL;

bool protocolInit(int pSocket, fd_set *pSocketSet, int pMaxSocket, int pBufferSize)
{
  UNUSED(pSocket);
  UNUSED(pSocketSet);
  UNUSED(pMaxSocket);
  return bufferAllocate(&gMessageBuffer, pBufferSize);
}

bool protocolConnect(int pSocket, fd_set *pSocketSet, int pMaxSocket, int pBufferSize)
{
  UNUSED(pSocketSet);
  UNUSED(pMaxSocket);
  UNUSED(pBufferSize);

  infof("New connection on socket %d.\n", pSocket);
  bool success = true;
  return success;
}

bool protocolUpdate(int pSocket, fd_set *pSocketSet, int pMaxSocket, int pBufferSize)
{
  UNUSED(pMaxSocket);
  bufferGrow(&gMessageBuffer, pBufferSize);
  bufferClear(&gMessageBuffer);

  infof("Communication on socket %d.\n", pSocket);

  // send(), recv(), close()
  // Call FD_CLR(pSocket, pSocketSet) on disconnection
  bool success = true;
  int messageSize;
  int flags = 0;

  // Receive a message from client
  messageSize = recv(pSocket, gMessageBuffer->buffer, gMessageBuffer->size, flags);
  if (0 < messageSize)
  {
    // Return the message to the sender
    send(pSocket, gMessageBuffer->buffer, strlen(gMessageBuffer->buffer), flags);
  }
  else if (0 == messageSize)
  {
    close(pSocket);
    FD_CLR(pSocket, pSocketSet);
    noticef("Client disconnected from socket %d.\n", pSocket);
    fflush(stdout);
  }
  else if (messageSize < 0)
  {
    warningf("Failed to receive data from socket %d.", pSocket);
    success = false;
  }
  return success;
}

bool protocolCleanup(int pSocket, fd_set *pSocketSet, int pMaxSocket, int pBufferSize)
{
  UNUSED(pSocket);
  UNUSED(pSocketSet);
  UNUSED(pMaxSocket);
  UNUSED(pBufferSize);
  return bufferFree(&gMessageBuffer);
}

