
#include "progpath_config.h"
#include "progpath.h"

#ifndef BUILD_BINARY

#define _GNU_SOURCE 1

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

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
#ifdef HAVE_DLFCN_H /* for dladdr */
#  include <dlfcn.h>
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
#ifdef HAVE_WINDOWS_H
#  include <windows.h>
#endif

extern const char *getprogname(void);
extern const char *getexecname(void);
extern int getpid(void);
extern void proc_pidpath(int, char *, size_t);
/* or ssize_t return */
extern int readlink(const char *, char *, size_t);


#ifndef MAXPATHLEN
#  ifdef PATH_MAX
#    define MAXPATHLEN PATH_MAX
#  else
#    define MAXPATHLEN 4096
#  endif
#endif


struct method {
  int id;
  int line;
  const char *label;
  int print;
};


static void print_method(struct method m, const char *result) {
  if (m.print)
    printf("Method %0.2d, line %0.4d: %s=[%s]\n", m.id, m.line, m.label, result);
}


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


char *progpath(char *buf, size_t buflen) {
  int method = 0;

  const char *progpath_debug = getenv("PROGPATH_DEBUG");
  int debug = (progpath_debug) ? 1 : 0;

  /* FIXME: should only allocate if returning a result */
  if (!buf || !buflen) {
    buflen = MAXPATHLEN;
    buf = calloc(buflen, sizeof(char));
    assert(buf);
  }


  /* verified, MacOSX, OpenBSD */
  /* short name */
#ifdef HAVE_GETPROGNAME
  {
    struct method m = {++method, __LINE__, "getprogname", debug};
    char mbuf[MAXPATHLEN] = {0};
    const char *argv0 = getprogname(); /* not malloc'd memory, may return NULL */
    finalize(m, mbuf, MAXPATHLEN, argv0);
  }
#endif


  /* UNVERIFIED, Sun */
#ifdef HAVE_GETEXECNAME
  {
    struct method m = {++method, __LINE__, "getexecname", debug};
    char mbuf[MAXPATHLEN] = {0};
    const char *argv0 = getexecname();
    finalize(m, mbuf, MAXPATHLEN, argv0);
  }
#endif


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
  }
#endif


  /* verified, MacOSX */
#ifdef HAVE_PROC_PIDPATH
  {
    struct method m = {++method, __LINE__, "proc_pidpath", debug};
    char mbuf[MAXPATHLEN] = {0};
    (void)proc_pidpath(getpid(), mbuf, buflen);
    finalize(m, mbuf, MAXPATHLEN, NULL);
  }
#endif


  /* verified, Linux */
  /* relative path name */
#ifdef HAVE_DECL_PROGRAM_INVOCATION_NAME
  extern char *program_invocation_name;
  if (program_invocation_name) {
    struct method m = {++method, __LINE__, "program_invocation_name", debug};
    char mbuf[MAXPATHLEN] = {0};
    finalize(m, mbuf, MAXPATHLEN, program_invocation_name);
  }
#endif


  /* verified, Linux */
  /* short name */
#ifdef HAVE_DECL_PROGRAM_INVOCATION_SHORT_NAME
  extern char *program_invocation_short_name;
  if (program_invocation_short_name) {
    struct method m = {++method, __LINE__, "program_invocation_short_name", debug};
    char mbuf[MAXPATHLEN] = {0};
    finalize(m, mbuf, MAXPATHLEN, program_invocation_short_name);
  }
#endif


#ifdef HAVE_DECL___ARGV
  extern char **__argv;
  if (__argv) {
    struct method m = {++method, __LINE__, "__argv", debug};
    char mbuf[MAXPATHLEN] = {0};
    finalize(m, mbuf, MAXPATHLEN, __argv[0]);
  }
#endif


  /* verified, Linux, MacOSX */
  /* relative path name */
#ifdef HAVE_DECL___PROGNAME_FULL
  extern char *__progname_full;
  if (__progname_full) {
    struct method m = {++method, __LINE__, "__progname_full", debug};
    char mbuf[MAXPATHLEN] = {0};
    finalize(m, mbuf, MAXPATHLEN, __progname_full);
  }
#endif


  /* verified, Linux, OpenBSD */
  /* short name */
#ifdef HAVE_DECL___PROGNAME
  extern char *__progname;
  if (__progname) {
    struct method m = {++method, __LINE__, "__progname", debug};
    char mbuf[MAXPATHLEN] = {0};
    finalize(m, mbuf, MAXPATHLEN, __progname);
  }
#endif


  /* verified, FreeBSD */
#if defined(HAVE_DECL_CTL_KERN) && defined(HAVE_DECL_KERN_PROC) && defined(HAVE_DECL_KERN_PROC_PATHNAME)
  {
    struct method m = {++method, __LINE__, "sysctl(KERN_PROC)", debug};
    char mbuf[MAXPATHLEN] = {0};
    size_t len = MAXPATHLEN-1;
    int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
    sysctl(mib, 4, mbuf, &len, NULL, 0);
    finalize(m, mbuf, MAXPATHLEN, NULL);
  }
#endif


#if defined(HAVE_DECL_CTL_KERN) && defined(HAVE_DECL_KERN_PROC_ARGS) && defined(HAVE_DECL_KERN_PROC_PATHNAME)
  {
    struct method m = {++method, __LINE__, "sysctl(KERN_PROC_ARGS)", debug};
    char mbuf[MAXPATHLEN] = {0};
    size_t len = MAXPATHLEN-1;
    int mib[4] = {CTL_KERN, KERN_PROC_ARGS, -1, KERN_PROC_PATHNAME};
    len = buflen;
    sysctl(mib, 4, buf, &len, NULL, 0);
    finalize(m, mbuf, MAXPATHLEN, NULL);
  }
