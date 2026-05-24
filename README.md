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

## example, included as 'progpath' binary:

```C
    #include "progpath.h"
    #include <stdio.h> // for printf
    #include <stdlib.h> // for free
    int main(int ac, char *av[]) {

      char pp[4096];
      progpath(pp, sizeof(pp)); // pass buffer
      printf(" Program executable is [ %s ]\n", pp);

      char *ipwd;
      ipwd = progipwd(NULL, 0); // or allocate
      printf("Initial working dir is [ %s ]\n", ipwd);
      free(ipwd);

      return 0;
    }
```

## compile and run demo:

```shell
     % git clone https://github.com/brlcad/progpath.git && cd progpath
     % cmake -S . -B build && cmake --build build
     % cd ..  # change dir just for fun, then run 'progpath' binary
     % progpath/build/progpath
     Program executable is [ /Users/morrison/progpath/build/progpath ]
    Initial working dir is [ /Users/morrison ]
```

## integrate into other codes

after `cmake --install`, callers can use:

- CMake: `find_package(progpath REQUIRED)` then link `progpath::progpath`

of find more detailed notes and other options in [INTEGRATION.md](INTEGRATION.md).

## initialization

progpath captures the initial working directory (ipwd) once, as early
as possible, so `progipwd()` can return it correctly even after
`chdir()`.  how that capture happens depends on how you link the
library:

| Linking mode | Init behavior |
|---|---|
| Shared library (`.so`/`.dylib`/`.dll`) | **Automatic** — C++ static constructor fires before `main()`. Nothing to do. |
| Static lib, linked from **C++** | **Automatic** — C++ runtime fires the constructor. |
| Static lib, linked from **pure C** | **Manual** — initial path is captured on first call to `progipwd()` or `progpath()`. Call either early, before any `chdir()`.|

thread safety: do not call progpath/progipwd concurrently before first
capture completes.  Init from main thread before spawning other
threads.

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
