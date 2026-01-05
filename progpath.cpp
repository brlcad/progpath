/*                      P R O G P A T H . C
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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#ifdef HAVE_SYS_AUXV_H
#  include <sys/auxv.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H /* for wait */
#  include <sys/wait.h>
#endif
#ifdef HAVE_SYS_SYSCTL_H
#  include <sys/sysctl.h>
#endif
#ifdef HAVE_SYS_PARAM_H /* for MAXPATHLEN */
#  include <sys/param.h>
#endif
#ifdef HAVE_SYS_PROCFS_H /* for psinfo */
#  include <sys/procfs.h>
#endif
#ifdef HAVE_SYS_IOCTL_H /* for ioctl */
#  include <sys/ioctl.h>
#endif
#ifdef HAVE_DLFCN_H /* for dladdr */
#  include <dlfcn.h>
#endif
#ifdef HAVE_FCNTL_H /* for open */
#  include <fcntl.h>
#endif
#ifdef HAVE_MACH_O_DYLD_H /* for dladdr */
#  include <mach-o/dyld.h>
#endif
#ifdef HAVE_PROCINFO_H
#  include <procinfo.h>
#endif
#ifdef HAVE_FINDDIRECTORY_H
#  include <FindDirectory.h>
#endif
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#ifdef HAVE_WINDOWS_H
#  include <windows.h>
#  define chdir _chdir
#endif

/* helper to simplify initialization */
#define METHOD(x) {0}; m.id = method++; m.line = __LINE__; m.label = (x); m.debug = debug;


extern "C" {
  extern const char *getprogname(void);
  extern const char *getexecname(void);
  extern void proc_pidpath(int, char *, size_t);
  extern int chdir(const char *);
}

#ifndef MAXPATHLEN
#  ifdef PATH_MAX
#    define MAXPATHLEN PATH_MAX
#  else
#    define MAXPATHLEN 4096
#  endif
#endif


char progpath_ipwd[MAXPATHLEN] = {0};


/* stateful structure used for debugging */
struct method {
  int id;
  int line;
  const char *label;
  int debug;
};


/* debug states encoded as bits */
enum {
  PP_DEFAULT = 0,     /* default: no debugging */
  PP_PRINT = 1<<0,    /* print debugging */
  PP_CONTINUE = 1<<1  /* try all methods */
};


/* PROGPATH_DEBUG=1 environment variable can be set in caller scope to
 * print useful debugging lines for methods that have results.
 */
static void print_method(struct method m, const char *result) {
  if (m.debug & PP_PRINT)
    printf("Method %02d, line %04d: %s=[%s]\n", m.id, m.line, m.label, result);
}


/* this function expands a given path, only modifying 'buf' with the
 * full path if it appears to have succeeded.
 */
static void resolve_to_full_path(char *buf, size_t buflen) {
  char rbuf[MAXPATHLEN] = {0};

  if (!buf || buflen < 1)
    return;

  /* work on a copy */
  strncpy(rbuf, buf, buflen-1);

  /* resolve links and relative paths */
  if (rbuf[0] == '/' || rbuf[0] == '.') {
#ifdef HAVE_REALPATH
    char rpbuf[MAXPATHLEN] = {0};
    realpath(rbuf, rpbuf);
    strncpy(rbuf, rpbuf, MAXPATHLEN);
#endif
  }

  /* resolve via PATH */

  /* copy full paths back to caller */
  if (rbuf[0] == '/')
    strncpy(buf, rbuf, buflen);

}


/* perform operations common to every method.  given a final output
 * buffer and an optional 'result' path, expand it to a full path,
 * printing debugging lines before and after expansion if enabled.
 * if 'result' is NULL, it will use the path in 'buf'.
 */
static void finalize(struct method m, char *buf, size_t buflen, const char *result) {
  if (!buf || buflen < 1)
    return;

  if (result)
    strncpy(buf, result, buflen-1);

  if (buf[0] == 0)
    return;

  print_method(m, buf);
  resolve_to_full_path(buf, MAXPATHLEN);
  print_method(m, buf);
}


