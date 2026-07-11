#include "progpath.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#  include <direct.h>
#  define getcwd _getcwd
#  define mkdir_compat(path) _mkdir(path)
#  define rmdir_compat(path) _rmdir(path)
#  define setenv_compat(name, value) _putenv_s(name, value)
#  define TEST_NAME "test_method_priority.exe"
#else
#  include <sys/stat.h>
#  include <unistd.h>
#  define mkdir_compat(path) mkdir(path, 0700)
#  define rmdir_compat(path) rmdir(path)
#  define setenv_compat(name, value) setenv(name, value, 1)
#  define TEST_NAME "test_method_priority"
#endif

#ifndef MAXPATHLEN
#  define MAXPATHLEN 4096
#endif

static int authoritative_provider_available(void) {
#if defined(HAVE_GETMODULEFILENAMEA) || defined(HAVE_PROC_PIDPATH) || defined(HAVE__NSGETEXECUTABLEPATH) ||   \
    defined(HAVE_FIND_PATH) ||                                                                                \
    (defined(HAVE_DECL_CTL_KERN) && defined(HAVE_DECL_KERN_PROC) && defined(HAVE_DECL_KERN_PROC_PATHNAME)) || \
    (defined(HAVE_DECL_CTL_KERN) && defined(HAVE_DECL_KERN_PROC_ARGS) && defined(HAVE_DECL_KERN_PROC_PATHNAME))
  return 1;
#elif defined(HAVE_UNISTD_H)
#  ifdef HAVE_READLINK
  char pid_path[MAXPATHLEN] = {0};
  const char *paths[] = {
      "/proc/self/exe",
      "/proc/curproc/file",
      "/proc/curproc/exe",
      "/proc/self/path/a.out",
      NULL,
  };
  int i;
  int pathlen;

  for (i = 0; paths[i]; i++) {
    if (access(paths[i], F_OK) == 0)
      return 1;
  }

  pathlen = snprintf(pid_path, sizeof(pid_path), "/proc/%d/file", (int)getpid());
  if (pathlen > 0 && pathlen < (int)sizeof(pid_path) && access(pid_path, F_OK) == 0)
    return 1;
  pathlen = snprintf(pid_path, sizeof(pid_path), "/proc/%d/path/a.out", (int)getpid());
  if (pathlen > 0 && pathlen < (int)sizeof(pid_path) && access(pid_path, F_OK) == 0)
    return 1;
#  endif
#  ifdef HAVE_READ
  if (access("/proc/self/exefile", F_OK) == 0)
    return 1;
#  endif
#endif

  return 0;
}

int main(void) {
  char cwd[MAXPATHLEN] = {0};
  char shadow_dir[MAXPATHLEN] = {0};
  char shadow_path[MAXPATHLEN] = {0};
  char result[MAXPATHLEN] = {0};
  FILE *fp;

  if (!getcwd(cwd, sizeof(cwd))) {
    fprintf(stderr, "FAIL: getcwd failed\n");
    return 1;
  }

  if (!authoritative_provider_available()) {
    printf("SKIP: no authoritative executable-path provider is available\n");
    return 0;
  }

  if (snprintf(shadow_dir, sizeof(shadow_dir), "%s/method_priority_shadow", cwd) >= (int)sizeof(shadow_dir) ||
      snprintf(shadow_path, sizeof(shadow_path), "%s/%s", shadow_dir, TEST_NAME) >= (int)sizeof(shadow_path)) {
    fprintf(stderr, "FAIL: test path is too long\n");
    return 1;
  }

  errno = 0;
  if (mkdir_compat(shadow_dir) != 0 && errno != EEXIST) {
    fprintf(stderr, "FAIL: could not create shadow directory\n");
    return 1;
  }

  fp = fopen(shadow_path, "wb");
  if (!fp) {
    fprintf(stderr, "FAIL: could not create shadow executable\n");
    return 1;
  }
  fclose(fp);

#ifndef _WIN32
  if (chmod(shadow_path, 0700) != 0) {
    fprintf(stderr, "FAIL: could not make shadow executable runnable\n");
    return 1;
  }
#endif

  if (setenv_compat("PATH", shadow_dir) != 0) {
    fprintf(stderr, "FAIL: could not set PATH\n");
    return 1;
  }

  if (!progpath(result, sizeof(result))) {
    fprintf(stderr, "FAIL: progpath returned NULL\n");
    return 1;
  }

  if (strstr(result, "method_priority_shadow")) {
    fprintf(stderr, "FAIL: PATH fallback shadowed the running executable: %s\n", result);
    return 1;
  }

  remove(shadow_path);
  rmdir_compat(shadow_dir);
  printf("PASS: authoritative provider won over PATH shadow: %s\n", result);
  return 0;
}
