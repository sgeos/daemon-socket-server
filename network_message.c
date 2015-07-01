#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include "log.h"
#include "network_message.h"

fd_set gExemptSet;

void networkMessageExemptSet(fd_set *pSocketSet)
{
  FD_COPY(pSocketSet, &gExemptSet);
}

bool networkMessagePrivate(int pSocket, const char *pMessage)
{
  // Send the message to a single party
  int flags = 0;
  bool success = -1 != send(pSocket, pMessage, strlen(pMessage), flags);
  if (!success) {
    warningf("Could not send message \"%s\" to socket %d.", pMessage, pSocket);
  }
  return success;
}

bool networkMessagePublic(int pSocket, fd_set *pSocketSet, int pMaxSocket, const char *pMessage)
{
  // Relay the message to all other clients
  bool success = true;
  for (int s = 0; s <= pMaxSocket; s++) {
    if (s != pSocket && FD_ISSET(s, pSocketSet) && !FD_ISSET(s, &gExemptSet)) {
      success = networkMessagePrivate(s, pMessage) && success;
    }
  }
  return success;
}

bool networkMessageAll(fd_set *pSocketSet, int pMaxSocket, const char *pMessage)
{
  // Relay the message to all clients
  bool success = true;
  for (int s = 0; s <= pMaxSocket; s++) {
    if (FD_ISSET(s, pSocketSet) && !FD_ISSET(s, &gExemptSet)) {
      success = networkMessagePrivate(s, pMessage) && success;
    }
  }
  return success;
}

