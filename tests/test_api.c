#include "progpath.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void) {
  char buf[1024];
  char *res;
  int failures = 0;

  /* Test progpath with sufficient buffer */
  memset(buf, 0, sizeof(buf));
  res = progpath(buf, sizeof(buf));
  if (!res) {
    fprintf(stderr, "FAIL: progpath returned NULL\n");
    failures++;
  } else if (strlen(buf) == 0) {
    fprintf(stderr, "FAIL: progpath returned empty string\n");
    failures++;
  } else {
    printf("PASS: progpath returned: %s\n", buf);
  }

  /* Test progpath with small buffer */
  /* Note: Behavior depends on implementation, but shouldn't crash */
  {
    char small_buf[1];
    small_buf[0] = 'X';
    res = progpath(small_buf, 0);
    /* We expect NULL or empty string if buffer is too small/invalid, 
       but primarily we want to ensure no crash */
    if (res && small_buf[0] != 'X' && small_buf[0] != '\0') {
        /* If it wrote something, it better be null terminated if size was > 0.
           With size 0, it shouldn't write. */
    }
    printf("PASS: progpath handled 0 size buffer without crash\n");
  }

  /* Test progipwd */
  {
    char *ipwd = progipwd(NULL, 0);
    if (!ipwd) {
      fprintf(stderr, "FAIL: progipwd(NULL, 0) returned NULL\n");
      failures++;
    } else {
      printf("PASS: progipwd returned: %s\n", ipwd);
      free(ipwd);
    }
  }

  /* Test PROGPATH_VERSION */
  #ifdef PROGPATH_VERSION
    if (strlen(PROGPATH_VERSION) > 0) {
        printf("PASS: PROGPATH_VERSION is set to \"%s\"\n", PROGPATH_VERSION);
    } else {
        fprintf(stderr, "FAIL: PROGPATH_VERSION is empty\n");
        failures++;
    }
  #else
    fprintf(stderr, "FAIL: PROGPATH_VERSION is not defined\n");
    failures++;
  #endif

  return failures > 0 ? 1 : 0;
}
