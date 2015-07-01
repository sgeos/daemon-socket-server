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
#include "user_control.h"
#include "protocol_wolf.h"

buffer_t *gMessageBuffer = NULL;
gameState *gGameState = NULL;

void gamePendingPlayerReady()
{
}

void gameUpdatePending(int pSocket, fd_set *pSocketSet, int pMaxSocket, char *pMessage)
{
  UNUSED(pSocketSet);
  if (isCommand("ready", pMessage)) {
    userReady(gGameState->userControl, pSocket);
    infof("Player %d is ready.", pSocket);
  }
  else {
    userWait(gGameState->userControl, pSocket);
    infof("Player %d is not ready.", pSocket);
  }
  if (userTurnComplete(gGameState->userControl)) {
    userNewTurn(gGameState->userControl);
    gGameState->status = GAME_STARTED;
    networkMessageAll(&(gGameState->userControl->userSet), pMaxSocket, "game start");
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

bool gameStateInit(gameState **pGameState, int pPlayerMax)
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
      info("Allocated game state.");
    }
  }
  else {
    info("Game state already exists.");
  }
  if (success && NULL == (*pGameState)->userControl) {
    (*pGameState)->userControl = calloc(1, sizeof(userControl_t));
    if (NULL == (*pGameState)->userControl) {
      error("Failed to allocate user control struct.");
      success = false;
    }
    else {
      initUserControl_t((*pGameState)->userControl, 2);
      info("Allocated user control struct.");
    }
  }
  if (success && NULL == (*pGameState)->player) {
    (*pGameState)->player = calloc(pPlayerMax, sizeof(gamePlayer *));
    if (NULL == (*pGameState)->player) {
      errorf("Failed to allocate array for %d player(s).", pPlayerMax);
      success = false;
    }
    else {
      infof("Allocate array for %d player(s).", pPlayerMax);
    }
  }
  else {
    info("Player data array already exists.");
  }
  return success;
}

bool gameStateCleanup(gameState **pGameState)
{
  for (int i = 0; i < (*pGameState)->userControl->userCount; i++) {
    if (NULL != (*pGameState)->player[i]) {
      free((*pGameState)->player[i]);
    }
  }
  if (0 < (*pGameState)->userControl->userCount) {
    info("Released memory for individual player data.");
  }
  else {
    info("No memory to release for individual player data.");
  }
  if (NULL != (*pGameState)->player) {
    free((*pGameState)->player);
    (*pGameState)->player = NULL;
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

  bool success = true;
  success = success && bufferAllocate(&gMessageBuffer, pBufferSize);
  success = success && gameStateInit(&gGameState, pMaxSocket);
  gGameState->status = GAME_PENDING;
  return success;
}

bool protocolConnect(int pSocket, fd_set *pSocketSet, int pMaxSocket, int pBufferSize)
{
  UNUSED(pSocketSet);
  UNUSED(pMaxSocket);
  UNUSED(pBufferSize);

  infof("New connection on socket %d.\n", pSocket);
  userAdd(gGameState->userControl, pSocket);
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
    userRemove(&(*gGameState->userControl), pSocket);
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

