# Contributing to progpath

Here are the essentials. Thanks for helping!

## Reporting Bugs

Please include:
- Environment details (OS & compiler versions, hardware if relevant).
- Steps to reproduce

## Adding Support for New Platforms

To add a new platform-specific method:

1.  Update `CheckProgPath.cmake` to detect necessary header or function (e.g., `check_symbol_exists(my_func my_header.h HAVE_MY_FUNC)`).  Platform symbol assumptions (e.g., _WIN32) strongly discouraged.
2.  Add new block in the `PROGPATH_IMPLEMENTATION` section of `progpath.h.in`, e.g.:

```cpp
#ifdef HAVE_MY_FUNC
  {
    char mbuf[MAXPATHLEN] = {0};
    struct method m = METHOD("my_func");
    /* ... call API to get path into mbuf ... */
    my_func(mbuf);
    finalize(m, mbuf, MAXPATHLEN, NULL);
    if (we_done_yet(m, &buf, buflen, mbuf)) {
      chdir_if_diff(cwd);
      return buf;
    }
  }
#endif
```

The generated `progpath.h` is the public artifact.  Keep `progpath.cpp` as a
minimal shim only.

## Building and Testing

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```
