#ifndef NETWORK_MESSAGE_H
#define NETWORK_MESSAGE_H

#include <stdbool.h>
#include <sys/select.h>

void networkMessageExemptSet(fd_set *pSocketSet);
bool networkMessagePrivate(int pSocket, const char *pMessage);
bool networkMessagePublic(int pSocket, fd_set *pSocketSet, int pMaxSocket, const char *pMessage);
bool networkMessageAll(fd_set *pSocketSet, int pMaxSocket, const char *pMessage);

#endif // NETWORK_MESSAGE_H