/* cheeky function checks whather we seem to have a full 'path',
 * writing a full path to the 'buf' output buffer or dynamically
 * allocating if 'buf' is NULL.
 */
static int we_done_yet(struct method m, char **buf, size_t buflen, const char *path) {
  size_t pathlen;
  if (!path)
    return 0;

  pathlen = strlen(path);
  if (pathlen < 2)
    return 0;

  /* if we seem to have a full path, we're done */
  if ((path[0] == '/') ||
      (pathlen > 2 && path[0] >= 'A' && path[0] <= 'Z' && path[1] == ':' && path[2] == '\\')) {

    if (!buf || !(*buf) || !buflen) {
      buflen = MAXPATHLEN;
      *buf = (char *)calloc(buflen, sizeof(char));
      assert(buf && *buf && (*buf)[0] == '\0');
    }
    strncpy(*buf, path, buflen-1);

    if (m.debug & PP_CONTINUE)
      return 0;
    return 1;
  }

  return 0;
}


/* obtain the current working directory.  not currently exposed as
 * public API, but could be convinced to make it public if enough
 * people ask.
 */
static char *progcwd(char *buf, size_t buflen) {

  /* environment variable can be set for debug printing */
  const char *progpath_debug = getenv("PROGPATH_DEBUG");
  int debug = 0;
  int method = 0;

  if (progpath_debug)
    debug = atoi(progpath_debug);

  if (debug & PP_PRINT)
    printf("progcwd() getting the current directory\n");

#ifdef HAVE_GETCWD
  {
    char cwd[MAXPATHLEN] = {0};
    char mbuf[MAXPATHLEN] = {0};
    struct method m = METHOD("getcwd");
    getcwd(cwd, MAXPATHLEN);
    finalize(m, mbuf, MAXPATHLEN, cwd);
    if (we_done_yet(m, &buf, buflen, mbuf))
      return buf;
  }
#endif


#ifdef HAVE__GETCWD
  {
    char cwd[MAXPATHLEN] = {0};
    char mbuf[MAXPATHLEN] = {0};
    struct method m = METHOD("_getcwd");
    _getcwd(cwd, MAXPATHLEN);
    finalize(m, mbuf, MAXPATHLEN, cwd);
    if (we_done_yet(m, &buf, buflen, mbuf))
      return buf;
  }
#endif


#ifdef HAVE_REALPATH
  {
    char cwd[MAXPATHLEN] = {0};
    char mbuf[MAXPATHLEN] = {0};
    struct method m = METHOD("realpath(.)");
    realpath(".", cwd);
    finalize(m, mbuf, MAXPATHLEN, cwd);
    if (we_done_yet(m, &buf, buflen, mbuf))
      return buf;
  }
#endif


#ifdef HAVE_GETCURRENTDIRECTORY
  {
    char cwd[MAXPATHLEN] = {0};
    char mbuf[MAXPATHLEN] = {0};
    struct method m = METHOD("GetCurrentDirectory");
    GetCurrentDirectory(MAXPATHLEN, cwd);
    finalize(m, mbuf, MAXPATHLEN, cwd);
    if (we_done_yet(m, &buf, buflen, mbuf))
      return buf;
  }
#endif

  /* really just to appease the compiler */
  {
    struct method m = METHOD("UNKNOWN");
    if (we_done_yet(m, &buf, buflen, NULL))
      return buf;
  }

  if (buf && buf[0] != '\0')
    return buf;
  return NULL;
}


static void chdir_if_diff(const char *wd) {
  char cwd[MAXPATHLEN] = {0};
  int ret;

  if (!wd || wd[0] == '\0')
    return;

  progcwd(cwd, MAXPATHLEN);

  if (strncmp(cwd, wd, MAXPATHLEN-1) == 0)
    return;

  ret = chdir(wd);
  if (ret != 0) {
    printf("WARNING: chdir(%s) failed\n", wd);
    perror("chdir");
  }
}


