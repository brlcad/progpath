
#include "./progpath_config.h"
#include "./progpath.h"

/* use any method at our disposal */
#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef HAVE_SYS_PARAM_H /* for MAXPATHLEN */
#  include <sys/param.h>
#endif
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#ifdef HAVE_WINDOWS_H
#  include <windows.h>
#endif


#ifndef MAXPATHLEN
#  ifdef PATH_MAX
#    define MAXPATHLEN PATH_MAX
#  else
#    define MAXPATHLEN 4096
#  endif
#endif


/* need to store/access the initial path, but not as API */
extern const char *progpath_icwd;
char icwd[MAXPATHLEN] = {0};


void progpath_init(void) {

  progpath_icwd = icwd;
  if (progpath_icwd[0] != '\0')
    return;


#ifdef HAVE_GETCWD
  {
    getcwd(icwd, MAXPATHLEN);
    if (progpath_icwd[0])
      return;
  }
#endif


#ifdef HAVE_REALPATH
  {
    realpath(".", icwd);
    if (progpath_icwd[0])
      return;
  }
#endif


  {
    char *pwd;
    pwd = getenv("PWD");
    if (pwd)
      strncpy(icwd, pwd, MAXPATHLEN-1);
    if (progpath_icwd[0])
      return;
  }

  printf("ERROR: Unable to determine initial working directory\n");
  assert(progpath_icwd && progpath_icwd[0]);
}


struct progpath_initializer {
  /* constructor */
  progpath_initializer() {
    char path[MAXPATHLEN] = {0};
    progpath_init();
    progpath(path, (size_t)MAXPATHLEN);
  }
  /* destructor */
  ~progpath_initializer() {
  }
};

static progpath_initializer pp;