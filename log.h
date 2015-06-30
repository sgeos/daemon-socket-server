#ifndef LOG_H
#define LOG_H

#include <syslog.h>

extern const char *logLevelString[];

#ifdef DEBUG
  #define log(priority, format, ...) \
    syslog(priority, "%s:%s:%d:%s:" format, logLevelString[priority], __FILE__, __LINE__, __func__, __VA_ARGS__)
#else // DEBUG
  #define log(priority, format, ...) syslog(priority, format, __VA_ARGS__)
#endif // DEBUG

// zero argument variadic macros do not work out of the box and the hacks suck
// MACRO() is for simple logging and MACROf() is for formatted logging
#define emergencyf(format, ...) syslog(LOG_EMERG, format, __VA_ARGS__)
#define alertf(format, ...) syslog(LOG_ALERT, format, __VA_ARGS__)
#define criticalf(format, ...) syslog(LOG_CRIT, format, __VA_ARGS__)
#define errorf(format, ...) syslog(LOG_ERR, format, __VA_ARGS__)
#define warningf(format, ...) syslog(LOG_WARNING, format, __VA_ARGS__)
#define noticef(format, ...) syslog(LOG_NOTICE, format, __VA_ARGS__)
#define infof(format, ...) syslog(LOG_INFO, format, __VA_ARGS__)
#define debugf(format, ...) syslog(LOG_DEBUG, format, __VA_ARGS__)

#define emergency(format) syslog(LOG_EMERG, format)
#define alert(format) syslog(LOG_ALERT, format)
#define critical(format) syslog(LOG_CRIT, format)
#define error(format) syslog(LOG_ERR, format)
#define warning(format) syslog(LOG_WARNING, format)
#define notice(format) syslog(LOG_NOTICE, format)
#define info(format) syslog(LOG_INFO, format)
#define debug(format) syslog(LOG_DEBUG, format)

void initLog(char *pLogName);
void stopLog();

#endif // LOG_H