char *progipwd(char *buf, size_t buflen) {

  const char *progpath_debug = getenv("PROGPATH_DEBUG");
  int debug = 0;
  int method = 0;
  struct method im = {0, __LINE__, "ipwd", 0};

  if (progpath_debug)
    debug = atoi(progpath_debug);

  if (debug & PP_PRINT)
    printf("=== progipwd() ===\n");

  if (progpath_ipwd[0]) {
    we_done_yet(im, &buf, buflen, progpath_ipwd);
    return buf; /* always return, even PP_CONTINUE */
  }


  /* TODO: add additional deterministic ipwd methods here */


  /* getenv(PWD) method only works for the initial working directory,
   * and is not updated after changing directories.
   */
  {
    char cwd[MAXPATHLEN] = {0};
    char mbuf[MAXPATHLEN] = {0};
    char *pwd;
    struct method m = METHOD("getenv(PWD)");
    pwd = getenv("PWD");
    if (pwd) {
      strncpy(cwd, pwd, MAXPATHLEN-1);
      finalize(m, mbuf, MAXPATHLEN, cwd);
      if (we_done_yet(m, &buf, buflen, mbuf))
        return buf;
    }
  }


  /* first call to progipwd() should be during init, so we can use the
   * current working directory when progpath_ipwd is unset.
   */
  {
    char cwd[MAXPATHLEN] = {0};
    char pbuf[MAXPATHLEN] = {0};

    if (debug & PP_PRINT) {
      printf("progipwd() using the current directory\n");
    }

    progcwd(cwd, MAXPATHLEN);
    finalize(im, pbuf, MAXPATHLEN, cwd);
    if (we_done_yet(im, &buf, buflen, pbuf))
      return buf;
  }

  if (buf && buf[0] != '\0')
    return buf;
  return NULL;
}


