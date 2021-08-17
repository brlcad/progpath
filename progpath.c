
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
#  define MAXPATHLEN 4096
#endif


char *progpath(char *buf, size_t buflen) {
  int method = 0;
  char *debug = getenv("PROGPATH_DEBUG");

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
    const char *argv0 = getprogname(); /* not malloc'd memory, may return NULL */
    if (argv0) {
      strncpy(buf, argv0, buflen-1);
      if (debug)
        printf("Method %0.2d, line %0.4d: getprogname=[%s]\n", ++method, __LINE__, buf);
    }
  }
#endif


#ifdef HAVE_GETEXECNAME
  {
    const char *argv0 = getexecname();
    if (argv0) {
      strncpy(buf, argv0, buflen-1);
      if (debug)
        printf("Method %0.2d, line %0.4d: getexecname=[%s]\n", ++method, __LINE__, buf);
    }
  }
#endif


#ifdef HAVE_GETMODULEFILENAME
  {
    TCHAR exeFileName[MAXPATHLEN] = {0};
    GetModuleFileName(NULL, exeFileName, MAXPATHLEN);
    memset(buf, 0, buflen);
    if (sizeof(TCHAR) == sizeof(char))
	    strncpy(buf, exeFileName, buflen-1);
    else
	    wcstombs(buf, exeFileName, wcslen(buf)+1);
    if (debug)
      printf("Method %0.2d, line %0.4d: GetModuleFileName=[%s]\n", ++method, __LINE__, buf);
  }
#endif


  /* verified, MacOSX */
#ifdef HAVE_PROC_PIDPATH
  {
    memset(buf, 0, buflen);
    (void)proc_pidpath(getpid(), buf, buflen);
    if (debug)
      printf("Method %0.2d, line %0.4d: proc_pidpath=[%s]\n", ++method, __LINE__, buf);
  }
#endif


  /* verified, Linux */
  /* relative path name */
#ifdef HAVE_DECL_PROGRAM_INVOCATION_NAME
  extern char *program_invocation_name;
  if (program_invocation_name) {
    strncpy(buf, program_invocation_name, buflen-1);
    if (debug)
      printf("Method %0.2d, line %0.4d: program_invocation_name=[%s]\n", ++method, __LINE__, buf);
  }
#endif


  /* verified, Linux */
  /* short name */
#ifdef HAVE_DECL_PROGRAM_INVOCATION_SHORT_NAME
  extern char *program_invocation_short_name;
  if (program_invocation_short_name) {
    strncpy(buf, program_invocation_short_name, buflen-1);
    if (debug)
      printf("Method %0.2d, line %0.4d: program_invocation_short_name=[%s]\n", ++method, __LINE__, buf);
  }
#endif


#ifdef HAVE_DECL___ARGV
  {
    const char *argv0 = buf;
    argv0 = __argv[0];
    strncpy(buf, argv0, buflen-1);
    if (debug)
      printf("Method %0.2d, line %0.4d: __argv=[%s]\n", ++method, __LINE__, buf);
  }
#endif


  /* verified, Linux, MacOSX */
  /* relative path name */
#ifdef HAVE_DECL___PROGNAME_FULL
  {
    const char *argv0 = buf;
    extern char *__progname_full;
    argv0 = __progname_full;
    strncpy(buf, argv0, buflen-1);
    if (debug)
      printf("Method %0.2d, line %0.4d: __progname_full=[%s]\n", ++method, __LINE__, buf);
  }
#endif


  /* verified, Linux, OpenBSD */
  /* short name */
#ifdef HAVE_DECL___PROGNAME
  {
    const char *argv0 = buf;
    extern char *__progname;
    argv0 = __progname;
    strncpy(buf, argv0, buflen-1);
    if (debug)
      printf("Method %0.2d, line %0.4d: __progname=[%s]\n", ++method, __LINE__, buf);
  }
#endif


  /* verified, FreeBSD */
#if defined(HAVE_DECL_CTL_KERN) && defined(HAVE_DECL_KERN_PROC) && defined(HAVE_DECL_KERN_PROC_PATHNAME)
  {
    int mib[4];
    size_t len = buflen;
    memset(buf, 0, buflen);
    mib[0] = CTL_KERN;  mib[1] = KERN_PROC;  mib[2] = KERN_PROC_PATHNAME;  mib[3] = -1;
    len = buflen;
    sysctl(mib, 4, buf, &len, NULL, 0);
    if (debug)
      printf("Method %0.2d, line %0.4d: sysctl(KERN_PROC)=[%s]\n", ++method, __LINE__, buf);
  }
#endif


