# progpath
[![Release Build and Install](https://github.com/brlcad/progpath/actions/workflows/release.yml/badge.svg)](https://github.com/brlcad/progpath/actions/workflows/release.yml)

tiny C/C++ library for getting initial paths for a running
application, encapsulating platform-specific details for getting a
path to the running executable or its initial working dir.  where
progpath differs from other efforts:

1. absolute API simplicity,
2. trivial build integration,
3. number of methods/environments,
4. ease adding new methods, and
5. works after changing dirs!

## primary integration: configured single-header

`progpath` now generates one self-contained `progpath.h` that embeds:

- version macros
- configure-time `HAVE_*` probe results
- public declarations
- implementation under `PROGPATH_IMPLEMENTATION`

The vendorable artifact is the generated `progpath.h` from your build or install
tree, not the checked-in `progpath.h.in` template.

Typical flow:

1. `cmake -S . -B build`
2. vendor `build/progpath.h` into your tree, or install and copy the installed header
3. define `PROGPATH_IMPLEMENTATION` in exactly one implementation translation unit
4. include `progpath.h` normally everywhere else

### one translation unit (C++)

```cpp
#define PROGPATH_IMPLEMENTATION
#include "progpath.h"

#include <cstdio>
#include <cstdlib>

int main() {
  char pp[4096];
  char *ipwd;

  progpath(pp, sizeof(pp));
  std::printf(" Program executable is [ %s ]\n", pp);

  ipwd = progipwd(NULL, 0);
  std::printf("Initial working dir is [ %s ]\n", ipwd);
  std::free(ipwd);
  return 0;
}
```

### split translation units (C caller + C++ implementation)

```cpp
/* progpath_impl.cpp */
#define PROGPATH_IMPLEMENTATION
#include "progpath.h"
```

```c
/* main.c */
#include "progpath.h"

int main(void) {
  char pp[4096];
  progpath(pp, sizeof(pp));
  return 0;
}
```

Compile the implementation translation unit as C++ if you want automatic
pre-`main()` initialization.

## build and run the bundled demo

```shell
     % git clone https://github.com/brlcad/progpath.git && cd progpath
     % cmake -S . -B build && cmake --build build
     % cd ..  # change dir just for fun, then run 'progpath' binary
     % progpath/build/progpath
     Program executable is [ /Users/morrison/progpath/build/progpath ]
    Initial working dir is [ /Users/morrison ]
```

## packaged library integration

The compiled shared/static targets remain supported for projects that prefer
package-manager or `find_package` integration.

After `cmake --install`, callers can use:

- CMake: `find_package(progpath REQUIRED)` then link `progpath::progpath`
- CMake static: link `progpath::progpath-static`
- `pkg-config`: `pkg-config --cflags --libs progpath`

Find more detailed notes in [INTEGRATION.md](INTEGRATION.md).

## initialization

progpath captures the initial working directory (ipwd) once, as early
as possible, so `progipwd()` can return it correctly even after
`chdir()`.  How that capture happens depends on how you build the
implementation:

| Build mode | Init behavior |
|---|---|
| Generated `progpath.h` implementation compiled as **C++** | **Automatic** — C++ static constructor fires before `main()`. Nothing to do. |
| Generated `progpath.h` implementation compiled as **C** | **Lazy** — first call captures state. A compile-time warning is emitted unless `PROGPATH_NO_C_INIT_WARNING` is defined. Call `progipwd()` or `progpath()` early, before any explicit `chdir()`. |
| Shared library (`.so`/`.dylib`/`.dll`) | **Automatic** — C++ static constructor fires before `main()`. Nothing to do. |
| Static lib, linked from **C++** | **Automatic** — C++ runtime fires the constructor. |
| Static lib, linked from **pure C** | **Manual** — initial path is captured on first call to `progipwd()` or `progpath()`. Call either early, before any `chdir()`.|

## notes and limitations

- `progpath.h` is configured for a specific target environment. Re-run CMake to generate it for the platform you intend to ship.
- For cross-compiles or redistributed artifacts, generate the header using the target toolchain and target feature probes.
- Thread safety: do not call `progpath()` or `progipwd()` concurrently before first capture completes. Initialize from the main thread before spawning other threads.
- Some lookup methods temporarily restore the initial working directory while resolving the executable path. Avoid concurrent directory changes while using the API.

progpath is library API but includes an example 'progpath' program for
testing that should work everywhere.  [let me
know](https://github.com/brlcad/progpath/issues) if you find an
environment that doesn't work!

## platform CI status matrix

following environments are continuously tested; badges reflect current status for each specific platform:

| OS Group | Environment | Architecture | Status |
|---|---|---|---|
| **Windows** | Windows Server 2022 | x86_64 | [![Windows 2022](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=Windows%20%28windows-2022%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |
| **macOS** | macOS 14 | arm64 | [![macOS 14](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=macOS%20%28macos-14%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |
| **Linux** | Ubuntu | x86_64 | [![Ubuntu](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=Linux%20%28Ubuntu%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |
| | Alpine | x86_64 | [![Alpine](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=Linux%20%28Alpine%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |
| | Fedora | x86_64 | [![Fedora](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=Linux%20%28Fedora%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |
| | Debian | x86_64 | [![Debian](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=Linux%20%28Debian%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |
| | Arch Linux | x86_64 | [![Arch Linux](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=Linux%20%28Arch%20Linux%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |
| | openSUSE | x86_64 | [![openSUSE](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=Linux%20%28openSUSE%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |
| | AlmaLinux 8.10 | ppc64le | [![AlmaLinux](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=CFarm%20%28AlmaLinux%208.10%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |
| **BSD** | FreeBSD 15.0 (sysctl) | x86_64 | [![FreeBSD sysctl](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=FreeBSD%20%28sysctl%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |
| | FreeBSD 15.0 (procfs) | x86_64 | [![FreeBSD procfs](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=FreeBSD%20%28procfs%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |
| | OpenBSD 7.8 | x86_64 | [![OpenBSD](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=OpenBSD&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |
| | NetBSD 10.1 | x86_64 | [![NetBSD](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=NetBSD&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |
| | DragonFlyBSD 6.4.2 | x86_64 | [![DragonFlyBSD](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=DragonFlyBSD&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |
| | MidnightBSD 4.0.4 | x86_64 | [![MidnightBSD](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=MidnightBSD%20%284.0.4%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |
| **Solaris** | Oracle Solaris 11.4 | x86_64 | [![Solaris](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=Solaris&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |
| | OmniOS r151056 | x86_64 | [![OmniOS](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=OmniOS%20%28r151056%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |
| | Sun Solaris 10 | sun4u | [![Solaris 10](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=CFarm%20%28Sun%20Solaris%2010%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |
| **Haiku** | Haiku r1beta5 | x86_64 | [![Haiku](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=Haiku%20%28r1beta5%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |
| **AIX** | AIX 7.3 | chrp | [![AIX 7.3](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=CFarm%20%28AIX%207.3%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |

---
&copy; 2021-2026 Christopher Sean Morrison [@brlcad]
MIT License