char *progpath(char *buf, size_t buflen) {

  /* environment variable can be set for debug printing */
  const char *progpath_debug = getenv("PROGPATH_DEBUG");

  /* we need to be where the app started before trying methods */
  char cwd[MAXPATHLEN] = {0};
  char ipwd[MAXPATHLEN] = {0};

  int debug = 0;
  int method = 0;

  if (progpath_debug)
    debug = atoi(progpath_debug);

  if (debug & PP_PRINT)
    printf("=== progpath() ===\n");

  progcwd(cwd, MAXPATHLEN);
  progipwd(ipwd, MAXPATHLEN);

  if (debug & PP_PRINT)
    printf("cwd=%s ipwd=%s\n", cwd, ipwd);

  chdir_if_diff(ipwd);

  /* verified, short: MacOSX, OpenBSD, Haiku */
#ifdef HAVE_GETPROGNAME
  {
    char mbuf[MAXPATHLEN] = {0};
    const char *argv0 = getprogname(); /* not malloc'd memory, may return NULL */
    struct method m = METHOD("getprogname");
    finalize(m, mbuf, MAXPATHLEN, argv0);
    if (we_done_yet(m, &buf, buflen, mbuf)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }
#endif


  /* UNVERIFIED: Solaris */
#ifdef HAVE_GETEXECNAME
  {
    char mbuf[MAXPATHLEN] = {0};
    const char *argv0 = getexecname();
    struct method m = METHOD("getexecname");
    finalize(m, mbuf, MAXPATHLEN, argv0);
    if (we_done_yet(m, &buf, buflen, mbuf)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }
#endif


  /* UNVERIFIED: Windows */
#ifdef HAVE_GETMODULEFILENAME
  {
    char mbuf[MAXPATHLEN] = {0};
    TCHAR exeFileName[MAXPATHLEN] = {0};
    GetModuleFileName(NULL, exeFileName, MAXPATHLEN);
    struct method m = METHOD("GetModuleFileName");
    if (sizeof(TCHAR) == sizeof(char))
	    strncpy(mbuf, exeFileName, MAXPATHLEN-1);
    else
	    wcstombs(mbuf, exeFileName, wcslen(mbuf)+1);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, &buf, buflen, mbuf)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }
#endif


  /* UNVERIFIED: Windows */
#ifdef HAVE__GET_PGMPTR
  {
    char mbuf[MAXPATHLEN] = {0};
    char *argv0 = NULL;
    struct method m = METHOD("_get_pgmptr");
    _get_pgmptr(&argv0);
    finalize(m, mbuf, MAXPATHLEN, argv0);
    if (we_done_yet(m, &buf, buflen, mbuf)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }
#endif


  /* verified, full: MacOSX */
#ifdef HAVE_PROC_PIDPATH
  {
    char mbuf[MAXPATHLEN] = {0};
    struct method m = METHOD("proc_pidpath");
    (void)proc_pidpath(getpid(), mbuf, buflen);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, &buf, buflen, mbuf)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }
#endif


  /* verified, relative: Linux */
#ifdef HAVE_DECL_PROGRAM_INVOCATION_NAME
  {
    extern char *program_invocation_name;
    if (program_invocation_name) {
      char mbuf[MAXPATHLEN] = {0};
      struct method m = METHOD("program_invocation_name");
      finalize(m, mbuf, MAXPATHLEN, program_invocation_name);
      if (we_done_yet(m, &buf, buflen, mbuf)) {
        chdir_if_diff(cwd);
        return buf;
      }
    }
  }
#endif


  /* verified, short: Linux */
#ifdef HAVE_DECL_PROGRAM_INVOCATION_SHORT_NAME
  {
    extern char *program_invocation_short_name;
    if (program_invocation_short_name) {
      char mbuf[MAXPATHLEN] = {0};
      struct method m = METHOD("program_invocation_short_name");
      finalize(m, mbuf, MAXPATHLEN, program_invocation_short_name);
      if (we_done_yet(m, &buf, buflen, mbuf)) {
        chdir_if_diff(cwd);
        return buf;
      }
    }
  }
#endif


  /* UNVERIFIED: Windows */
#ifdef HAVE_DECL___ARGV
  {
    extern char **__argv;
    if (__argv) {
      char mbuf[MAXPATHLEN] = {0};
      struct method m = METHOD("__argv");
      finalize(m, mbuf, MAXPATHLEN, __argv[0]);
      if (we_done_yet(m, &buf, buflen, mbuf)) {
        chdir_if_diff(cwd);
        return buf;
      }
    }
  }
#endif


  /* verified, relative: Linux */
  /* verified, short: MacOSX */
#ifdef HAVE_DECL___PROGNAME_FULL
  {
    extern char *__progname_full;
    if (__progname_full) {
      char mbuf[MAXPATHLEN] = {0};
      struct method m = METHOD("__progname_full");
      finalize(m, mbuf, MAXPATHLEN, __progname_full);
      if (we_done_yet(m, &buf, buflen, mbuf)) {
        chdir_if_diff(cwd);
        return buf;
      }
    }
  }
#endif


  /* UNVERIFIED, short: OpenBSD */
  /* verified, short: Linux, FreeBSD, Haiku */
#ifdef HAVE_DECL___PROGNAME
  {
    extern char *__progname;
    if (__progname) {
      char mbuf[MAXPATHLEN] = {0};
      struct method m = METHOD("__progname");
      finalize(m, mbuf, MAXPATHLEN, __progname);
      if (we_done_yet(m, &buf, buflen, mbuf)) {
        chdir_if_diff(cwd);
        return buf;
      }
    }
  }
#endif


  /* verified, relative: Linux */
#ifdef HAVE_GETAUXVAL
  {
    char mbuf[MAXPATHLEN] = {0};
    char *argv0 = (char *)getauxval(AT_EXECFN);
    struct method m = METHOD("getauxval");
    finalize(m, mbuf, MAXPATHLEN, argv0);
    if (we_done_yet(m, &buf, buflen, mbuf)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }
#endif


  /* verified, full: FreeBSD */
#if defined(HAVE_DECL_CTL_KERN) && defined(HAVE_DECL_KERN_PROC) && defined(HAVE_DECL_KERN_PROC_PATHNAME)
  {
    char mbuf[MAXPATHLEN] = {0};
    size_t len = MAXPATHLEN-1;
    int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
    struct method m = METHOD("sysctl(KERN_PROC)");
    sysctl(mib, 4, mbuf, &len, NULL, 0);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, &buf, buflen, mbuf)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }
#endif


  /* UNVERIFIED: NetBSD */
#if defined(HAVE_DECL_CTL_KERN) && defined(HAVE_DECL_KERN_PROC_ARGS) && defined(HAVE_DECL_KERN_PROC_PATHNAME)
  {
    char mbuf[MAXPATHLEN] = {0};
    size_t len = MAXPATHLEN-1;
    int mib[4] = {CTL_KERN, KERN_PROC_ARGS, -1, KERN_PROC_PATHNAME};
    struct method m = METHOD("sysctl(KERN_PROC_ARGS)");
    len = buflen;
    sysctl(mib, 4, buf, &len, NULL, 0);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, &buf, buflen, mbuf)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }
#endif


  /* verified, relative: MacOSX */
 #if defined(HAVE_DECL_CTL_KERN) && defined(HAVE_DECL_KERN_PROCARGS2)
  {
    char mbuf[MAXPATHLEN] = {0};
    int mib[4] = {CTL_KERN, KERN_ARGMAX, -1, -1};
    int argmax;
    size_t argmaxsz = sizeof(argmax);
    char *pbuf;
    size_t pbufsz;
    struct method m = METHOD("sysctl(KERN_PROCARGS2)");
    sysctl(mib, 2, &argmax, &argmaxsz, NULL, 0);
    pbuf = (char *)calloc(argmax, sizeof(char));
    mib[0] = CTL_KERN;  mib[1] = KERN_PROCARGS2;  mib[2] = getpid();  mib[3] = -1;
    pbufsz = (size_t)argmax; /* must be full size or sysctl returns nothing */
    sysctl(mib, 3, pbuf, &pbufsz, NULL, 0);
    finalize(m, mbuf, MAXPATHLEN, pbuf + sizeof(int)); /* from sysctl, exec_path comes after argc */
    if (we_done_yet(m, &buf, buflen, mbuf)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }
#endif


  /* verified, short: MacOSX */
#if defined(HAVE_DECL_CTL_KERN) && defined(HAVE_DECL_KERN_PROC) && defined(HAVE_DECL_KERN_PROCNAME)
  {
    char mbuf[MAXPATHLEN] = {0};
    int mib[4] = {CTL_KERN, KERN_PROCNAME, -1, -1};
    size_t len = MAXPATHLEN-1;
    struct method m = METHOD("sysctl(KERN_PROCNAME)");
    sysctl(mib, 2, mbuf, &len, NULL, 0);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, &buf, buflen, mbuf)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }
#endif


  /* verified, relative: OpenBSD */
#if defined(HAVE_DECL_CTL_KERN) && defined(HAVE_DECL_KERN_PROC_ARGS) && defined(HAVE_DECL_KERN_PROC_ARGV)
  {
    char mbuf[MAXPATHLEN] = {0};
    int mib[4] = {CTL_KERN, KERN_PROC_ARGS, getpid(), KERN_PROC_ARGV};
    char **retargs;
    size_t len = MAXPATHLEN-1;
    struct method m = METHOD("sysctl(KERN_PROCNAME)");
    sysctl(mib, 4, NULL, &len, NULL, 0);
    retargs = (char **)calloc(len, sizeof(char *));
    sysctl(mib, 4, retargs, &len, NULL, 0);
    finalize(m, mbuf, MAXPATHLEN, regargs[0]);
    free(retargs);
    if (we_done_yet(m, &buf, buflen, mbuf)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }
#endif


  /* verified, short: MacOSX */
#if defined(HAVE_SYSCTLBYNAME)
  {
    char mbuf[MAXPATHLEN] = {0};
    size_t len = MAXPATHLEN-1;
    struct method m = METHOD("sysctlbyname(kern.procname)");
    sysctlbyname("kern.procname", mbuf, &len, NULL, 0);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, &buf, buflen, mbuf)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }
#endif


  /* verified, full: MacOSX */
#ifdef HAVE__NSGETEXECUTABLEPATH
  {
    char mbuf[MAXPATHLEN] = {0};
    uint32_t ulen = MAXPATHLEN-1;
    struct method m = METHOD("_NSGetExecutablePath");
    _NSGetExecutablePath(mbuf, &ulen);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, &buf, buflen, mbuf)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }
