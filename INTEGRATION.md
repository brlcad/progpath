# integrating progpath

`progpath` is intentionally small and simple.  Main way to use it is
by including a single header OR link against a compiled library.

## Build

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Direct single-header use

Use the generated `progpath.h` from the build tree or install tree.
Do not install or use the `progpath.h.in` template directly.

One translation unit must define `PROGPATH_IMPLEMENTATION`:

```cpp
/* progpath_impl.cpp */
#define PROGPATH_IMPLEMENTATION
#include "progpath.h"
```

Include `progpath.h` normally everywhere else for declarations.

If that implementation translation unit is compiled as C++, automatic
pre-`main()` initialization is preserved.  If it is compiled as C,
progpath emits a compile-time warning and falls back to lazy first-call
initialization; define `PROGPATH_NO_C_INIT_WARNING` to suppress the warning
once that tradeoff is intentional.

## Installed CMake package

After `cmake --install`, consumers can use:

```cmake
find_package(progpath REQUIRED)
target_link_libraries(myapp PRIVATE progpath::progpath)
```

Use `progpath::progpath-static` if you specifically want the static target.
On Windows, that target automatically adds `PROGPATH_STATIC` for consumers.

## Installed pkg-config package

After install, make sure `pkg-config` can find `progpath.pc` and then use:

```sh
pkg-config --cflags --libs progpath
```

## In-tree library targets

If you bundle the source directly, add the project with `add_subdirectory(...)`
and link either `progpath` or `progpath-static`.

The compiled library is backed by a tiny `progpath.cpp` shim that
defines `PROGPATH_IMPLEMENTATION` and includes the generated header.
Library consumers and direct-header consumers exercise the same
implementation.

## Installed docs

`make install` also installs this file and `README.md` into the platform's
standard documentation directory for the package, along with a `progpath(3)`
man page when the platform defines a manpage install directory.
