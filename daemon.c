#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "args.h"
#include "log.h"
#include "utility.h"

const int SLEEP_DELAY = 0;
const char SAFE_DIR[] = "/";
const char DEV_NULL_DIR[] = "/dev/null";

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
    errorf("Could not change working directory to: %s\n", pSafeDir);
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

static void init(bool pDaemon, char *pLogName, char *pSafeDir)
{
  if (pDaemon) {
    forkChildAndExit();
    becomeSessionLeader();
    initSignals();
    forkChildAndExit(); // guarantee daemon is detached from a terminal permanently
    setFilePermissions();
    closeAllOpenFileDescrtiptors();
  }

  // always do the following
  initLog(pLogName);
  moveToSafeDirectory(SAFE_DIR);

  if (pDaemon) {
    notice("Started as daemon.");
  else {
    notice("Started as interactive program.");
  }
}

static void cleanup()
{
  notice("Program terminated.");
  stopLog();
}

int argsInteractive(int argc, char **argv, int argn, args_param_t *argsparam, void *data)
{
  // unused parameters
  UNUSED(argc); UNUSED(argv); UNUSED(argn); UNUSED(argsparam);

  bool *daemonFlag = (bool *)data;
  *daemonFlag = false;
  return 1;
}

int argsDaemon(int argc, char **argv, int argn, args_param_t *argsparam, void *data)
{
  // unused parameters
  UNUSED(argc); UNUSED(argv); UNUSED(argn); UNUSED(argsparam);

  bool *daemonFlag = (bool *)data;
  *daemonFlag = true;
  return 1;
}

int main(int argc, char **argv)
{
  bool daemon = true;
  char *logName = argv[0];
  char *safeDir = SAFE_DIR;
  int sleepDelay = SLEEP_DELAY;
  args_param_t args_param_list[] =
  {
    {"-i",            &daemon,     argsInteractive },
    {"--interactive", &daemon,     argsInteractive },
    {"-d",            &daemon,     argsDaemon },
    {"--daemon",      &daemon,     argsDaemon },
    {"-s",            &sleepDelay, argsInteger },
    {"--sleep",       &sleepDelay, argsInteger },
    {"-l",            &logName,    argsString },
    {"--log",         &logName,    argsString },
    {"--dir",         &safeDir,    argsString },
    ARGS_DONE
  };
  argsProcess(argc, argv, args_param_list);

  init(daemon, logName, safeDir);

  // run mainLoop() before calling sleep so that local control responds right away
  // if done is true after the first execution, the program will exit right away
  bool done = mainLoop(argc, argv);
  while (!done) {
    sleep(sleepDelay);
    done = mainLoop(argc, argv);
  }

  cleanup();
  return EXIT_SUCCESS;
}

