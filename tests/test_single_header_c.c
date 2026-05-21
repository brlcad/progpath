#define PROGPATH_NO_C_INIT_WARNING
#define PROGPATH_IMPLEMENTATION
#include "progpath.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#  include <direct.h>
#  define mkdir_compat(path) _mkdir(path)
#  define getcwd_compat _getcwd
#  define unsetenv_compat(name) _putenv_s(name, "")
#else
#  include <sys/stat.h>
#  include <unistd.h>
#  define mkdir_compat(path) mkdir(path, 0777)
#  define getcwd_compat getcwd
#  define unsetenv_compat(name) unsetenv(name)
#endif

static int ensure_dir(const char *path) {
  errno = 0;
  if (mkdir_compat(path) == 0 || errno == EEXIST)
    return 0;

  fprintf(stderr, "FAIL: mkdir(%s) failed\n", path);
  return -1;
}

int main(void) {
  char start_cwd[4096] = {0};
  char lazy_cwd[4096] = {0};
  char ipwd[4096] = {0};
  char exe_path[4096] = {0};

  if (!getcwd_compat(start_cwd, sizeof(start_cwd))) {
    fprintf(stderr, "FAIL: getcwd() failed\n");
    return 1;
  }

  if (ensure_dir("single_header_c_dir") != 0)
    return 1;

  unsetenv_compat("PWD");

  if (chdir("single_header_c_dir") != 0) {
    fprintf(stderr, "FAIL: chdir(single_header_c_dir) failed\n");
    return 1;
  }

  if (!getcwd_compat(lazy_cwd, sizeof(lazy_cwd))) {
    fprintf(stderr, "FAIL: getcwd() after chdir failed\n");
    return 1;
  }

  if (!progipwd(ipwd, sizeof(ipwd))) {
    fprintf(stderr, "FAIL: progipwd() returned NULL\n");
    return 1;
  }
  if (strcmp(ipwd, lazy_cwd) != 0) {
    fprintf(stderr, "FAIL: progipwd() [%s] != lazy cwd [%s]\n", ipwd, lazy_cwd);
    return 1;
  }
  if (strcmp(ipwd, start_cwd) == 0) {
    fprintf(stderr, "FAIL: progipwd() unexpectedly captured the pre-main cwd\n");
    return 1;
  }

  if (!progpath(exe_path, sizeof(exe_path)) || !exe_path[0]) {
    fprintf(stderr, "FAIL: progpath() returned empty path\n");
    return 1;
  }

  if (chdir(start_cwd) != 0) {
    fprintf(stderr, "FAIL: chdir(start_cwd) failed\n");
    return 1;
  }

  printf("PASS: pure C single-header build uses documented lazy init [%s]\n", ipwd);
  return 0;
}
