#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

bool socketProtocol(int pSocket, fd_set *pSocketSet, int pMaxSocket, int pBufferSize)
{
  printf("Communication on socket %d.\n", pSocket);

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
    for (int s = 0; s <= pMaxSocket; s++) {
      if (s != pSocket && FD_ISSET(s, pSocketSet)) {
        // Relay the message to all other clients
        send(s, messageBuffer, strlen(messageBuffer), flags);
      }
    }
  }
  else if (0 == messageSize)
  {
    close(pSocket);
    FD_CLR(pSocket, pSocketSet);
    printf("Client disconnected socket %d.\n", pSocket);
    fflush(stdout);
  }
  else if (messageSize < 0)
  {
    perror("recv failed");
    success = false;
  }
  return success;
}

