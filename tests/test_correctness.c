/*              T E S T _ C O R R E C T N E S S . C
 * progpath
 *
 * Verifies that progpath() and progipwd() return paths that are
 * actually correct, not just non-NULL non-empty.  Tests:
 *
 *   - returned path is absolute (starts with '/' on Unix, drive-letter on Windows)
 *   - returned path exists on disk
 *   - progpath() result contains this executable's own basename
 *   - where /proc/self/exe is available, cross-reference for exact match
 *   - progipwd() result is an existing directory
 */

#include "progpath.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#  include <io.h> /* _access */
#  include <windows.h>
#  define F_OK 0
#  define access _access
#else
#  include <sys/stat.h>
#  include <unistd.h>
#endif

#define BUFSIZE 4096

static int failures = 0;
static int passes = 0;

#define PASS(msg)                \
  do {                           \
    printf("PASS: %s\n", (msg)); \
    passes++;                    \
  } while (0)
#define FAIL(msg)                         \
  do {                                    \
    fprintf(stderr, "FAIL: %s\n", (msg)); \
    failures++;                           \
  } while (0)
#define FAILF(fmt, ...)                              \
  do {                                               \
    fprintf(stderr, "FAIL: " fmt "\n", __VA_ARGS__); \
    failures++;                                      \
  } while (0)

/* Returns 1 if 'path' looks like an absolute path on this platform. */
static int is_absolute(const char *path) {
  if (!path || path[0] == '\0')
    return 0;
  if (path[0] == '/')
    return 1;
  /* drive-letter path: C:\ or C:/ */
  if (path[1] == ':' && (path[2] == '\\' || path[2] == '/'))
    return 1;
  /* UNC path: \\server\share */
  if (path[0] == '\\' && path[1] == '\\')
    return 1;
  return 0;
}

/* Returns 1 if 'path' exists on disk (any type). */
static int path_exists(const char *path) {
  return access(path, F_OK) == 0;
}

#ifndef _WIN32
/* Returns 1 if 'path' is a directory. */
static int path_is_dir(const char *path) {
  struct stat st;
  return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}
#else
static int path_is_dir(const char *path) {
  DWORD attr = GetFileAttributesA(path);
  return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY);
}
#endif

/* Returns the basename portion of 'path' (final component after last separator). */
static const char *path_basename(const char *path) {
  const char *p = path;
  const char *last = path;
  while (*p) {
    if (*p == '/' || *p == '\\')
      last = p + 1;
    p++;
  }
  return last;
}

int main(void) {
  char pp[BUFSIZE] = {0};
  char ipwd[BUFSIZE] = {0};
  const char *myname;

  /* ------------------------------------------------------------------ */
  /* progpath() tests                                                     */
  /* ------------------------------------------------------------------ */

  if (!progpath(pp, sizeof(pp)) || pp[0] == '\0') {
    FAIL("progpath() returned NULL or empty string");
    /* remaining path tests would all fail, still run ipwd tests */
    goto test_ipwd;
  }

  /* 1. Must be absolute */
  if (is_absolute(pp)) {
    PASS("progpath() returns an absolute path");
  } else {
    FAILF("progpath() returned non-absolute path: \"%s\"", pp);
  }

  /* 2. Must exist on disk */
  if (path_exists(pp)) {
    PASS("progpath() path exists on disk");
  } else {
    FAILF("progpath() path does not exist: \"%s\"", pp);
  }

  /* 3. Basename must be "test_correctness" (our own executable name).
   *    On Windows the binary is "test_correctness.exe". */
  myname = path_basename(pp);
  if (strncmp(myname, "test_correctness", 16) == 0) {
    PASS("progpath() basename matches test executable name");
  } else {
    FAILF("progpath() basename \"%s\" does not match \"test_correctness\"", myname);
  }

  /* 4. Cross-reference with /proc/self/exe where available */
#ifndef _WIN32
  {
    char procpath[BUFSIZE] = {0};
    ssize_t len = 0;

    if (path_exists("/proc/self/exe")) {
      len = readlink("/proc/self/exe", procpath, sizeof(procpath) - 1);
    } else if (path_exists("/proc/curproc/file")) {
      len = readlink("/proc/curproc/file", procpath, sizeof(procpath) - 1);
    } else if (path_exists("/proc/curproc/exe")) {
      len = readlink("/proc/curproc/exe", procpath, sizeof(procpath) - 1);
    }

    if (len > 0) {
      procpath[len] = '\0';
      if (strcmp(pp, procpath) == 0) {
        PASS("progpath() matches /proc readlink result");
      } else {
        FAILF("progpath() \"%s\" != /proc readlink \"%s\"", pp, procpath);
      }
    } else {
      printf("SKIP: /proc readlink not available on this system\n");
    }
  }
#else
  printf("SKIP: /proc readlink not available on this system\n");
#endif

test_ipwd:
  /* ------------------------------------------------------------------ */
  /* progipwd() tests                                                     */
  /* ------------------------------------------------------------------ */

  if (!progipwd(ipwd, sizeof(ipwd)) || ipwd[0] == '\0') {
    FAIL("progipwd() returned NULL or empty string");
    goto done;
  }

  /* 5. Must be absolute */
  if (is_absolute(ipwd)) {
    PASS("progipwd() returns an absolute path");
  } else {
    FAILF("progipwd() returned non-absolute path: \"%s\"", ipwd);
  }

  /* 6. Must exist on disk */
  if (path_exists(ipwd)) {
    PASS("progipwd() path exists on disk");
  } else {
    FAILF("progipwd() path does not exist: \"%s\"", ipwd);
  }

  /* 7. Must be a directory */
  if (path_is_dir(ipwd)) {
    PASS("progipwd() path is a directory");
  } else {
    FAILF("progipwd() path is not a directory: \"%s\"", ipwd);
  }

done:
  printf("\n%d passed, %d failed\n", passes, failures);
  return failures > 0 ? 1 : 0;
}
