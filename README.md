# progpath
[![Release Build and Install](https://github.com/brlcad/progpath/actions/workflows/release.yml/badge.svg)](https://github.com/brlcad/progpath/actions/workflows/release.yml)

tiny C/C++ single-header (or compiled) library for getting initial
paths for a running app, encapsulating platform-specific details for
getting a path to the running executable or the initial working dir.
progpath's focus:

1. absolute API simplicity,
2. trivial build integration,
3. number of methods/environments,
4. ease adding new methods, and
5. works after changing dirs!

### example, just include the header

```C
#define PROGPATH_IMPLEMENTATION
#include "progpath.h"
#include <stdio.h>
#include <stdlib.h>

int main(int ac, char *av[]) {
  char pp[4096];
  progpath(pp, sizeof(pp)); //pass buffer
  printf(" Program executable is [ %s ]\n", pp);

  char *ipwd;
  ipwd = progipwd(NULL, 0); // or allocate
  printf("Initial working dir is [ %s ]\n", ipwd);
  free(ipwd);

  return 0;
}
```

## build and run the demo

```shell
     % git clone https://github.com/brlcad/progpath.git && cd progpath
     % cmake -S . -B build && cmake --build build
     % cd ..  # change dir just for fun, then run 'progpath' binary
     % progpath/build/progpath
     Program executable is [ /Users/morrison/progpath/build/progpath ]
    Initial working dir is [ /Users/morrison ]
```

## cmake and/or pkg-config integration

compiled shared/static targets are supported for projects that prefer
package-manager or `find_package` integration.

after `cmake --install`, callers can use:

- CMake: `find_package(progpath REQUIRED)` then link `progpath::progpath`
- CMake static: link `progpath::progpath-static`
- `pkg-config`: `pkg-config --cflags --libs progpath`

more detailed notes in [INTEGRATION.md](INTEGRATION.md).

## optional UTF-8 contract

configure with `-DPROGPATH_UTF8=ON` to require both public functions to
return valid UTF-8 while keeping the same two-function `char *` API:

```shell
cmake -S . -B build -DPROGPATH_UTF8=ON
```

on Windows, progpath uses native UTF-16 path APIs and converts their
results strictly to UTF-8. on byte-oriented systems such as macOS,
Linux, BSD, Solaris, AIX, and Haiku, paths that are already valid UTF-8
are returned unchanged. a native byte path that is not valid UTF-8
cannot be converted without changing which file it identifies, so the
call fails instead with `errno` set to `EILSEQ`.

the generated header emits a compile-time error when UTF-8 mode is
requested on an incompatible character model or when the required
Windows Unicode APIs are unavailable. caller-provided buffers that are
too small also fail rather than returning partial UTF-8. native path
behavior remains the default.

## initialization

progpath captures the initial working directory (ipwd) once, as early
as possible, often automatically, so `progipwd()` can return it
correctly even after `chdir()`.  how that capture happens depends on
how you integrate:

| Build mode | Init behavior |
|---|---|
| Generated `progpath.h` implementation compiled as **C++** | **Automatic** — C++ static constructor fires before `main()`. nothing to do. |
| Generated `progpath.h` implementation compiled as **C** | **Automatic** — C static constructor fires before `main()`. nothing to do. |
| Shared library (`.so`/`.dylib`/`.dll`) | **Automatic** — static constructor fires before `main()`. nothing to do. |
| Static lib, linked from **C++** | **Automatic** — C++ runtime fires the constructor. |
| Static lib, linked from **pure C** | **Automatic** — C static constructor fires before `main()`. nothing to do.|

## notes and limitations

- `progpath.h` is configured for a specific target environment. run
  CMake to generate it for the platform you intend to ship.

- for cross-compiles or redistributed artifacts, generate the header
  using the target toolchain and target feature probes.

- thread safety: do not call `progpath()` or `progipwd()` concurrently
  before first capture completes. initialize from the main thread
  before spawning other threads.

- some methods temporarily change the working directory while
  resolving the executable path. avoid concurrent directory changes
  while using the API.

progpath is library API but includes an example 'progpath' demo
program for testing that should work everywhere.  [let me
know](https://github.com/brlcad/progpath/issues) if you find an
environment that doesn't work!

## platform CI status matrix

the broad platform matrix runs the default native mode on every change
and repeats automatically in UTF-8 mode each week. focused Linux and
Windows UTF-8 smoke jobs run on every change, exercising compiled-library
and single-header builds from a directory containing non-ASCII
characters. release CI installs and tests native packages on Linux,
macOS, and Windows, plus UTF-8 packages on representative POSIX and
Windows systems. badges reflect current status for each environment:

| OS Group | Environment | Architecture | Status |
|---|---|---|---|
| **Windows** | Windows Server 2022 | x86_64 | [![Windows 2022](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=Windows%20%28windows-2022%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |
| **macOS** | macOS 15 | arm64 | [![macOS 15](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=macOS%20%28macos-15%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |
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
| | CheriBSD (purecap) | Morello | [![CheriBSD purecap](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=CFarm%20%28CheriBSD%20purecap%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |
| **Solaris** | Oracle Solaris 11.4 | x86_64 | [![Solaris](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=Oracle%20Solaris%20CBE&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |
| | OmniOS r151056 | x86_64 | [![OmniOS](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=OmniOS%20%28r151056%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |
| | Sun Solaris 10 | sun4u | [![Solaris 10 host down](https://img.shields.io/badge/host-down-lightgrey)](https://www.mail-archive.com/cfarm-users@lists.tetaneutral.net/msg01552.html) |
| **Haiku** | Haiku r1beta6 | x86_64 | [![Haiku](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=Haiku%20%28r1beta6%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |
| **AIX** | AIX 7.3 | chrp | [![AIX 7.3](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=CFarm%20%28AIX%207.3%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |
| **GNU** | GNU/Hurd 2025 | x86_64 | [![GNU/Hurd](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=GNU%2FHurd%20%282025%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |
| **Redox** | Redox | x86_64 | [![Redox](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=Redox%20%28x86_64%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |
| **ReactOS** | ReactOS 0.4.15 | i386 | [![ReactOS](https://img.shields.io/github/check-runs/brlcad/progpath/main?nameFilter=ReactOS%20%280.4.15%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/matrix.yml) |

---
&copy; 2021-2026 Christopher Sean Morrison [@brlcad]
MIT License
