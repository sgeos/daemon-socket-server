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

bool socketProtocol(int pSocket, fd_set *pSocketSet, int pMaxSocket, int pBufferSize)
{
  UNUSED(pMaxSocket);

  infof("Communication on socket %d.\n", pSocket);

  // send(), recv(), close()
  // Call FD_CLR(pSocket, pSocketSet) on disconnection
  bool success = true;
  int messageSize;
  char messageBuffer[pBufferSize];
  int flags = 0;

  // Receive a message from client
  memset(&messageBuffer, 0, pBufferSize * sizeof(char));
  messageSize = recv(pSocket, messageBuffer, pBufferSize, flags);
  if (0 < messageSize)
  {
    // Return the message to the sender
    send(pSocket, messageBuffer, strlen(messageBuffer), flags);
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

