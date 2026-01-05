# Contributing to progpath

Thanks for contributing! Here are the essentials.

## Reporting Bugs

Please include:
- Steps to reproduce.
- Expected vs. actual behavior.
- Environment details (OS, compiler).

## Pull Requests

1.  Run `clang-format -i` on modified files.
2.  Build and run tests locally before submitting.
3.  Use `pp_print` for debug logging, not `printf`.

## Building and Testing

```bash
mkdir build && cd build
cmake .. && cmake --build .
ctest --output-on-failure
```

## Adding Support for New Platforms

To add a new platform-specific method:

1.  **Detect Feature**: Update `CheckProgPath.cmake` to detect the necessary header or function (e.g., `check_symbol_exists(my_func my_header.h HAVE_MY_FUNC)`).
2.  **Implement Method**: Add a block in `progpath.cpp`:

```cpp
#ifdef HAVE_MY_FUNC
  {
    char mbuf[MAXPATHLEN] = {0};
    struct method m = METHOD("my_func");
    /* ... call API to get path into mbuf ... */
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, &buf, buflen, mbuf)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }
#endif
```
