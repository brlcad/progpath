#define PROGPATH_IMPLEMENTATION
#include "progpath.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
#  include <direct.h>
#  define mkdir_compat(path) _mkdir(path)
#  define getcwd_compat _getcwd
#else
#  include <sys/stat.h>
#  include <unistd.h>
#  define mkdir_compat(path) mkdir(path, 0777)
#  define getcwd_compat getcwd
#endif

static int ensure_dir(const char *path) {
  errno = 0;
  if (mkdir_compat(path) == 0 || errno == EEXIST)
    return 0;

  std::fprintf(stderr, "FAIL: mkdir(%s) failed\n", path);
  return -1;
}

int main() {
  char start_cwd[4096] = {0};
  char ipwd[4096] = {0};
  char exe_path[4096] = {0};

  if (!getcwd_compat(start_cwd, sizeof(start_cwd))) {
    std::fprintf(stderr, "FAIL: getcwd() failed\n");
    return 1;
  }

  if (ensure_dir("single_header_cpp_dir") != 0)
    return 1;

  if (chdir("single_header_cpp_dir") != 0) {
    std::fprintf(stderr, "FAIL: chdir(single_header_cpp_dir) failed\n");
    return 1;
  }

  if (!progipwd(ipwd, sizeof(ipwd))) {
    std::fprintf(stderr, "FAIL: progipwd() returned NULL\n");
    return 1;
  }
  if (std::strcmp(ipwd, start_cwd) != 0) {
    std::fprintf(stderr, "FAIL: progipwd() [%s] != start cwd [%s]\n", ipwd, start_cwd);
    return 1;
  }

  if (!progpath(exe_path, sizeof(exe_path)) || !exe_path[0]) {
    std::fprintf(stderr, "FAIL: progpath() returned empty path\n");
    return 1;
  }

  if (chdir(start_cwd) != 0) {
    std::fprintf(stderr, "FAIL: chdir(start_cwd) failed\n");
    return 1;
  }

  std::printf("PASS: single-header C++ auto-init preserved initial cwd [%s]\n", ipwd);
  return 0;
}
