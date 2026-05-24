#define PROGPATH_IMPLEMENTATION
#include "progpath.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#  include <direct.h>
#  define chdir _chdir
#else
#  include <unistd.h>
#endif

int main(int ac, char *av[]) {
  char *ipwd;
  char buf[1234] = {0};

  if (ac > 1) {
    printf("Usage: %s\n", av[0]);
    return 1;
  }

  /* more challenging */
  chdir("../../../..");

  /* pass a buffer */
  progpath(buf, sizeof(buf));
  if (!buf[0]) {
    fprintf(stderr,
            "ERROR: failed to get the program's path\n"
            "       (set PROGPATH_DEBUG=3 to debug)\n");
    return 2;
  }
  printf(" Program executable is [ %s ]\n", buf);

  /* or dynamically allocate */
  ipwd = progipwd(NULL, 0);
  if (!ipwd) {
    fprintf(stderr,
            "ERROR: failed to get initial path\n"
            "       (set PROGPATH_DEBUG=3 to debug)\n");
    return 1;
  }
  fprintf(stderr, "Initial working dir is [ %s ]\n", ipwd);
  free(ipwd);

  return 0;
}
