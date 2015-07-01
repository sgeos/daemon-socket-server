#ifndef PROTOCOL_WOLF_H
#define PROTOCOL_WOLF_H

#include <stdbool.h>
#include <sys/select.h>

typedef enum gameStatus_t {
  GAME_NONE = 0,
  GAME_PENDING = 1,
  GAME_STARTED = 2,
  GAME_OVER = 3
} gameStatus;

typedef enum playerType_t {
  PLAYER_HUMAN = 0,
  PLAYER_WEREWOLF = 1
} playerType;

typedef enum playerStatus_t {
  PLAYER_NONE = 0,
  PLAYER_THINKING = 1,
  PLAYER_MOVED = 2,
  PLAYER_DEAD = 3
} playerStatus;

typedef struct gamePlayer_t {
  int id;
  playerType type;
  playerStatus status;
  int vote;
  int kill;
} gamePlayer;

typedef struct gameState_t {
  gameStatus status;
  int playerMax;
  int playerCount;
  int humansAlive;
  int wolvesfAlive;
  gamePlayer **player;
} gameState;

fd_set playerSet;

#endif // PROTOCOL_WOLF_H

