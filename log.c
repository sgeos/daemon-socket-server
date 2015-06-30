#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <syslog.h>
#include "log.h"

const char *logLevelString[] = {
  "EMERGENCY",
  "ALERT",
  "CRITICAL",
  "ERROR",
  "WARNING",
  "NOTICE",
  "INFO",
  "DEBUG",
};

void initLog(char *pLogName)
{
  openlog(pLogName, LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_DAEMON);
  setlogmask(LOG_UPTO(LOG_DEBUG));
  notice("Started logging.\n");
}

void stopLog()
{
  closelog();
}

