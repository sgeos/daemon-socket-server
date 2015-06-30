// Reference:
// http://www.martinbroadhurst.com/source/select-server.c.html

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

const char *PORT = "32001";
const int BACKLOG = 10;
const int BUFFER_SIZE = 2000;
const bool INTERACTIVE = true;

bool localControl(int pSocket, fd_set *pSocketSet, int pMaxSocket, int pBufferSize);
bool socketProtocol(int pSocket, fd_set *pSocketSet, int pMaxSocket, int pBufferSize);

bool getAddressInfo(const char *pPort, struct addrinfo **pAddressInfo)
{
  struct addrinfo *hints = calloc(1, sizeof(struct addrinfo));
  hints->ai_family = AF_UNSPEC;
  hints->ai_socktype = SOCK_STREAM;
  hints->ai_flags = AI_PASSIVE;
  bool success = 0 == getaddrinfo(NULL, pPort, hints, pAddressInfo);
  free(hints);

  if (!success) {
    perror("getaddrinfo");
  }
  return success;
}

bool createSocket(struct addrinfo *pAddressInfo, int *pSocket)
{
  *pSocket = socket(pAddressInfo->ai_family, pAddressInfo->ai_socktype, pAddressInfo->ai_protocol);
  bool success = 0 <= *pSocket;
  if (!success) {
    perror("socket");
  }
  return success;
}

bool socketAddressReuse(int pSocket, int pReuseAddress)
{
  bool success = 0 <= setsockopt(pSocket, SOL_SOCKET, SO_REUSEADDR, &pReuseAddress, sizeof(int));
  if (!success) {
    perror("setsockopt");
  }
  return success;
}

bool socketAddressBind(int pSocket, struct addrinfo *pAddressInfo)
{
  bool success = 0 <= bind(pSocket, pAddressInfo->ai_addr, pAddressInfo->ai_addrlen);

  char ip4[INET_ADDRSTRLEN];  // space to hold the IPv4 string
  struct sockaddr_in sa = *(struct sockaddr_in *)(pAddressInfo->ai_addr);
  inet_ntop(AF_INET, &(sa.sin_addr), ip4, INET_ADDRSTRLEN);
  printf("bind to address %s\n", ip4);

  if (!success) {
    perror("bind");
  }
  return success;
}

bool socketListen(int pSocket, int pBacklog)
{
  bool success = 0 <= listen(pSocket, pBacklog);
  if (!success) {
    perror("listen");
  }
  return success;
}

bool socketOpen(const char *pPort, int pBacklog, int *pSocket)
{
  struct addrinfo *addressInfo = NULL;

  bool success = true;
  success = success && getAddressInfo(pPort, &addressInfo);
  success = success && createSocket(addressInfo, pSocket);
  success = success && socketAddressReuse(*pSocket, true);
  success = success && socketAddressBind(*pSocket, addressInfo);
  freeaddrinfo(addressInfo);
  success = success && socketListen(*pSocket, pBacklog);
  return success;
}

bool socketSetInitialize(int pSocket, fd_set *pSocketSet, bool pInteractive)
{
  FD_ZERO(pSocketSet);
  FD_SET(pSocket, pSocketSet);
  if (pInteractive) {
    FD_SET(STDIN_FILENO, pSocketSet);
  }
  bool success = true;
  return success;
}

bool socketSelect(int pMaxSocket, fd_set *pSocketSet)
{
  bool success = 0 <= select(pMaxSocket + 1, pSocketSet, NULL, NULL, NULL);
  if (!success) {
    perror("select");
  }
  return success;
}

bool socketConnectionNew(int pSocket, int *pMaxSocket, fd_set *pSocketSet)
{
  bool success;
  int newSocket;
  struct sockaddr_in incomingAddress;
  socklen_t size = sizeof(struct sockaddr_in);
  newSocket = accept(pSocket, (struct sockaddr*)&incomingAddress, &size);
  if (newSocket < 0) {
    perror("accept");
    success = false;
  }
  else {
    printf("New connection from %s on port %d.\n", 
      inet_ntoa(incomingAddress.sin_addr), htons(incomingAddress.sin_port));
    FD_SET(newSocket, pSocketSet);
    if (*pMaxSocket < newSocket) {
      *pMaxSocket = newSocket;
    }
    success = true;
  }
  return success;
}

bool mainLoop(int argc, char **argv)
{
  UNUSED(argc);
  UNUSED(argv);
  const char *port = PORT;
  int backlog = BACKLOG;
  bool interactive = INTERACTIVE;
  int socket = -1;
  fd_set socketSet;

  bool success = true;
  success = success && socketOpen(port, backlog, &socket);
  success = success && socketSetInitialize(socket, &socketSet, interactive);

  // main loop
  int maxSocket = socket;
  bool done = !success; // skip loop on error
  while (!done) {
    fd_set readSocketSet = socketSet;
    success = success && socketSelect(maxSocket, &readSocketSet);
    for (int s = 0; !done && s <= maxSocket; s++) {
      if (FD_ISSET(s, &readSocketSet)) {
        if (s == socket) {
          socketConnectionNew(socket, &maxSocket, &socketSet); // failure is not terminal
        }
        else if (STDIN_FILENO == s) {
          // system control
          int bufferSize = BUFFER_SIZE;
          done = localControl(s, &socketSet, maxSocket, bufferSize);
        }
        else {
          // existing connection
          int bufferSize = BUFFER_SIZE;
          socketProtocol(s, &socketSet, maxSocket, bufferSize); // failure is not terminal
        }
      }
    }
    done = done || !success;
  }

  close(socket);

  return done;
}