#endif


  /* verified: Haiku */
#ifdef HAVE_FIND_PATH
  {
    char mbuf[MAXPATHLEN] = {0};
    struct method m = METHOD("find_path");
    find_path(B_APP_IMAGE_SYMBOL, B_FIND_PATH_IMAGE_PATH, NULL, mbuf, MAXPATHLEN);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, &buf, buflen, mbuf)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }
#endif


  /* verified, full: Linux */
#ifdef HAVE_READLINK
  {
    char mbuf[MAXPATHLEN] = {0};
    struct method m = METHOD("readlink(/proc/self/exe)");
    readlink("/proc/self/exe", mbuf, MAXPATHLEN-1);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, &buf, buflen, mbuf)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }
#endif


  /* UNVERIFIED: QNX */
#ifdef HAVE_READ
  {
    char mbuf[MAXPATHLEN] = {0};
    int fd;
    struct method m = METHOD("readlink(/proc/self/exefile)");
    fd = open("/proc/self/exefile", O_RDONLY);
    if (fd >= 0) {
      read(fd, mbuf, MAXPATHLEN-1);
      close(fd);
      finalize(m, mbuf, MAXPATHLEN, NULL);
      if (we_done_yet(m, &buf, buflen, mbuf)) {
        chdir_if_diff(cwd);
        return buf;
      }
    }
  }
