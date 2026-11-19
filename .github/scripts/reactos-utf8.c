#include <stdio.h>
#include <windows.h>

int main(void) {
  /* The pinned VM's tar uses ANSI filenames; restore CTest's Unicode directory
   * with wide APIs after transferring it under an ASCII name. */
  const WCHAR staged_dir[] = L"tests\\utf8-staging";
  const WCHAR test_dir[] = L"tests\\utf8-\x6d4b\x8bd5";

  if (!MoveFileW(staged_dir, test_dir)) {
    fprintf(stderr, "ERROR: restoring Unicode test directory failed (%lu)\n", GetLastError());
    return 1;
  }
  return 0;
}
