#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>

const char SAFE_DIR[] = "/";
const char DEV_NULL_DIR[] = "/dev/null";
const int MAIN_LOOP_DELAY = 20;

bool mainLoop(int argc, char **argv);

void forkChildAndExit()
{
  pid_t pid = fork();

  if (pid < 0) {
    fprintf(stderr,"Error: Fork failed.\n");
    exit(EXIT_FAILURE);
  } else if (0 < pid) {
    exit(EXIT_SUCCESS);
  }
}

void becomeSessionLeader()
{
  if (setsid() < 0) {
    fprintf(stderr,"Error: Failed to become session leader.\n");
    exit(EXIT_FAILURE);
  }
}

// TODO: Implement a working signal handler
void initSignals()
{
  signal(SIGCHLD, SIG_IGN);
  signal(SIGHUP, SIG_IGN);
}

void setFilePermissions()
{
  umask(0);
}

void moveToSafeDirectory(const char *pSafeDir)
{
  if (chdir(pSafeDir) < 0) {
    syslog(LOG_ERR, "Could not change working directory to: %s\n", pSafeDir);
    exit(EXIT_FAILURE);
  }
}

void closeAllOpenFileDescrtiptors()
{
  for (int x = sysconf(_SC_OPEN_MAX); 0 < x; x--)
  {
    close(x);
  }
  stdin = fopen(DEV_NULL_DIR, "r");
  stdout = fopen(DEV_NULL_DIR, "w+");
  stderr = fopen(DEV_NULL_DIR, "w+");
}

void initSyslog(char *pLogName)
{
  openlog(pLogName, LOG_PID, LOG_DAEMON); 
  syslog(LOG_NOTICE, "Daemon started.\n"); 
}

void stopSyslog()
{
  closelog();
}

static void initDaemon(char *pLogName)
{
  forkChildAndExit();
  becomeSessionLeader();
  initSignals();
  forkChildAndExit(); // guarantee daemon is detached from a terminal permanently
  setFilePermissions();
  closeAllOpenFileDescrtiptors();
  initSyslog(pLogName);
  moveToSafeDirectory(SAFE_DIR);
}

static void stopDaemon()
{
  syslog(LOG_NOTICE, "Daemon terminated.");
  stopSyslog();
}

int main(int argc, char **argv)
{
  initDaemon(argv[0]);

  syslog(LOG_NOTICE, "Daemon running.");
  bool done = false;
  while (!done)
  {
    done = mainLoop(argc, argv);
    sleep(MAIN_LOOP_DELAY);
  }

  stopDaemon();
  return EXIT_SUCCESS;
}

