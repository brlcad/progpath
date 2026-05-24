# integrating progpath

`progpath` is intentionally small and simple.  Just include the header
OR link against the compiled library.

## build

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## single-header

Use the generated `progpath.h` from the build tree or install tree.
Do not install or use the `progpath.h.in` template directly.

One translation unit must define `PROGPATH_IMPLEMENTATION`:

```cpp
/* progpath_impl.cpp */
#define PROGPATH_IMPLEMENTATION
#include "progpath.h"
```

Include `progpath.h` normally everywhere else for declarations.

If implementation is compiled as C++, there is automatic pre-`main()`
initialization.  Compiled as C, progpath emits a warning and falls
back to lazy first-call init; define `PROGPATH_NO_C_INIT_WARNING` to
suppress the first-call init warning.


## cmake package

After `cmake --install`, consumers can use:

```cmake
find_package(progpath REQUIRED)
target_link_libraries(myapp PRIVATE progpath::progpath)
```

Use `progpath::progpath-static` if you specifically want the static
library target.  On Windows, that target automatically adds
`PROGPATH_STATIC` for consumers.

## pkg-config package

After library install, make sure `pkg-config` can find `progpath.pc`
to then use:

```sh
pkg-config --cflags --libs progpath
```

## header vs library targets

If you embed the header and define `PROGPATH_IMPLEMENTATION`, you
don't need to do anything else.  If you use the library, link either
`progpath` or `progpath-static`.

The compiled library is backed by a tiny `progpath.cpp` shim that
defines `PROGPATH_IMPLEMENTATION` and includes the progpath.h header.

Header and library approaches both exercise the same implementation.

## installed docs

`make install` installs `README.md` and this file into the platform's
standard documentation directory for the package, along with a
`progpath(3)` man page.
