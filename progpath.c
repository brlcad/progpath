
#include "progpath_config.h"

#define _GNU_SOURCE 1

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif
#ifdef HAVE_SYS_PARAM_H /* for MAXPATHLEN */
#  include <sys/param.h>
#endif
#ifdef HAVE_SYS_WAIT_H /* for wait */
#  include <sys/wait.h>
#endif
#ifdef HAVE_SYS_SYSCTL_H
#  include <sys/sysctl.h>
#endif
#ifdef HAVE_SYS_PROCFS_H
#  include <sys/procfs.h>
#endif
#ifdef HAVE_DLFCN_H
#  include <dlfcn.h>
#endif
#ifdef HAVE_PROCINFO_H
#  include <procinfo.h>
#endif

#ifdef HAVE_WINDOWS_H
#  include <windows.h>
#endif

#ifdef HAVE_MACH_O_DYLD_H
#  include <mach-o/dyld.h>
#endif

#ifdef HAVE_FINDDIRECTORY_H
#  include <FindDirectory.h>
#endif


extern const char *getprogname(void);
extern const char *getexecname(void);
extern int getpid(void);
extern void proc_pidpath(int, char *, size_t);
/* or ssize_t return */
extern int readlink(const char *, char *, size_t);



#ifndef MAXPATHLEN
#  define MAXPATHLEN 1024
#endif


