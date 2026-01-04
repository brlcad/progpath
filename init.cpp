/*                        I N I T . C P P
 * progpath
 *
 * Copyright (c) 2021-2026 Christopher Sean Morrison
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
