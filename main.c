
#include "progpath.h"

#include <stdio.h>
#include <stdlib.h>

/* platform symbols only to prove progpath_config.h is not required */
#ifdef _WIN32
#  include <windows.h> /* for _chdir */
#  define chdir _chdir
#else
#  include <unistd.h> /* for chdir */
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

  /* allocate dynamically */
  ipwd = progipwd(NULL, 0);
  if (!ipwd) {
    fprintf(stderr,
            "ERROR: failed to get initial path\n"
            "       (set PROGPATH_DEBUG=3 to debug)\n");
    return 1;
  }
  fprintf(stderr, "Initial directory is %s\n", ipwd);
  free(ipwd);

  /* or pass a buffer */
  progpath(buf, sizeof(buf));
  if (!buf[0]) {
    fprintf(stderr,
            "ERROR: failed to get the program's path\n"
            "       (set PROGPATH_DEBUG=3 to debug)\n");
    return 2;
  }

  printf("%s\n", buf);
  return 0;
}

