// Reference:
// http://www.martinbroadhurst.com/source/select-server.c.html

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include "utility.h"
#include "args.h"
#include "log.h"
#include "network_message.h"

const char *PORT = "32001";
const int BACKLOG = 10;
const int BUFFER_SIZE = 2000;
const bool INTERACTIVE = true;

bool localControl(int pSocket, fd_set *pSocketSet, int pMaxSocket, int pBufferSize);
bool protocolInit(int pSocket, fd_set *pSocketSet, int pMaxSocket, int pBufferSize);
bool protocolConnect(int pSocket, fd_set *pSocketSet, int pMaxSocket, int pBufferSize);
bool protocolUpdate(int pSocket, fd_set *pSocketSet, int pMaxSocket, int pBufferSize);
bool protocolCleanup(int pSocket, fd_set *pSocketSet, int pMaxSocket, int pBufferSize);

bool getAddressInfo(const char *pPort, struct addrinfo **pAddressInfo)
{
  struct addrinfo *hints = calloc(1, sizeof(struct addrinfo));
  hints->ai_family = AF_UNSPEC;
  hints->ai_socktype = SOCK_STREAM;
  hints->ai_flags = AI_PASSIVE;
  bool success = 0 == getaddrinfo(NULL, pPort, hints, pAddressInfo);
  free(hints);
  noticef("Port set to %s.", pPort);

  if (!success) {
    error("Failed to get address info.");
  }
  return success;
}

bool createSocket(struct addrinfo *pAddressInfo, int *pSocket)
{
  *pSocket = socket(pAddressInfo->ai_family, pAddressInfo->ai_socktype, pAddressInfo->ai_protocol);
  bool success = 0 <= *pSocket;
  if (!success) {
   error("Failed to create socket");
  }
  return success;
}

bool socketAddressReuse(int pSocket, int pReuseAddress)
{
  bool success = 0 <= setsockopt(pSocket, SOL_SOCKET, SO_REUSEADDR, &pReuseAddress, sizeof(int));
  if (!success) {
    error("Failed to set reuse socket address option.");
  }
  return success;
}

bool socketAddressBind(int pSocket, struct addrinfo *pAddressInfo)
{
  bool success = 0 <= bind(pSocket, pAddressInfo->ai_addr, pAddressInfo->ai_addrlen);

  char ip4[INET_ADDRSTRLEN];  // space to hold the IPv4 string
  struct sockaddr_in sa = *(struct sockaddr_in *)(pAddressInfo->ai_addr);
  inet_ntop(AF_INET, &(sa.sin_addr), ip4, INET_ADDRSTRLEN);
  noticef("Bound to address %s\n", ip4);

  if (!success) {
    error("Failed to bind.");
  }
  return success;
}

bool socketListen(int pSocket, int pBacklog)
{
  bool success = 0 <= listen(pSocket, pBacklog);
  infof("Socket connection backlog set to %d.", pBacklog);
  if (!success) {
    error("Failed to listen");
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
    FD_SET(STDOUT_FILENO, pSocketSet);
    info("Local control commands enabled.");
  }
  else {
    info("Local control commands disabled.");
  }
  networkMessageExemptSet(pSocketSet);
  bool success = true;
  return success;
}

bool socketSelect(int pMaxSocket, fd_set *pSocketSet)
{
  bool success = 0 <= select(pMaxSocket + 1, pSocketSet, NULL, NULL, NULL);
  if (!success) {
    errorf("Failed to read from socket using select. %s.", strerror(errno));
  }
  return success;
}

// FIXME: this is a mess because it is calling protocolConnect() at the end
bool socketConnectionNew(int pSocket, int *pMaxSocket, fd_set *pSocketSet, int pBufferSize)
{
  bool success;
  int newSocket;
  struct sockaddr_in incomingAddress;
  socklen_t size = sizeof(struct sockaddr_in);
  newSocket = accept(pSocket, (struct sockaddr*)&incomingAddress, &size);
  if (newSocket < 0) {
    error("Failed to accept new socket connection");
    success = false;
  }
  else {
    noticef("New connection from %s on port %d.\n", 
      inet_ntoa(incomingAddress.sin_addr), htons(incomingAddress.sin_port));
    FD_SET(newSocket, pSocketSet);
    if (*pMaxSocket < newSocket) {
      *pMaxSocket = newSocket;
    }
    success = true;
  }
  return success && protocolConnect(newSocket, pSocketSet, *pMaxSocket, pBufferSize); // failure is not terminal
}

bool mainLoop(int argc, char **argv)
{
  const char *port = PORT;
  int backlog = BACKLOG;
  int bufferSize = BUFFER_SIZE;
  bool interactive = INTERACTIVE;
  args_param_t args_param_list[] =
  {
    {"-p",            &port,        argsString },
    {"--port",        &port,        argsString },
    {"-b",            &backlog,     argsInteger },
    {"--backlog",     &backlog,     argsInteger },
    {"-B",            &bufferSize,  argsInteger },
    {"--buffer",      &bufferSize,  argsInteger },
    {"-i",            &interactive, argsBoolTrue },
    {"--interactive", &interactive, argsBoolTrue },
    {"-d",            &interactive, argsBoolFalse },
    {"--daemon",      &interactive, argsBoolFalse },
    ARGS_DONE
  };
  argsProcess(argc, argv, args_param_list);

  int socket = -1;
  fd_set socketSet;

  bool success = true;
  success = success && socketOpen(port, backlog, &socket);
  success = success && socketSetInitialize(socket, &socketSet, interactive);
  if (success) {
    infof("Message buffer size set to %d.", bufferSize);
  }

  // main loop
  int maxSocket = socket;
  success = success && protocolInit(socket, &socketSet, maxSocket, bufferSize);
  bool done = !success; // skip loop on error
  while (!done) {
    fd_set readSocketSet;
    FD_COPY(&socketSet, &readSocketSet);
    success = success && socketSelect(maxSocket, &readSocketSet);
    for (int s = 0; !done && s <= maxSocket; s++) {
      if (FD_ISSET(s, &readSocketSet)) {
        if (s == socket) {
          socketConnectionNew(s, &maxSocket, &socketSet, bufferSize); // failure is not terminal
        }
        else if (STDIN_FILENO == s) {
          // system control
          done = localControl(s, &socketSet, maxSocket, bufferSize);
        }
        else {
          // existing connection
          protocolUpdate(s, &socketSet, maxSocket, bufferSize); // failure is not terminal
        }
      }
    }
    done = done || !success;
  }
  protocolCleanup(socket, &socketSet, maxSocket, bufferSize); // always

  close(socket);

  return done;
}

