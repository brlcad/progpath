#define PROGPATH_IMPLEMENTATION
#include <progpath.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
#  include <direct.h>
#  define chdir _chdir
#else
#  include <unistd.h>
#endif

int main() {
  char exe_path[4096] = {0};
  char ipwd[4096] = {0};

  if (chdir("..") != 0) {
    std::fprintf(stderr, "test_header_consumer: chdir(..) failed\n");
    return 1;
  }

  if (!progpath(exe_path, sizeof(exe_path)) || !exe_path[0]) {
    std::fprintf(stderr, "test_header_consumer: progpath() failed\n");
    return 1;
  }

  if (!progipwd(ipwd, sizeof(ipwd)) || !ipwd[0]) {
    std::fprintf(stderr, "test_header_consumer: progipwd() failed\n");
    return 1;
  }

  std::printf("Installed single-header test success!\n");
  std::printf("Path: %s\n", exe_path);
  std::printf("IPWD: %s\n", ipwd);
  return 0;
}