#endif


  /* UNVERIFIED: OpenBSD */
  /* verified, full: FreeBSD with /proc */
#ifdef HAVE_READLINK
  {
    char mbuf[MAXPATHLEN] = {0};
    struct method m = METHOD("readlink(/proc/curproc/file)");
    readlink("/proc/curproc/file", mbuf, MAXPATHLEN-1);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, &buf, buflen, mbuf)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }
#endif


  /* UNVERIFIED: NetBSD */
#ifdef HAVE_READLINK
  {
    char mbuf[MAXPATHLEN] = {0};
    struct method m = METHOD("readlink(/proc/curproc/exe)");
    readlink("/proc/curproc/exe", mbuf, MAXPATHLEN-1);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, &buf, buflen, mbuf)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }
#endif


  /* verified, full: FreeBSD with /proc */
#ifdef HAVE_READLINK
  {
    char mbuf[MAXPATHLEN] = {0};
    char pbuf[MAXPATHLEN] = {0};
    struct method m = METHOD("readlink(/proc/$PID/file)");
    snprintf(pbuf, MAXPATHLEN-1, "/proc/%d/file", getpid());
    readlink(pbuf, mbuf, MAXPATHLEN-1);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, &buf, buflen, mbuf)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }
#endif


  /* verified, relative: AIX */
