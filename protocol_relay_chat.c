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

char *gMessageBuffer = NULL;
int gBufferSize = 0;

bool freeBuffer(void) {
  bool success;
  if (NULL != gMessageBuffer) {
    free(gMessageBuffer);
    infof("Freed message buffer of size %d.", gBufferSize);
    gBufferSize = 0;
    success = true;
  }
  else {
    warning("Failed to free message buffer.");
    success = false;
  }
  return success;
}

bool allocateBuffer(int pBufferSize)
{
  if (NULL != gMessageBuffer) {
    freeBuffer();
  }

  bool success;
  gMessageBuffer = calloc(pBufferSize, sizeof(char));
  if (NULL == gMessageBuffer) {
    errorf("Failed to allocate buffer of size %d.", pBufferSize);
    gBufferSize = 0;
    success = false;
  }
  else {
    gBufferSize = pBufferSize;
    infof("Allocated buffer of size %d.", pBufferSize);
    success = true;
  }
  return success;
}

bool protocolInit(int pSocket, fd_set *pSocketSet, int pMaxSocket, int pBufferSize)
{
  UNUSED(pSocket);
  UNUSED(pSocketSet);
  UNUSED(pMaxSocket);
  return allocateBuffer(pBufferSize);
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
  if (gBufferSize < pBufferSize) {
    freeBuffer();
    allocateBuffer(pBufferSize);
  }

  infof("Communication on socket %d.\n", pSocket);

  // send(), recv(), close()
  // Call FD_CLR(pSocket, pSocketSet) on disconnection
  bool success = true;
  int messageSize;
  int flags = 0;

  // Receive a message from client
  memset(gMessageBuffer, 0, gBufferSize * sizeof(char));
  messageSize = recv(pSocket, gMessageBuffer, gBufferSize, flags);
  if (0 < messageSize)
  {
    for (int s = 0; s <= pMaxSocket; s++) {
      // Relay the message to all other clients
      if (s != pSocket && FD_ISSET(s, pSocketSet)) {
        send(s, gMessageBuffer, strlen(gMessageBuffer), flags);
      }
    }
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
  return freeBuffer();
}