#if defined(HAVE_DECL_CTL_KERN) && defined(HAVE_DECL_KERN_PROC_ARGS) && defined(HAVE_DECL_KERN_PROC_PATHNAME)
  {
    int mib[4];
    size_t len = buflen;
    memset(buf, 0, buflen);
    mib[0] = CTL_KERN;  mib[1] = KERN_PROC_ARGS;  mib[2] = -1;  mib[3] = KERN_PROC_PATHNAME;
    len = buflen;
    sysctl(mib, 4, buf, &len, NULL, 0);
    if (debug)
      printf("Method %0.2d, line %0.4d: sysctl(KERN_PROC_ARGS)=[%s]\n", ++method, __LINE__, buf);
  }
#endif


  /* verified, MacOSX */
  /* relative path name */
 #if defined(HAVE_DECL_CTL_KERN) && defined(HAVE_DECL_KERN_PROCARGS2)
  {
    int mib[4];
    size_t len;
    size_t size;
    int argmax;
    char *pbuf;
    memset(buf, 0, buflen);
    mib[0] = CTL_KERN;  mib[1] = KERN_ARGMAX; mib[2] = -1; mib[3] = -1;
    sysctl(mib, 2, &argmax, &len, NULL, 0);
    pbuf = calloc(argmax, sizeof(char));
    mib[0] = CTL_KERN;  mib[1] = KERN_PROCARGS2;  mib[2] = getpid();  mib[3] = -1;
    size = (size_t)argmax; // must be full size or sysctl returns nothing
    sysctl(mib, 3, pbuf, &size, NULL, 0);
    strncpy(buf, (pbuf+sizeof(int)), buflen); // from sysctl, exec_path comes after argc
    if (debug)
      printf("Method %0.2d, line %0.4d: sysctl(KERN_PROCARGS2)=[%s]\n", ++method, __LINE__, buf);
  }
#endif


  /* verified, MacOSX */
  /* short name */
#if defined(HAVE_DECL_CTL_KERN) && defined(HAVE_DECL_KERN_PROC) && defined(HAVE_DECL_KERN_PROCNAME)
  {
    int mib[4];
    size_t len = buflen;
    memset(buf, 0, buflen);
    mib[0] = CTL_KERN;  mib[1] = KERN_PROCNAME;  mib[2] = -1 ;  mib[3] = -1;
    len = buflen;
    sysctl(mib, 2, buf, &len, NULL, 0);
    if (debug)
      printf("Method %0.2d, line %0.4d: sysctl(KERN_PROCNAME)=[%s]\n", ++method, __LINE__, buf);
  }
#endif


  /* verified, OpenBSD */
  /* relative path name */
#if defined(HAVE_DECL_CTL_KERN) && defined(HAVE_DECL_KERN_PROC_ARGS) && defined(HAVE_DECL_KERN_PROC_ARGV)
  {
    int mib[4];
    char **retargs;
    size_t len = buflen;
    mib[0] = CTL_KERN;  mib[1] = KERN_PROC_ARGS;  mib[2] = getpid();  mib[3] = KERN_PROC_ARGV;
    sysctl(mib, 4, NULL, &len, NULL, 0);
    retargs = malloc(sizeof(char *) * len);
    sysctl(mib, 4, retargs, &len, NULL, 0);
    strncpy(buf, retargs[0], buflen-1);
    if (debug)
      printf("Method %0.2d, line %0.4d: sysctl(KERN_PROC_ARGV)=[%s]\n", ++method, __LINE__, buf);
    free(retargs);
  }
#endif


  /* verified, MacOSX */
  /* short name */
#if defined(HAVE_SYSCTLBYNAME)
  {
    size_t len = buflen;
    memset(buf, 0, buflen);
    len = buflen;
    sysctlbyname("kern.procname", buf, &len, NULL, 0);
    if (debug)
      printf("Method %0.2d, line %0.4d: sysctlbyname(kern.procname)=[%s]\n", ++method, __LINE__, buf);
  }
#endif


  /* verified, MacOSX */
#ifdef HAVE__NSGETEXECUTABLEPATH
  {
    uint32_t ulen = buflen;
    memset(buf, 0, buflen);
    _NSGetExecutablePath(buf, &ulen);
    if (debug)
      printf("Method %0.2d, line %0.4d: _NSGetExecutablePath=[%s]\n", ++method, __LINE__, buf);
  }
#endif


#ifdef HAVE_FIND_PATH
  {
    memset(buf, 0, buflen);
    find_path(B_APP_IMAGE_SYMBOL, B_FIND_PATH_IMAGE_PATH, NULL, buf, buflen);
    if (debug)
      printf("Method %0.2d, line %0.4d: find_path=[%s]\n", ++method, __LINE__, buf);
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
    memset(buf, 0, buflen);
    readlink("/proc/self/exe", buf, buflen);
    if (buf[0]) {
      if (debug)
        printf("Method %0.2d, line %0.4d: readlink(/proc/self/exe)=[%s]\n", ++method, __LINE__, buf);
    }
  }