#ifdef HAVE_STRUCT_PSINFO
  {
    char mbuf[MAXPATHLEN] = {0};
    char pbuf[MAXPATHLEN] = {0};
    const char *argv0;
    struct psinfo p;
    int fd;
    struct method m = METHOD("read(/proc/$PID/psinfo)");
    snprintf(pbuf, MAXPATHLEN-1, "/proc/%d/psinfo", getpid());
    fd = open(pbuf, O_RDONLY);
    if (fd >= 0) {
      read(fd, &p, sizeof(p));
      close(fd);
      argv0 = (*(char ***)((intptr_t)p.pr_argv))[0];
      finalize(m, mbuf, MAXPATHLEN, argv0);
      if (we_done_yet(m, &buf, buflen, mbuf)) {
        chdir_if_diff(cwd);
        return buf;
      }
    }
  }
#endif


  /* UNVERIFIED: BSD, IRIX, Solaris */
#if defined(HAVE_STRUCT_PRPSINFO) && defined(HAVE_DECL_PIOCPSINFO)
  {
    char mbuf[MAXPATHLEN] = {0};
    char pbuf[MAXPATHLEN] = {0};
    const char *argv0;
    struct prpsinfo p;
    int fd;
    struct method m = METHOD("ioctl(/proc/$PID,prpsinfo)");
    snprintf(pbuf, sizeof(pbuf), "/proc/%d", getpid());
    fd = open(pbuf, O_RDONLY);
    if (fd >= 0) {
      ioctl(fd, PIOCPSINFO, &p);
      close(fd);
      argv0 = p.pr_fname;
      finalize(m, mbuf, MAXPATHLEN, argv0);
      if (we_done_yet(m, &buf, buflen, mbuf)) {
        chdir_if_diff(cwd);
        return buf;
      }
    }
  }
#endif


  /* UNVERIFIED: Solaris */
#ifdef HAVE_READLINK
  {
    char mbuf[MAXPATHLEN] = {0};
    char pbuf[MAXPATHLEN] = {0};
    struct method m = METHOD("readlink(/proc/$PID/cmdline)");
    snprintf(pbuf, MAXPATHLEN-1, "/proc/%d/cmdline", getpid());
    readlink(pbuf, mbuf, MAXPATHLEN-1);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, &buf, buflen, mbuf)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }
#endif


  /* verified, relative: Linux */
#ifdef HAVE_READ
  {
    char mbuf[MAXPATHLEN] = {0};
    char pbuf[MAXPATHLEN] = {0};
    int fd;
    struct method m = METHOD("read(/proc/$PID/cmdline)");
    snprintf(pbuf, MAXPATHLEN-1, "/proc/%d/cmdline", getpid());
    fd = open(pbuf, O_RDONLY);
    if (fd >= 0) {
      read(fd, mbuf, MAXPATHLEN-1);
      close(fd);
      finalize(m, mbuf, MAXPATHLEN, NULL);
      if (we_done_yet(m, &buf, buflen, mbuf)) {
        chdir_if_diff(cwd);
        return buf;
      }
    }
  }
#endif


  /* UNVERIFIED: Solaris */
#ifdef HAVE_READLINK
  {
    char mbuf[MAXPATHLEN] = {0};
    char pbuf[MAXPATHLEN] = {0};
    struct method m = METHOD("readlink(/proc/$PID/path/a.out)");
    snprintf(pbuf, MAXPATHLEN-1, "/proc/%d/path/a.out", getpid());
    readlink(pbuf, mbuf, MAXPATHLEN-1);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, &buf, buflen, mbuf)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }
#endif


  /* UNVERIFIED: Solaris */
#ifdef HAVE_READLINK
  {
    char mbuf[MAXPATHLEN] = {0};
    struct method m = METHOD("readlink(/proc/self/path/a.out)");
    readlink("/proc/self/path/a.out", mbuf, MAXPATHLEN-1);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, &buf, buflen, mbuf)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }
#endif


  /* UNVERIFIED: IRIX */
