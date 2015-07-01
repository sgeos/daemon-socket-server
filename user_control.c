#include <stdbool.h>
#include <string.h>
#include <sys/select.h>
#include "user_control.h"

void initUserControl_t(userControl_t *pUserControl, int pRequiredUsers)
{
  FD_ZERO(&(pUserControl->userSet));
  FD_ZERO(&(pUserControl->readySet));
  pUserControl->userCount = 0;
  pUserControl->maxUser = 0;
  pUserControl->requiredUsers = pRequiredUsers;
}

void userAdd(userControl_t *pUserControl, int pUser)
{
  if (!FD_ISSET(pUser, &(pUserControl->userSet))) {
    FD_SET(pUser, &(pUserControl->userSet));
    pUserControl->userCount++;
    if (pUserControl->maxUser < pUser) {
      pUserControl->maxUser = pUser;
    }
  }
}

void userRemove(userControl_t *pUserControl, int pUser)
{
  if (FD_ISSET(pUser, &(pUserControl->userSet))) {
    FD_CLR(pUser, &(pUserControl->userSet));
    FD_CLR(pUser, &(pUserControl->readySet));
    pUserControl->userCount--;
  }
}

void userReady(userControl_t *pUserControl, int pUser)
{
  FD_SET(pUser, &(pUserControl->readySet));
}

void userWait(userControl_t *pUserControl, int pUser)
{
  FD_CLR(pUser, &(pUserControl->readySet));
}

bool userCountSufficient(userControl_t *pUserControl)
{
  bool success = pUserControl->requiredUsers <= pUserControl->userCount;
  return success;
}

bool userAllReady(userControl_t *pUserControl)
{
  bool success = 0 == memcmp(&(pUserControl->userSet), &(pUserControl->readySet), sizeof(fd_set));
  return success;
}

bool userTurnComplete(userControl_t *pUserControl)
{
  bool success = userCountSufficient(pUserControl) && userAllReady(pUserControl);
  return success;
}

void userNewTurn(userControl_t *pUserControl)
{
  FD_ZERO(&(pUserControl->readySet));
}

