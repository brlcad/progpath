#ifndef PROGPATH_TEST_PATH_COMPARE_H
#define PROGPATH_TEST_PATH_COMPARE_H

/* CTest's PWD uses '/', while Windows getcwd() returns '\\'. */
static int test_paths_equal(const char *left, const char *right) {
  for (; *left && *right; ++left, ++right) {
    if (*left == *right)
      continue;
#ifdef _WIN32
    if ((*left == '/' || *left == '\\') && (*right == '/' || *right == '\\'))
      continue;
#endif
    return 0;
  }
  return *left == *right;
}

#endif