#ifdef HAVE_READLINK
  {
    char mbuf[MAXPATHLEN] = {0};
    struct method m = METHOD("readlink(/proc/pinfo)");
    readlink("/proc/pinfo", mbuf, MAXPATHLEN-1);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, &buf, buflen, mbuf)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }
#endif


  /* UNVERIFIED: OSF */
#ifdef HAVE_READLINK
  {
    char mbuf[MAXPATHLEN] = {0};
    char pbuf[MAXPATHLEN] = {0};
    struct method m = METHOD("readlink(/proc/$PID)");
    snprintf(pbuf, MAXPATHLEN-1, "/proc/%d", getpid());
    readlink(pbuf, mbuf, MAXPATHLEN-1);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, &buf, buflen, mbuf)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }
#endif


  /* verified, full: MacOSX, Haiku */
  /* verified, relative: OpenBSD */
#ifdef HAVE_DLADDR
  {
    char mbuf[MAXPATHLEN] = {0};
    Dl_info i;
    const void *mainfunc = dlsym(RTLD_DEFAULT, "main");
    struct method m = METHOD("dladdr(main)");
    dladdr(mainfunc, &i);
    finalize(m, mbuf, MAXPATHLEN, i.dli_fname);
    if (we_done_yet(m, &buf, buflen, mbuf)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }
#endif


  /* verified, short: AIX */
#ifdef HAVE_GETPROCS
  {
    char mbuf[MAXPATHLEN] = {0};
    const char *argv0 = buf;
    struct procsinfo pinfo[16];
    int numproc;
    int index = 0;
    struct method m = METHOD("getprocs");
    while ((numproc = getprocs(pinfo, sizeof(struct procsinfo), NULL, 0, &index, 16)) > 0) {
      for (int i = 0; i < numproc; i++) {
        if (pinfo[i].pi_state == SZOMB)
                continue;
        if (getpid() == (pid_t)pinfo[i].pi_pid) {
          argv0 = pinfo[i].pi_comm;
          finalize(m, mbuf, MAXPATHLEN, argv0);
          break;
        }
      }
    }
    if (we_done_yet(m, &buf, buflen, mbuf)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }
#endif


  /* verified, short: AIX */
#ifdef HAVE_GETPROCS64
  {
    char mbuf[MAXPATHLEN] = {0};
    const char *argv0 = buf;
    struct procentry64 *pentry;
    int numproc;
    int index = 0;
    pid_t proc1 = (pid_t)0;
    int proccnt;
    struct method m = METHOD("getprocs64");

    /* get up to 1M procs, same limit as IBM ps command */
    proccnt = getprocs64(NULL, 0, NULL, 0, &proc1, 1000000);
    pentry = calloc(proccnt, sizeof(struct procentry64));
    while ((numproc = getprocs64(pentry, sizeof(struct procentry64), NULL, 0, &index, proccnt)) > 0) {
      for (int i = 0; i < numproc; i++) {
        if (pentry[i].pi_state == SZOMB)
                continue;
        if (getpid() == (int)pentry[i].pi_pid) {
          argv0 = pentry[i].pi_comm;
          finalize(m, mbuf, MAXPATHLEN, argv0);
          break;
        }
      }
    }
    free(pentry);
    if (we_done_yet(m, &buf, buflen, mbuf)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }
#endif


#if 0
  /* Unreliable */
  /* verified, relative: FreeBSD bash */
  {
    char mbuf[MAXPATHLEN] = {0};
    char *argv0 = getenv("_");
    struct method m = METHOD("getenv(\"_\")");
    finalize(m, mbuf, MAXPATHLEN, argv0);
    if (we_done_yet(m, &buf, buflen, mbuf)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }
#endif

  /* really just to appease the compiler */
  {
    struct method m = METHOD("UNKNOWN");
    if (we_done_yet(m, &buf, buflen, NULL)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }

  /* restore path if needed */
  chdir_if_diff(cwd);

  if (buf && buf[0] != '\0')
    return buf;
  return NULL;
}


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
