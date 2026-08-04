# integrating progpath

`progpath` is intentionally small and simple.  Just include the header
OR link against the compiled library.

## build

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

To build either integration form with a strict UTF-8 result contract:

```sh
cmake -S . -B build -DPROGPATH_UTF8=ON
```

This setting is recorded in the generated header and therefore applies
to both the compiled libraries and single-header implementations. It
does not change the API or ABI: `progpath()` and `progipwd()` still
return `char *`, with `len` measured in bytes.

In UTF-8 mode Windows paths are obtained as UTF-16 and converted to
UTF-8. Byte-oriented platforms validate native path bytes and reject a
path that is not valid UTF-8 with `errno == EILSEQ`; substituting or
locale-converting those bytes could identify a different file. An
undersized caller buffer returns `NULL` with an empty output buffer
rather than splitting a UTF-8 sequence. Configuration or compilation
fails when the target cannot provide the required guarantees.

For a manually configured single-header integration, defining
`PROGPATH_UTF8` before `PROGPATH_IMPLEMENTATION` selects the same mode.
The header's platform feature macros must come from CMake probes for the
actual target environment.

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
