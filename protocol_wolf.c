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
#include "network_message.h"
#include "protocol_wolf.h"

buffer_t *gMessageBuffer = NULL;
gameState *gGameState = NULL;
fd_set gPlayerSet;
fd_set gReadySet;

void gamePendingPlayerReady()
{
}

void gameUpdatePending(int pSocket, fd_set *pSocketSet, int pMaxSocket, char *pMessage)
{
  UNUSED(pSocketSet);
  if (isCommand("ready", pMessage)) {
    FD_SET(pSocket, &gReadySet);
    infof("Player %d is ready.", pSocket);
  }
  else {
    FD_CLR(pSocket, &gReadySet);
    infof("Player %d is not ready.", pSocket);
  }
  if (1 < gGameState->playerCount && 0 == memcmp(&gPlayerSet, &gReadySet, sizeof(fd_set))) {
    gGameState->status = GAME_STARTED;
    networkMessageAll(&gPlayerSet, pMaxSocket, "game start");
    info("Game started.");
  }
}

void gameUpdate(int pSocket, fd_set *pSocketSet, int pMaxSocket, char *pMessage)
{
  // switch based on game mode
  switch (gGameState->status) {
    case GAME_PENDING:
      gameUpdatePending(pSocket, pSocketSet, pMaxSocket, pMessage);
    break;
    case GAME_STARTED:
    break;
    case GAME_OVER:
    break;
    case GAME_NONE:
    default:
      error("Bad game status.");
    break;
  }
}

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

bool protocolInit(int pSocket, fd_set *pSocketSet, int pMaxSocket, int pBufferSize)
{
  UNUSED(pSocket);
  UNUSED(pSocketSet);
  UNUSED(pMaxSocket);

  FD_ZERO(&gPlayerSet);
  FD_ZERO(&gReadySet);

  bool success = true;
  success = success && bufferAllocate(&gMessageBuffer, pBufferSize);
  success = success && gameStateInit(&gGameState, 0, pMaxSocket);
  gGameState->status = GAME_PENDING;
  return success;
}

bool protocolConnect(int pSocket, fd_set *pSocketSet, int pMaxSocket, int pBufferSize)
{
  UNUSED(pSocketSet);
  UNUSED(pMaxSocket);
  UNUSED(pBufferSize);

  infof("New connection on socket %d.\n", pSocket);
  FD_SET(pSocket, &gPlayerSet);
  gGameState->playerCount++;
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
    gameUpdate(pSocket, pSocketSet, pMaxSocket, gMessageBuffer->buffer);
  }
  else if (0 == messageSize)
  {
    close(pSocket);
    FD_CLR(pSocket, pSocketSet);
    FD_CLR(pSocket, &gPlayerSet);
    FD_CLR(pSocket, &gReadySet);
    gGameState->playerCount--;
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
  success = success && bufferFree(&gMessageBuffer);
  return success;
}