int main(int ac, char *av[]) {
  const char *argv0 = NULL;
  char buf[MAXPATHLEN] = {0};

  if (ac > 1) {
    printf("Usage: %s\n", av[0]);
    return 1;
  }
  
  /* verified, MacOSX, OpenBSD */
  /* short name */
#ifdef HAVE_GETPROGNAME
  /* BSD's libc provides a way */
  argv0 = getprogname(); /* not malloc'd memory, may return NULL */
  printf("Method 1: getprogname=[%s]\n", argv0);
#endif

#ifdef HAVE_GETEXECNAME
  argv0 = getexecname();
  printf("Method 2: getexecname=[%s]\n", argv0);
#endif

#ifdef HAVE_GETMODULEFILENAME
  {
    TCHAR exeFileName[MAXPATHLEN] = {0};
    GetModuleFileName(NULL, exeFileName, MAXPATHLEN);
    if (sizeof(TCHAR) == sizeof(char))
	    bu_strlcpy(buf, exeFileName, MAXPATHLEN);
    else
	    wcstombs(buf, exeFileName, wcslen(buf)+1);
    printf("Method 3: GetModuleFileName=[%s]\n", buf);
  }
#endif

    /* verified, MacOSX */
#ifdef HAVE_PROC_PIDPATH
  {
    int pid = getpid();
    (void)proc_pidpath(pid, buf, sizeof(buf));
    printf("Method 4: proc_pidpath=[%s]\n", buf);
  }
#endif

    /* verified, Linux */
    /* relative path name */
#ifdef HAVE_DECL_PROGRAM_INVOCATION_NAME
  extern char *program_invocation_name;
  if (program_invocation_name)
    printf("Method 5: program_invocation_name=[%s]\n", program_invocation_name);
#endif

    /* verified, Linux */
    /* short name */
#ifdef HAVE_DECL_PROGRAM_INVOCATION_SHORT_NAME
  extern char *program_invocation_short_name;
  if (program_invocation_short_name)
    printf("Method 5a: program_invocation_short_name=[%s]\n", program_invocation_short_name);
#endif

#ifdef HAVE_DECL___ARGV
  argv0 = __argv[0];
  printf("Method 6: __argv=[%s]\n", argv0);
#endif

    /* verified, Linux, MacOSX */
    /* relative path name */
#ifdef HAVE_DECL___PROGNAME_FULL
  extern char *__progname_full;
  argv0 = __progname_full;
  printf("Method 7: __progname_full=[%s]\n", argv0);
#endif

    /* verified, Linux, OpenBSD */
    /* short name */
#ifdef HAVE_DECL___PROGNAME
  extern char *__progname;
  argv0 = __progname;
  printf("Method 7a: __progname=[%s]\n", argv0);
#endif

#if defined(HAVE_SYSCTL)
  {
    int mib[4];
    size_t len = sizeof(buf);

    /* verified, FreeBSD */
#  if defined(HAVE_DECL_CTL_KERN) && defined(HAVE_DECL_KERN_PROC) && defined(HAVE_DECL_KERN_PROC_PATHNAME)
    memset(buf, 0, sizeof(buf));
    mib[0] = CTL_KERN;  mib[1] = KERN_PROC;  mib[2] = KERN_PROC_PATHNAME;  mib[3] = -1;
    sysctl(mib, 4, buf, &len, NULL, 0);
    printf("Method 8: sysctl(KERN_PROC)=[%s]\n", buf);
#  endif

#  if defined(HAVE_DECL_CTL_KERN) && defined(HAVE_DECL_KERN_PROC_ARGS) && defined(HAVE_DECL_KERN_PROC_PATHNAME)
    memset(buf, 0, sizeof(buf));
    mib[0] = CTL_KERN;  mib[1] = KERN_PROC_ARGS;  mib[2] = -1;  mib[3] = KERN_PROC_PATHNAME;
    sysctl(mib, 4, buf, &len, NULL, 0);
    printf("Method 8a: sysctl(KERN_PROC_ARGS)=[%s]\n", buf);
#  endif

    /* verified, MacOSX */
    /* short name */
#  if defined(HAVE_DECL_CTL_KERN) && defined(HAVE_DECL_KERN_PROC) && defined(KERN_PROCNAME)
    memset(buf, 0, sizeof(buf));
    mib[0] = CTL_KERN;  mib[1] = KERN_PROCNAME;  mib[2] = -1 ;  mib[3] = -1;
    sysctl(mib, 2, buf, &len, NULL, 0);
    printf("Method 8b: sysctl(KERN_PROCNAME)=[%s]\n", buf);
#  endif

    /* verified, OpenBSD */
    /* relative path name */
#  if defined(HAVE_DECL_CTL_KERN) && defined(HAVE_DECL_KERN_PROC_ARGS) && defined(HAVE_DECL_KERN_PROC_ARGV)
    {
      char **retargs;
      memset(buf, 0, sizeof(buf));
      mib[0] = CTL_KERN;  mib[1] = KERN_PROC_ARGS;  mib[2] = getpid();  mib[3] = KERN_PROC_ARGV;
      sysctl(mib, 4, NULL, &len, NULL, 0);
      retargs = malloc(sizeof(char *) * len);
      sysctl(mib, 4, retargs, &len, NULL, 0);
      printf("Method 8c: sysctl(KERN_PROC_ARGV)=[%s]\n", retargs[0]);
      free(retargs);
    }
#  endif

  }
#endif

    /* verified, MacOSX */
    /* short name */
#if defined(HAVE_SYSCTLBYNAME)
  {
    size_t len = sizeof(buf);
    sysctlbyname("kern.procname", buf, &len, NULL, 0);
    printf("Method 9: sysctlbyname(kern.procname)=[%s]\n", buf);
  }
#endif

    /* verified, MacOSX */
#ifdef HAVE__NSGETEXECUTABLEPATH
  {
    uint32_t len = sizeof(buf);
    _NSGetExecutablePath(buf, &len);
    printf("Method 10: _NSGetExecutablePath=[%s]\n", buf);
  }
#endif

#ifdef HAVE_FIND_PATH
  find_path(B_APP_IMAGE_SYMBOL, B_FIND_PATH_IMAGE_PATH, NULL, buf, sizeof(buf));
  printf("Method 11: find_path=[%s]\n", buf);
#endif

  /* PID-based methods */

  // IRIX: /proc/pinfo
  // LINUX: /proc/self/exe
  // OBSD: /proc/curproc/file
  // OSF: /proc/%d
  // SYSV: /proc/%d/cmdline
#ifdef HAVE_READLINK
  /* verified, Linux */
  memset(buf, 0, sizeof(buf));
  readlink("/proc/self/exe", buf, sizeof(buf));
  if (buf[0])
    printf("Method 11: readlink(/proc/self/exe)=[%s]\n", buf);

  memset(buf, 0, sizeof(buf));
  readlink("/proc/curproc/file", buf, sizeof(buf));
  if (buf[0])
    printf("Method 11a: readlink(/proc/curproc/file)=[%s]\n", buf);

  memset(buf, 0, sizeof(buf));
  readlink("/proc/pinfo", buf, sizeof(buf));
  if (buf[0])
    printf("Method 11b: readlink(/proc/pinfo)=[%s]\n", buf);

  {
    char pbuf[MAXPATHLEN] = {0};

    memset(buf, 0, sizeof(buf));
    snprintf(pbuf, sizeof(pbuf), "/proc/%d", getpid());
    readlink(pbuf, buf, sizeof(buf));
    if (buf[0])
      printf("Method 11c: readlink(%s)=[%s]\n", pbuf, buf);

    memset(buf, 0, sizeof(buf));
    snprintf(pbuf, sizeof(pbuf), "/proc/%d/cmdline", getpid());
    readlink(pbuf, buf, sizeof(buf));
    if (buf[0])
      printf("Method 11d: readlink(%s)=[%s]\n", pbuf, buf);

    /* verified, AIX */
    /* relative path on AIX */
    {
      struct psinfo p;
      int fd;

      memset(buf, 0, sizeof(buf));
      snprintf(pbuf, sizeof(pbuf), "/proc/%d/psinfo", getpid());
      fd = open(pbuf, O_RDONLY);
      read(fd, &p, sizeof(p));
      close(fd);
      argv0 = (*(char ***)((intptr_t)p.pr_argv))[0];
      if (argv0)
	memcpy(buf, argv0, strlen(argv0)+1);
      if (buf[0])
	printf("Method 11e: readlink(%s)=[%s]\n", pbuf, buf);
    }
  }
#endif

  /* verified, MacOSX */
  /* relative path on OBSD */
#ifdef HAVE_DLADDR
  {
    Dl_info i;
    dladdr(&main, &i);
    printf("Method 12: dladdr(main)=[%s]\n", i.dli_fname);
  }
#endif

  /* verified, AIX */
  /* short name */
#ifdef HAVE_GETPROCS
  {
    struct procsinfo pinfo[16];
    int numproc;
    int index = 0;
    while ((numproc = getprocs(pinfo, sizeof(struct procsinfo), NULL, 0, &index, 16)) > 0) {
      for (int i = 0; i < numproc; i++) {
        if (pinfo[i].pi_state == SZOMB)
                continue;
        if (getpid() == (pid_t)pinfo[i].pi_pid) {
          argv0 = pinfo[i].pi_comm;
          printf("Method 13: getprocs=[%s]\n", argv0);
        }
      }
    }
  }
#endif

  /* verified, AIX */
  /* short name */
#ifdef HAVE_GETPROCS64
  {
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
          printf("Method 14: getprocs64=[%s]\n", argv0);
        }
      }
    }
  }
#endif


  // HPUX64: pstat_getproc64()
  // HPUX: pstat_getproc()
  // IBM: w_getpsent()

  /* TODO: PATH-based methods */

  printf("Done\n");

  return 0;
}
