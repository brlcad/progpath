#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
#  include <direct.h>
#  define mkdir_compat(path) _mkdir(path)
#  define setenv_compat(name, value) _putenv_s(name, value)
#  define unsetenv_compat(name) _putenv_s(name, "")
#else
#  include <sys/stat.h>
#  include <unistd.h>
#  define mkdir_compat(path) mkdir(path, 0777)
#  define setenv_compat(name, value) setenv(name, value, 1)
#  define unsetenv_compat(name) unsetenv(name)
#endif

#include "../progpath.cpp"

static int ensure_dir(const char *path) {
  errno = 0;
  if (mkdir_compat(path) == 0 || errno == EEXIST)
    return 0;

  std::fprintf(stderr, "FAIL: mkdir(%s) failed\n", path);
  return -1;
}

static int touch_file(const char *path) {
  FILE *fp = std::fopen(path, "wb");
  if (!fp) {
    std::fprintf(stderr, "FAIL: fopen(%s) failed\n", path);
    return -1;
  }

  std::fclose(fp);
  return 0;
}


static int join_path(char *dst, size_t dst_size, const char *lhs, const char *rhs) {
  size_t lhs_len = std::strlen(lhs);
  size_t rhs_len = std::strlen(rhs);

  if (lhs_len + 1 + rhs_len + 1 > dst_size) {
    std::fprintf(stderr, "FAIL: path too long [%s/%s]\n", lhs, rhs);
    return -1;
  }

  std::memcpy(dst, lhs, lhs_len);
  dst[lhs_len] = '/';
  std::memcpy(dst + lhs_len + 1, rhs, rhs_len + 1);
  return 0;
}


int main() {
#if !defined(HAVE_REALPATH) || !defined(HAVE_UNISTD_H)
  std::puts("SKIP: explicit relative-path hardening requires realpath() and POSIX PATH handling");
  return 0;
#else
  char saved_cwd[MAXPATHLEN] = {0};
  char base_dir[MAXPATHLEN] = {0};
  char real_dir[MAXPATHLEN] = {0};
  char real_subdir[MAXPATHLEN] = {0};
  char real_tool[MAXPATHLEN] = {0};
  char shadow_dir[MAXPATHLEN] = {0};
  char shadow_subdir[MAXPATHLEN] = {0};
  char shadow_tool[MAXPATHLEN] = {0};
  char resolved[MAXPATHLEN] = {0};
  char expected[MAXPATHLEN] = {0};
  char *saved_path = NULL;
  const char *current_path = std::getenv("PATH");
  int ret = 1;

  if (!getcwd(saved_cwd, sizeof(saved_cwd))) {
    std::fprintf(stderr, "FAIL: getcwd() failed\n");
    return 1;
  }

  if (current_path) {
    saved_path = strdup(current_path);
    if (!saved_path) {
      std::fprintf(stderr, "FAIL: strdup(PATH) failed\n");
      return 1;
    }
  }

  if (join_path(base_dir, sizeof(base_dir), saved_cwd, "resolve_path_test") != 0 ||
      join_path(real_dir, sizeof(real_dir), base_dir, "real") != 0 ||
      join_path(real_subdir, sizeof(real_subdir), real_dir, "subdir") != 0 ||
      join_path(real_tool, sizeof(real_tool), real_subdir, "tool") != 0 ||
      join_path(shadow_dir, sizeof(shadow_dir), base_dir, "shadow") != 0 ||
      join_path(shadow_subdir, sizeof(shadow_subdir), shadow_dir, "subdir") != 0 ||
      join_path(shadow_tool, sizeof(shadow_tool), shadow_subdir, "tool") != 0)
    goto cleanup;

  if (ensure_dir(base_dir) != 0 || ensure_dir(real_dir) != 0 || ensure_dir(real_subdir) != 0 ||
      ensure_dir(shadow_dir) != 0 || ensure_dir(shadow_subdir) != 0)
    goto cleanup;

  if (touch_file(real_tool) != 0 || touch_file(shadow_tool) != 0)
    goto cleanup;

  if (chdir(real_dir) != 0) {
    std::fprintf(stderr, "FAIL: chdir(%s) failed\n", real_dir);
    goto cleanup;
  }

  if (setenv_compat("PATH", shadow_dir) != 0) {
    std::fprintf(stderr, "FAIL: set PATH failed\n");
    goto cleanup;
  }

  std::strncpy(resolved, "subdir/tool", sizeof(resolved) - 1);
  resolve_to_full_path(real_dir, resolved, sizeof(resolved));

  if (!realpath("subdir/tool", expected)) {
    std::fprintf(stderr, "FAIL: realpath(subdir/tool) failed\n");
    goto cleanup;
  }

  if (std::strcmp(resolved, expected) != 0) {
    std::fprintf(stderr, "FAIL: resolved [%s] expected [%s]\n", resolved, expected);
    goto cleanup;
  }

  std::printf("PASS: explicit relative paths resolve before PATH search [%s]\n", resolved);
  ret = 0;

cleanup:
  if (saved_path) {
    setenv_compat("PATH", saved_path);
    free(saved_path);
  } else {
    unsetenv_compat("PATH");
  }

  chdir(saved_cwd);
  return ret;
#endif
}
