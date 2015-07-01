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
#include "protocol_wolf.h"

char *gMessageBuffer = NULL;
int gBufferSize = 0;

gameState *gGameState = NULL;
fd_set gPlayerSet;

bool gameStateInit(gameState **pGameState, int pPlayerCount, int pPlayerMax)
{
  bool success = true;
  if (NULL == *pGameState) {
    *pGameState = calloc(1, sizeof(gameState));
    if (NULL == *pGameState) {
      error("Failed to allocate game state.");
      success = false;
    }
    else {
      // running into a malloc erro
      (*pGameState)->player = NULL;
      (*pGameState)->playerCount = 0;
      (*pGameState)->playerMax = 0;
      info("Allocated game state.");
    }
  }
  else {
    info("Game state already exists.");
  }
  if (success && NULL == (*pGameState)->player) {
    (*pGameState)->player = calloc(pPlayerMax, sizeof(gamePlayer *));
    if (NULL == (*pGameState)->player) {
      errorf("Failed to allocate array for %d player(s).", pPlayerMax);
      (*pGameState)->playerCount = 0;
      (*pGameState)->playerMax = 0;
      success = false;
    }
    else {
      infof("Allocate array for %d player(s).", pPlayerMax);
      (*pGameState)->playerCount = pPlayerCount;
      (*pGameState)->playerMax = pPlayerMax;
    }
  }
  else {
    info("Player data array already exists.");
  }
  return success;
}

bool gameStateCleanup(gameState **pGameState)
{
  for (int i = 0; i < (*pGameState)->playerMax; i++) {
    if (NULL != (*pGameState)->player[i]) {
      free((*pGameState)->player[i]);
      (*pGameState)->playerCount--;
    }
  }
  if (0 < (*pGameState)->playerMax) {
    info("Released memory for individual player data.");
  }
  else {
    info("No memory to release for individual player data.");
  }
  if (NULL != (*pGameState)->player) {
    free((*pGameState)->player);
    (*pGameState)->player = NULL;
    (*pGameState)->playerCount = 0;
    (*pGameState)->playerMax = 0;
  }
  info("Released memory for player data array.");
  if (NULL != *pGameState) {
    free(*pGameState);
    *pGameState = NULL;
  }
  info("Released memory for root game state.");
  bool success = true;
  return success;
}

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
  bool success = true;
  success = success && allocateBuffer(pBufferSize);
  success = success && gameStateInit(&gGameState, 0, pMaxSocket);
  return success;
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
  bool success = true;
  success = success && gameStateCleanup(&gGameState);
  success = success && freeBuffer();
  return success;
}