#endif


  /* verified, MacOSX */
  /* relative path name */
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
  }
#endif


  /* verified, MacOSX */
  /* short name */
#if defined(HAVE_DECL_CTL_KERN) && defined(HAVE_DECL_KERN_PROC) && defined(HAVE_DECL_KERN_PROCNAME)
  {
    struct method m = {++method, __LINE__, "sysctl(KERN_PROCNAME)", debug};
    char mbuf[MAXPATHLEN] = {0};
    int mib[4] = {CTL_KERN, KERN_PROCNAME, -1, -1};
    size_t len = MAXPATHLEN-1;
    sysctl(mib, 2, mbuf, &len, NULL, 0);
    finalize(m, mbuf, MAXPATHLEN, NULL);
  }
#endif


  /* verified, OpenBSD */
  /* relative path name */
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
  }
#endif


  /* verified, MacOSX */
  /* short name */
#if defined(HAVE_SYSCTLBYNAME)
  {
    struct method m = {++method, __LINE__, "sysctlbyname(kern.procname)", debug};
    char mbuf[MAXPATHLEN] = {0};
    size_t len = MAXPATHLEN-1;
    sysctlbyname("kern.procname", mbuf, &len, NULL, 0);
    finalize(m, mbuf, MAXPATHLEN, NULL);
  }
#endif


  /* verified, MacOSX */
#ifdef HAVE__NSGETEXECUTABLEPATH
  {
    struct method m = {++method, __LINE__, "_NSGetExecutablePath", debug};
    char mbuf[MAXPATHLEN] = {0};
    uint32_t ulen = MAXPATHLEN-1;
    _NSGetExecutablePath(mbuf, &ulen);
    finalize(m, mbuf, MAXPATHLEN, NULL);
  }
#endif


#ifdef HAVE_FIND_PATH
  {
    struct method m = {++method, __LINE__, "find_path", debug};
    char mbuf[MAXPATHLEN] = {0};
    find_path(B_APP_IMAGE_SYMBOL, B_FIND_PATH_IMAGE_PATH, NULL, mbuf, MAXPATHLEN);
    finalize(m, mbuf, MAXPATHLEN, NULL);
  }
#endif

  /* PID-based methods */

  // IRIX: /proc/pinfo
  // LINUX: /proc/self/exe
  // OBSD: /proc/curproc/file
  // OSF: /proc/%d
  // SYSV: /proc/%d/cmdline


#ifdef HAVE_READLINK
  /* verified, Linux */
  {
    struct method m = {++method, __LINE__, "readlink(/proc/self/exe)", debug};
    char mbuf[MAXPATHLEN] = {0};
    readlink("/proc/self/exe", mbuf, MAXPATHLEN-1);
    finalize(m, mbuf, MAXPATHLEN, NULL);
  }
#endif


#ifdef HAVE_READLINK
  {
    struct method m = {++method, __LINE__, "readlink(/proc/curproc/file)", debug};
    char mbuf[MAXPATHLEN] = {0};
    readlink("/proc/curproc/file", mbuf, MAXPATHLEN-1);
    finalize(m, mbuf, MAXPATHLEN, NULL);
  }
#endif


#ifdef HAVE_READLINK
  {
    struct method m = {++method, __LINE__, "readlink(/proc/pinfo)", debug};
    char mbuf[MAXPATHLEN] = {0};
    readlink("/proc/pinfo", mbuf, MAXPATHLEN-1);
    finalize(m, mbuf, MAXPATHLEN, NULL);
  }
#endif


#ifdef HAVE_READLINK
  {
    struct method m = {++method, __LINE__, "readlink(/proc/$PID)", debug};
    char mbuf[MAXPATHLEN] = {0};
    char pbuf[MAXPATHLEN] = {0};
    snprintf(pbuf, MAXPATHLEN-1, "/proc/%d", getpid());
    readlink(pbuf, mbuf, MAXPATHLEN-1);
    finalize(m, mbuf, MAXPATHLEN, NULL);
  }
#endif


#ifdef HAVE_READLINK
  {
    struct method m = {++method, __LINE__, "readlink(/proc/$PID/cmdline)", debug};
    char mbuf[MAXPATHLEN] = {0};
    char pbuf[MAXPATHLEN] = {0};
    snprintf(pbuf, MAXPATHLEN-1, "/proc/%d/cmdline", getpid());
    readlink(pbuf, mbuf, MAXPATHLEN-1);
    finalize(m, mbuf, MAXPATHLEN, NULL);
  }
#endif


  /* verified, AIX */
  /* relative path on AIX */
#if defined(HAVE_READLINK) && defined(HAVE_SYS_PROCFS_H)
  {
    struct method m = {++method, __LINE__, "readlink(/proc/$PID/psinfo)", debug};
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
  }
#endif


  /* verified, MacOSX */
  /* relative path on OBSD */
#ifdef HAVE_DLADDR
  {
    struct method m = {++method, __LINE__, "dladdr(main)", debug};
    char mbuf[MAXPATHLEN] = {0};
    Dl_info i;
    dladdr(&main, &i);
    finalize(m, mbuf, MAXPATHLEN, i.dli_fname);
  }
#endif


  /* verified, AIX */
  /* short name */
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
  }
#endif


  /* verified, AIX */
  /* short name */
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
  }
#endif

  // HPUX64: pstat_getproc64()
  // HPUX: pstat_getproc()
  // IBM: w_getpsent()

  /* TODO: PATH-based methods */

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
