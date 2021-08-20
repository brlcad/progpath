
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


extern char progpath_ipwd[];


void progpath_init(void) {
  if (progpath_ipwd[0] != '\0')
    return;

  progipwd(progpath_ipwd, MAXPATHLEN);
}


struct progpath_initializer {
  /* constructor */
  progpath_initializer() {
    progpath_init();
  }
  /* destructor */
  ~progpath_initializer() {
  }
};

static progpath_initializer pp;
