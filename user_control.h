#ifndef USER_CONTROL_H
#define USER_CONTROL_H

#include <stdbool.h>
#include <sys/select.h>

typedef struct userControl_t {
  fd_set userSet;
  fd_set readySet;
  int userCount;
  int maxUser;
  int requiredUsers;
} userControl_t;

void initUserControl_t(userControl_t *pUserControl, int pRequiredUsers);
void userAdd(userControl_t *pUserControl, int pUser);
void userRemove(userControl_t *pUserControl, int pUser);
void userReady(userControl_t *pUserControl, int pUser);
void userWait(userControl_t *pUserControl, int pUser);
bool userCountSufficient(userControl_t *pUserControl);
bool userAllReady(userControl_t *pUserControl);
bool userTurnComplete(userControl_t *pUserControl);
void userNewTurn(userControl_t *pUserControl);

#endif // USER_CONTROL_H

