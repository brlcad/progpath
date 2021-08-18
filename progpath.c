
#include "progpath_config.h"
#include "progpath.h"

#ifndef BUILD_BINARY

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
#endif

extern const char *getprogname(void);
extern const char *getexecname(void);
extern int getpid(void);
extern void proc_pidpath(int, char *, size_t);

#ifndef HAVE_UNISTD_H
//extern int readlink(const char *, char *, size_t); /* or ssize_t return */
#endif


#ifndef MAXPATHLEN
#  ifdef PATH_MAX
#    define MAXPATHLEN PATH_MAX
#  else
#    define MAXPATHLEN 4096
#  endif
#endif


/* stateful structure used for debugging */
struct method {
  int id;
  int line;
  const char *label;
  int debug;
};


/* debug states */
enum {
  PP_DEFAULT = 0,
  PP_PRINT = 1,
  PP_CONTINUE = 2
};


/* PROGPATH_DEBUG=1 environment variable can be set in caller scope to
 * print useful debugging lines for methods that have results.
 */
static void print_method(struct method m, const char *result) {
  if (m.debug >= PP_PRINT)
    printf("Method %0.2d, line %0.4d: %s=[%s]\n", m.id, m.line, m.label, result);
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
static int we_done_yet(struct method m, char *buf, size_t buflen, const char *path) {
  size_t pathlen;
  if (!path)
    return 0;

  pathlen = strlen(path);
  if (pathlen < 2)
    return 0;

  /* if we seem to have a full path, we're done */
  if ((path[0] == '/') ||
      (pathlen > 2 && path[0] >= 'A' && path[0] <= 'Z' && path[1] == ':' && path[2] == '\\')) {

    if (!buf || !buflen) {
      buflen = MAXPATHLEN;
      buf = calloc(buflen, sizeof(char));
      assert(buf);
    }

    strncpy(buf, path, buflen-1);

    if (m.debug >= PP_CONTINUE)
      return 0;
    return 1;
  }

  return 0;
}


/* main workhorse.  if 'buf' is NULL and a full path is identified,
 * memory is allocated dynamically, returned, and caller must call
 * free() it.  function returns NULL and 'buf' is unmodified if
 * program path cannot be found.  function returns pointer to path
 * held in 'buf'
 */
char *progpath(char *buf, size_t buflen) {
  int method = 0;

  /* environment variable can be set for debug printing */
  const char *progpath_debug = getenv("PROGPATH_DEBUG");
  int debug = 0;

  if (progpath_debug)
    debug = atoi(progpath_debug);

  /* verified, short: MacOSX, OpenBSD */
#ifdef HAVE_GETPROGNAME
  {
    struct method m = {++method, __LINE__, "getprogname", debug};
    char mbuf[MAXPATHLEN] = {0};
    const char *argv0 = getprogname(); /* not malloc'd memory, may return NULL */
    finalize(m, mbuf, MAXPATHLEN, argv0);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* UNVERIFIED: Solaris */
#ifdef HAVE_GETEXECNAME
  {
    struct method m = {++method, __LINE__, "getexecname", debug};
    char mbuf[MAXPATHLEN] = {0};
    const char *argv0 = getexecname();
    finalize(m, mbuf, MAXPATHLEN, argv0);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* UNVERIFIED: Windows */
#ifdef HAVE_GETMODULEFILENAME
  {
    struct method m = {++method, __LINE__, "GetModuleFileName", debug};
    char mbuf[MAXPATHLEN] = {0};
    TCHAR exeFileName[MAXPATHLEN] = {0};
    GetModuleFileName(NULL, exeFileName, MAXPATHLEN);
    if (sizeof(TCHAR) == sizeof(char))
	    strncpy(mbuf, exeFileName, MAXPATHLEN-1);
    else
	    wcstombs(mbuf, exeFileName, wcslen(mbuf)+1);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* UNVERIFIED: Windows */
#ifdef HAVE__GET_PGMPTR
  {
    struct method m = {++method, __LINE__, "_get_pgmptr", debug};
    char mbuf[MAXPATHLEN] = {0};
    char *argv0 = NULL;
    _get_pgmptr(&argv0);
    finalize(m, mbuf, MAXPATHLEN, argv0);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* verified, full: MacOSX */
#ifdef HAVE_PROC_PIDPATH
  {
    struct method m = {++method, __LINE__, "proc_pidpath", debug};
    char mbuf[MAXPATHLEN] = {0};
    (void)proc_pidpath(getpid(), mbuf, buflen);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* verified, relative: Linux */
#ifdef HAVE_DECL_PROGRAM_INVOCATION_NAME
  extern char *program_invocation_name;
  if (program_invocation_name) {
    struct method m = {++method, __LINE__, "program_invocation_name", debug};
    char mbuf[MAXPATHLEN] = {0};
    finalize(m, mbuf, MAXPATHLEN, program_invocation_name);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* verified, short: Linux */
#ifdef HAVE_DECL_PROGRAM_INVOCATION_SHORT_NAME
  extern char *program_invocation_short_name;
  if (program_invocation_short_name) {
    struct method m = {++method, __LINE__, "program_invocation_short_name", debug};
    char mbuf[MAXPATHLEN] = {0};
    finalize(m, mbuf, MAXPATHLEN, program_invocation_short_name);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* UNVERIFIED: Windows */
#ifdef HAVE_DECL___ARGV
  extern char **__argv;
  if (__argv) {
    struct method m = {++method, __LINE__, "__argv", debug};
    char mbuf[MAXPATHLEN] = {0};
    finalize(m, mbuf, MAXPATHLEN, __argv[0]);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* UNVERIFIED, relative: Linux */
  /* verified, short: MacOSX */
#ifdef HAVE_DECL___PROGNAME_FULL
  extern char *__progname_full;
  if (__progname_full) {
    struct method m = {++method, __LINE__, "__progname_full", debug};
    char mbuf[MAXPATHLEN] = {0};
    finalize(m, mbuf, MAXPATHLEN, __progname_full);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* UNVERIFIED, short: Linux, OpenBSD */
  /* verified, short: FreeBSD */
#ifdef HAVE_DECL___PROGNAME
  extern char *__progname;
  if (__progname) {
    struct method m = {++method, __LINE__, "__progname", debug};
    char mbuf[MAXPATHLEN] = {0};
    finalize(m, mbuf, MAXPATHLEN, __progname);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* UNVERIFIED: Linux */
#ifdef HAVE_GETAUXVAL
  {
    struct method m = {++method, __LINE__, "getauxval", debug};
    char mbuf[MAXPATHLEN] = {0};
    char *argv0 = (char *)getauxval(AT_EXECFN);
    finalize(m, mbuf, MAXPATHLEN, argv0);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* verified, full: FreeBSD */
#if defined(HAVE_DECL_CTL_KERN) && defined(HAVE_DECL_KERN_PROC) && defined(HAVE_DECL_KERN_PROC_PATHNAME)
  {
    struct method m = {++method, __LINE__, "sysctl(KERN_PROC)", debug};
    char mbuf[MAXPATHLEN] = {0};
    size_t len = MAXPATHLEN-1;
    int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
    sysctl(mib, 4, mbuf, &len, NULL, 0);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* UNVERIFIED: NetBSD */
#if defined(HAVE_DECL_CTL_KERN) && defined(HAVE_DECL_KERN_PROC_ARGS) && defined(HAVE_DECL_KERN_PROC_PATHNAME)
  {
    struct method m = {++method, __LINE__, "sysctl(KERN_PROC_ARGS)", debug};
    char mbuf[MAXPATHLEN] = {0};
    size_t len = MAXPATHLEN-1;
    int mib[4] = {CTL_KERN, KERN_PROC_ARGS, -1, KERN_PROC_PATHNAME};
    len = buflen;
    sysctl(mib, 4, buf, &len, NULL, 0);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* verified, relative: MacOSX */
 #if defined(HAVE_DECL_CTL_KERN) && defined(HAVE_DECL_KERN_PROCARGS2)
  {
    struct method m = {++method, __LINE__, "sysctl(KERN_PROCARGS2)", debug};
    char mbuf[MAXPATHLEN] = {0};
    int mib[4] = {CTL_KERN, KERN_ARGMAX, -1, -1};
    int argmax;
    size_t argmaxsz = sizeof(argmax);
    char *pbuf;
    size_t pbufsz;
    sysctl(mib, 2, &argmax, &argmaxsz, NULL, 0);
    pbuf = calloc(argmax, sizeof(char));
    mib[0] = CTL_KERN;  mib[1] = KERN_PROCARGS2;  mib[2] = getpid();  mib[3] = -1;
    pbufsz = (size_t)argmax; // must be full size or sysctl returns nothing
    sysctl(mib, 3, pbuf, &pbufsz, NULL, 0);
    finalize(m, mbuf, MAXPATHLEN, pbuf + sizeof(int)); /* from sysctl, exec_path comes after argc */
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* verified, short: MacOSX */
#if defined(HAVE_DECL_CTL_KERN) && defined(HAVE_DECL_KERN_PROC) && defined(HAVE_DECL_KERN_PROCNAME)
  {
    struct method m = {++method, __LINE__, "sysctl(KERN_PROCNAME)", debug};
    char mbuf[MAXPATHLEN] = {0};
    int mib[4] = {CTL_KERN, KERN_PROCNAME, -1, -1};
    size_t len = MAXPATHLEN-1;
    sysctl(mib, 2, mbuf, &len, NULL, 0);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* verified, relative: OpenBSD */
#if defined(HAVE_DECL_CTL_KERN) && defined(HAVE_DECL_KERN_PROC_ARGS) && defined(HAVE_DECL_KERN_PROC_ARGV)
  {
    struct method m = {++method, __LINE__, "sysctl(KERN_PROCNAME)", debug};
    char mbuf[MAXPATHLEN] = {0};
    int mib[4] = {CTL_KERN, KERN_PROC_ARGS, getpid(), KERN_PROC_ARGV};
    char **retargs;
    size_t len = MAXPATHLEN-1;
    sysctl(mib, 4, NULL, &len, NULL, 0);
    retargs = calloc(len, sizeof(char *));
    sysctl(mib, 4, retargs, &len, NULL, 0);
    finalize(m, mbuf, MAXPATHLEN, regargs[0]);
    free(retargs);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* verified, short: MacOSX */
#if defined(HAVE_SYSCTLBYNAME)
  {
    struct method m = {++method, __LINE__, "sysctlbyname(kern.procname)", debug};
    char mbuf[MAXPATHLEN] = {0};
    size_t len = MAXPATHLEN-1;
    sysctlbyname("kern.procname", mbuf, &len, NULL, 0);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* verified, full: MacOSX */
#ifdef HAVE__NSGETEXECUTABLEPATH
  {
    struct method m = {++method, __LINE__, "_NSGetExecutablePath", debug};
    char mbuf[MAXPATHLEN] = {0};
    uint32_t ulen = MAXPATHLEN-1;
    _NSGetExecutablePath(mbuf, &ulen);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* UNVERIFIED: Haiku */
#ifdef HAVE_FIND_PATH
  {
    struct method m = {++method, __LINE__, "find_path", debug};
    char mbuf[MAXPATHLEN] = {0};
    find_path(B_APP_IMAGE_SYMBOL, B_FIND_PATH_IMAGE_PATH, NULL, mbuf, MAXPATHLEN);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* UNVERIFIED: Linux */
#ifdef HAVE_READLINK
  {
    struct method m = {++method, __LINE__, "readlink(/proc/self/exe)", debug};
    char mbuf[MAXPATHLEN] = {0};
    readlink("/proc/self/exe", mbuf, MAXPATHLEN-1);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* UNVERIFIED: QNX */
#ifdef HAVE_READLINK
  {
    struct method m = {++method, __LINE__, "readlink(/proc/self/exefile)", debug};
    char mbuf[MAXPATHLEN] = {0};
    int fd;
    fd = open("/proc/self/exefile", O_RDONLY);
    read(fd, mbuf, MAXPATHLEN-1);
    close(fd);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* UNVERIFIED: OpenBSD */
  /* verified, full: FreeBSD with /proc */
#ifdef HAVE_READLINK
  {
    struct method m = {++method, __LINE__, "readlink(/proc/curproc/file)", debug};
    char mbuf[MAXPATHLEN] = {0};
    readlink("/proc/curproc/file", mbuf, MAXPATHLEN-1);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* UNVERIFIED: NetBSD */
#ifdef HAVE_READLINK
  {
    struct method m = {++method, __LINE__, "readlink(/proc/curproc/exe)", debug};
    char mbuf[MAXPATHLEN] = {0};
    readlink("/proc/curproc/exe", mbuf, MAXPATHLEN-1);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* verified, full: FreeBSD with /proc */
#ifdef HAVE_READLINK
  {
    struct method m = {++method, __LINE__, "readlink(/proc/$PID/file)", debug};
    char mbuf[MAXPATHLEN] = {0};
    char pbuf[MAXPATHLEN] = {0};
    snprintf(pbuf, MAXPATHLEN-1, "/proc/%d/file", getpid());
    readlink(pbuf, mbuf, MAXPATHLEN-1);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* (RECHECK)verified, relative: AIX */
#ifdef HAVE_STRUCT_PSINFO
  {
    struct method m = {++method, __LINE__, "read(/proc/$PID/psinfo)", debug};
    char mbuf[MAXPATHLEN] = {0};
    char pbuf[MAXPATHLEN] = {0};
    const char *argv0;
    struct psinfo p;
    int fd;
    snprintf(pbuf, MAXPATHLEN-1, "/proc/%d/psinfo", getpid());
    fd = open(pbuf, O_RDONLY);
    read(fd, &p, sizeof(p));
    close(fd);
    argv0 = (*(char ***)((intptr_t)p.pr_argv))[0];
    finalize(m, mbuf, MAXPATHLEN, argv0);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* UNVERIFIED: BSD, IRIX, Solaris */
#if defined(HAVE_STRUCT_PRPSINFO) && defined(HAVE_DECL_PIOCPSINFO)
  {
    struct method m = {++method, __LINE__, "ioctl(/proc/$PID,prpsinfo)", debug};
    char mbuf[MAXPATHLEN] = {0};
    char pbuf[MAXPATHLEN] = {0};
    const char *argv0;
    struct prpsinfo p;
    int fd;
    snprintf(pbuf, sizeof(pbuf), "/proc/%d", getpid());
    fd = open(pbuf, O_RDONLY);
    ioctl(fd, PIOCPSINFO, &p);
    close(fd);
    argv0 = p.pr_fname;
    printf("av0 = %s\n",argv0);
    finalize(m, mbuf, MAXPATHLEN, argv0);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* UNVERIFIED: Solaris */
#ifdef HAVE_READLINK
  {
    struct method m = {++method, __LINE__, "readlink(/proc/$PID/cmdline)", debug};
    char mbuf[MAXPATHLEN] = {0};
    char pbuf[MAXPATHLEN] = {0};
    snprintf(pbuf, MAXPATHLEN-1, "/proc/%d/cmdline", getpid());
    readlink(pbuf, mbuf, MAXPATHLEN-1);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* UNVERIFIED: Solaris */
#ifdef HAVE_READLINK
  {
    struct method m = {++method, __LINE__, "readlink(/proc/$PID/path/a.out)", debug};
    char mbuf[MAXPATHLEN] = {0};
    char pbuf[MAXPATHLEN] = {0};
    snprintf(pbuf, MAXPATHLEN-1, "/proc/%d/path/a.out", getpid());
    readlink(pbuf, mbuf, MAXPATHLEN-1);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* UNVERIFIED: Solaris */
#ifdef HAVE_READLINK
  {
    struct method m = {++method, __LINE__, "readlink(/proc/self/path/a.out)", debug};
    char mbuf[MAXPATHLEN] = {0};
    readlink("/proc/self/path/a.out", mbuf, MAXPATHLEN-1);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* UNVERIFIED: IRIX */
#ifdef HAVE_READLINK
  {
    struct method m = {++method, __LINE__, "readlink(/proc/pinfo)", debug};
    char mbuf[MAXPATHLEN] = {0};
    readlink("/proc/pinfo", mbuf, MAXPATHLEN-1);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* UNVERIFIED: OSF */
#ifdef HAVE_READLINK
  {
    struct method m = {++method, __LINE__, "readlink(/proc/$PID)", debug};
    char mbuf[MAXPATHLEN] = {0};
    char pbuf[MAXPATHLEN] = {0};
    snprintf(pbuf, MAXPATHLEN-1, "/proc/%d", getpid());
    readlink(pbuf, mbuf, MAXPATHLEN-1);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* verified, full: MacOSX */
  /* verified, relative: OpenBSD */
#ifdef HAVE_DLADDR
  {
    //extern int (main)(int ac, char **);
    struct method m = {++method, __LINE__, "dladdr(main)", debug};
    char mbuf[MAXPATHLEN] = {0};
    Dl_info i;
    const void *mainfunc = dlsym(RTLD_DEFAULT, "main");
    dladdr(mainfunc, &i);
    finalize(m, mbuf, MAXPATHLEN, i.dli_fname);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* verified, short: AIX */
#ifdef HAVE_GETPROCS
  {
    struct method m = {++method, __LINE__, "getprocs", debug};
    char mbuf[MAXPATHLEN] = {0};
    const char *argv0 = buf;
    struct procsinfo pinfo[16];
    int numproc;
    int index = 0;
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
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


  /* verified, short: AIX */
#ifdef HAVE_GETPROCS64
  {
    struct method m = {++method, __LINE__, "getprocs64", debug};
    char mbuf[MAXPATHLEN] = {0};
    const char *argv0 = buf;
    struct procentry64 *pentry;
    int numproc;
    int index = 0;
    pid_t proc1 = (pid_t)0;

    // get up to 1M procs, same limit as IBM ps command
    int proccnt = getprocs64(NULL, 0, NULL, 0, &proc1, 1000000);
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
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif


#if 0
  /* Unreliable */
  /* verfified, relative: FreeBSD bash */
  {
    struct method m = {++method, __LINE__, "getenv(\"_\")", debug};
    char mbuf[MAXPATHLEN] = {0};
    char *argv0 = getenv("_");
    finalize(m, mbuf, MAXPATHLEN, argv0);
    if (we_done_yet(m, buf, buflen, mbuf))
      return buf;
  }
#endif

  return buf;
}


#else
/* build a simple main that executes the API call */


#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_SYS_PARAM_H /* for MAXPATHLEN */
#  include <sys/param.h>
#endif

#ifndef MAXPATHLEN
#  define MAXPATHLEN 4096
#endif


int main(int ac, char *av[]) {
  char buf[MAXPATHLEN] = {0};

  if (ac > 1) {
    printf("Usage: %s\n", av[0]);
    return 1;
  }

  progpath(buf, MAXPATHLEN);
  if (buf[0]) {
    printf("%s\n", buf);
    return 0;
  }

  fprintf(stderr, "ERROR: failed to get the program's path\n");
  return 1;
}

#endif
