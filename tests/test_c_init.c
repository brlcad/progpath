#define PROGPATH_NO_C_INIT_WARNING 1
#include "progpath.h"
#include "test_path_compare.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#  include <direct.h>
#  define chdir _chdir
#  define getcwd _getcwd
#else
#  include <unistd.h>
#endif

#ifndef MAXPATHLEN
#  define MAXPATHLEN 4096
#endif

int main(void) {
  char initial_expected[MAXPATHLEN] = {0};
  char *ipwd;

  if (!getcwd(initial_expected, sizeof(initial_expected))) {
    fprintf(stderr, "getcwd failed\n");
    return 1;
  }

  if (chdir("..") != 0) {
    fprintf(stderr, "chdir failed\n");
    return 1;
  }

  ipwd = progipwd(NULL, 0);
  if (!ipwd) {
    fprintf(stderr, "progipwd failed\n");
    return 1;
  }

  if (!test_paths_equal(initial_expected, ipwd)) {
    fprintf(stderr, "FAIL: Expected ipwd '%s', but got '%s'\n", initial_expected, ipwd);
    free(ipwd);
    return 1;
  }

  printf("PASS: ipwd matches '%s'\n", ipwd);
  free(ipwd);
  return 0;
}
