#ifdef PROGPATH_UTF8_HEADER_ONLY
#  define PROGPATH_NO_C_INIT_WARNING 1
#  define PROGPATH_IMPLEMENTATION
#endif

#include "progpath.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef PROGPATH_UTF8
#  error "test_utf8 must be compiled with PROGPATH_UTF8 enabled"
#endif

static int valid_utf8(const char *str) {
  const unsigned char *s = (const unsigned char *)str;

  if (!s)
    return 0;

  while (*s) {
    if (*s <= 0x7f) {
      s++;
    } else if (*s >= 0xc2 && *s <= 0xdf) {
      if (s[1] < 0x80 || s[1] > 0xbf)
        return 0;
      s += 2;
    } else if (*s == 0xe0) {
      if (s[1] < 0xa0 || s[1] > 0xbf || s[2] < 0x80 || s[2] > 0xbf)
        return 0;
      s += 3;
    } else if ((*s >= 0xe1 && *s <= 0xec) || (*s >= 0xee && *s <= 0xef)) {
      if (s[1] < 0x80 || s[1] > 0xbf || s[2] < 0x80 || s[2] > 0xbf)
        return 0;
      s += 3;
    } else if (*s == 0xed) {
      if (s[1] < 0x80 || s[1] > 0x9f || s[2] < 0x80 || s[2] > 0xbf)
        return 0;
      s += 3;
    } else if (*s == 0xf0) {
      if (s[1] < 0x90 || s[1] > 0xbf || s[2] < 0x80 || s[2] > 0xbf ||
          s[3] < 0x80 || s[3] > 0xbf)
        return 0;
      s += 4;
    } else if (*s >= 0xf1 && *s <= 0xf3) {
      if (s[1] < 0x80 || s[1] > 0xbf || s[2] < 0x80 || s[2] > 0xbf ||
          s[3] < 0x80 || s[3] > 0xbf)
        return 0;
      s += 4;
    } else if (*s == 0xf4) {
      if (s[1] < 0x80 || s[1] > 0x8f || s[2] < 0x80 || s[2] > 0xbf ||
          s[3] < 0x80 || s[3] > 0xbf)
        return 0;
      s += 4;
    } else {
      return 0;
    }
  }

  return 1;
}

static int check_path(const char *label, const char *path, const char *marker) {
  if (!path) {
    fprintf(stderr, "FAIL: %s returned NULL\n", label);
    return 1;
  }
  if (!valid_utf8(path)) {
    fprintf(stderr, "FAIL: %s returned invalid UTF-8\n", label);
    return 1;
  }
  if (!strstr(path, marker)) {
    fprintf(stderr, "FAIL: %s did not preserve the UTF-8 directory marker: %s\n", label, path);
    return 1;
  }

  printf("PASS: %s returned valid UTF-8: %s\n", label, path);
  return 0;
}

int main(void) {
  static const char marker[] = "\xe6\xb5\x8b\xe8\xaf\x95"; /* U+6D4B U+8BD5 */
#ifdef PROGPATH_UTF8_HEADER_ONLY
  static const char overlong[] = "\xc0\xaf";
  static const char surrogate[] = "\xed\xa0\x80";
  static const char too_large[] = "\xf4\x90\x80\x80";
  static const char invalid_path[] = "/\xff";
#  ifdef HAVE_WINDOWS_H
  static const WCHAR surrogate_pair[] = {(WCHAR)0xd801, (WCHAR)0xdc37, 0};
  static const WCHAR lone_surrogate[] = {(WCHAR)0xd800, 0};
  static const char supplementary_utf8[] = "\xf0\x90\x90\xb7"; /* U+10437 */
#  endif
#endif
  char *path = progpath(NULL, 0);
  char *ipwd = progipwd(NULL, 0);
  const char *marker_at;
  int failures = 0;

  failures += check_path("progpath()", path, marker);
  failures += check_path("progipwd()", ipwd, marker);

#ifdef PROGPATH_UTF8_HEADER_ONLY
  if (is_valid_utf8(marker) && !is_valid_utf8(overlong) &&
      !is_valid_utf8(surrogate) && !is_valid_utf8(too_large)) {
    printf("PASS: strict UTF-8 validation rejects invalid scalar encodings\n");
  } else {
    fprintf(stderr, "FAIL: strict UTF-8 validation accepted an invalid encoding\n");
    failures++;
  }
  {
    char rejected[8] = {0};
    char *rejected_ptr = rejected;
    struct method m = {0, 0, "invalid UTF-8 test", 0};
    errno = 0;
    if (we_done_yet(m, &rejected_ptr, sizeof(rejected), invalid_path) ||
        rejected[0] != '\0' || errno != EILSEQ) {
      fprintf(stderr, "FAIL: invalid native path was not rejected with EILSEQ\n");
      failures++;
    } else {
      printf("PASS: invalid native path is rejected with EILSEQ\n");
    }
  }
#  ifdef HAVE_WINDOWS_H
  {
    char *converted = wide_to_utf8(surrogate_pair);
    char *rejected = wide_to_utf8(lone_surrogate);
    if (!converted || strcmp(converted, supplementary_utf8) != 0 || rejected) {
      fprintf(stderr, "FAIL: strict Windows UTF-16 conversion is incorrect\n");
      failures++;
    } else {
      printf("PASS: Windows surrogate pairs convert strictly to UTF-8\n");
    }
    free(converted);
    free(rejected);
  }
#  endif
#endif

  marker_at = path ? strstr(path, marker) : NULL;
  if (marker_at) {
    size_t short_len = (size_t)(marker_at - path) + 2;
    char *short_buf = (char *)malloc(short_len);
    if (!short_buf) {
      fprintf(stderr, "FAIL: could not allocate truncation test buffer\n");
      failures++;
    } else {
      memset(short_buf, 'X', short_len);
      if (progpath(short_buf, short_len) != NULL || short_buf[0] != '\0') {
        fprintf(stderr, "FAIL: progpath() returned a truncated UTF-8 path\n");
        failures++;
      } else {
        printf("PASS: progpath() rejects buffers that would split UTF-8 output\n");
      }
      free(short_buf);
    }
  }

  marker_at = ipwd ? strstr(ipwd, marker) : NULL;
  if (marker_at) {
    size_t short_len = (size_t)(marker_at - ipwd) + 2;
    char *short_buf = (char *)malloc(short_len);
    if (!short_buf) {
      fprintf(stderr, "FAIL: could not allocate cwd truncation test buffer\n");
      failures++;
    } else {
      memset(short_buf, 'X', short_len);
      if (progipwd(short_buf, short_len) != NULL || short_buf[0] != '\0') {
        fprintf(stderr, "FAIL: progipwd() returned a truncated UTF-8 path\n");
        failures++;
      } else {
        printf("PASS: progipwd() rejects buffers that would split UTF-8 output\n");
      }
      free(short_buf);
    }
  }

  free(path);
  free(ipwd);
  return failures ? 1 : 0;
}