#endif


#ifdef HAVE_READLINK
  {
    memset(buf, 0, buflen);
    readlink("/proc/curproc/file", buf, buflen);
    if (buf[0]) {
      if (debug)
        printf("Method %0.2d, line %0.4d: readlink(/proc/curproc/file)=[%s]\n", ++method, __LINE__, buf);
    }
  }
#endif


#ifdef HAVE_READLINK
  {
    memset(buf, 0, buflen);
    readlink("/proc/pinfo", buf, buflen);
    if (buf[0]) {
      if (debug)
        printf("Method %0.2d, line %0.4d: readlink(/proc/pinfo)=[%s]\n", ++method, __LINE__, buf);
    }
  }
#endif


#ifdef HAVE_READLINK
  {
    char pbuf[MAXPATHLEN] = {0};
    memset(buf, 0, buflen);
    snprintf(pbuf, MAXPATHLEN-1, "/proc/%d", getpid());
    readlink(pbuf, buf, buflen);
    if (buf[0]) {
      if (debug)
        printf("Method %0.2d, line %0.4d: readlink(%s)=[%s]\n", ++method, __LINE__, pbuf, buf);
    }
  }
#endif


#ifdef HAVE_READLINK
  {
    char pbuf[MAXPATHLEN] = {0};
    memset(buf, 0, buflen);
    snprintf(pbuf, MAXPATHLEN-1, "/proc/%d/cmdline", getpid());
    readlink(pbuf, buf, buflen-1);
    if (buf[0]) {
      if (debug)
        printf("Method %0.2d, line %0.4d: readlink(%s)=[%s]\n", ++method, __LINE__, pbuf, buf);
    }
  }
#endif


  /* verified, AIX */
  /* relative path on AIX */
#if defined(HAVE_READLINK) && defined(HAVE_SYS_PROCFS_H)
  {
    char pbuf[MAXPATHLEN] = {0};
    const char *argv0 = buf;
    struct psinfo p;
    int fd;
    memset(buf, 0, buflen);
    snprintf(pbuf, MAXPATHLEN-1, "/proc/%d/psinfo", getpid());
    fd = open(pbuf, O_RDONLY);
    read(fd, &p, sizeof(p));
    close(fd);
    argv0 = (*(char ***)((intptr_t)p.pr_argv))[0];
    if (argv0)
      memcpy(buf, argv0, strlen(argv0)+1);
    if (buf[0]) {
      if (debug)
        printf("Method %0.2d, line %0.4d: readlink(%s)=[%s]\n", ++method, __LINE__, pbuf, buf);
    }
  }
#endif


  /* verified, MacOSX */
  /* relative path on OBSD */
#ifdef HAVE_DLADDR
  {
    Dl_info i;
    dladdr(&main, &i);
    memset(buf, 0, buflen);
    strncpy(buf, i.dli_fname, buflen-1);
    if (debug)
      printf("Method %d: dladdr(main)=[%s]\n", ++method, __LINE__, buf);
  }
#endif


  /* verified, AIX */
  /* short name */
#ifdef HAVE_GETPROCS
  {
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
          strncpy(buf, argv0, buflen-1);
          if (debug)
            printf("Method %0.2d, line %0.4d: getprocs=[%s]\n", ++method, __LINE__, buf);
        }
      }
    }
  }
#endif


  /* verified, AIX */
  /* short name */
#ifdef HAVE_GETPROCS64
  {
    const char *argv0 = buf;
    struct procentry64 *pentry;
    int numproc;
    int index = 0;
    pid_t proc1 = (pid_t)0;

    // get up to 1M procs, same limit as IBM ps command
    int proccnt = getprocs64(NULL, 0, NULL, 0, &proc1, 1000000);
    pentry = malloc(sizeof(struct procentry64) * proccnt);
    while ((numproc = getprocs64(pentry, sizeof(struct procentry64), NULL, 0, &index, proccnt)) > 0) {
      for (int i = 0; i < numproc; i++) {
        if (pentry[i].pi_state == SZOMB)
                continue;
        if (getpid() == pentry[i].pi_pid) {
          argv0 = pentry[i].pi_comm;
          strncpy(buf, argv0, buflen-1);
          if (debug)
            printf("Method %0.2d, line %0.4d: getprocs64=[%s]\n", ++method, __LINE__, buf);
        }
      }
    }
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
