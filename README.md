# progpath
[![CI](https://github.com/brlcad/progpath/actions/workflows/ci.yml/badge.svg)](https://github.com/brlcad/progpath/actions/workflows/ci.yml)

tiny C/C++ library for getting initial paths for a running application,
encapsulating platform-specific details for getting a path to the running
executable or its initial working dir.

example usage:

```C
    #include "progpath.h"
    #include <stdio.h> // for printf
    #include <stdlib.h> // for free
    int main(int ac, char *av[]) {
      char pp[4096], *ipwd;

      progpath(pp, sizeof(pp)); // pass buffer
      printf(" Program executable is [ %s ]\n", pp);

      ipwd = progipwd(NULL, 0); // or allocate
      printf("Initial working dir is [ %s ]\n", ipwd);
      free(ipwd);

      return 0;
    }
```

that code is included and compiled by default, so can see it in action:

```shell
     % git clone https://github.com/brlcad/progpath.git && cd progpath
     % cmake -S . -B build && cmake --build build
     % cd ..  # change dir just for fun, then run 'progpath' binary
     % progpath/build/progpath
     Program executable is [ /Users/morrison/progpath/build/progpath ]
    Initial working dir is [ /Users/morrison ]
```

## Build

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Integrate

After `cmake --install`, consumers can use either:

- CMake: `find_package(progpath REQUIRED)` then link `progpath::progpath` or `progpath::progpath-static`
- pkg-config: `pkg-config --cflags --libs progpath`

Detailed consumer notes live in [INTEGRATION.md](INTEGRATION.md).  `make install`
also installs that file and this README into the package documentation directory.

## Initialization

progpath captures the initial working directory (ipwd) once, as early as
possible, so `progipwd()` returns it correctly even after `chdir()`.  How
that capture happens depends on how you link the library:

| Linking mode | Init behavior |
|---|---|
| Shared library (`.so`/`.dylib`/`.dll`) | Automatic — C++ static constructor fires before `main()`. Nothing to do. |
| Static lib, linked from **C++** | Same — C++ runtime fires the constructor. |
| Static lib, linked from **pure C** | **Lazy** — ipwd is captured on the first call to `progipwd()` or `progpath()`. Call one of them early in `main()`, before any `chdir()`. |

For the pure-C static case, the safe pattern is:

```c
int main(int argc, char *argv[]) {
    char ipwd[4096];
    progipwd(ipwd, sizeof(ipwd));  /* capture before any chdir */
    /* ... rest of program ... */
}
```

Thread safety: do not call progpath/progipwd concurrently before the
first capture completes.  Initialize from the main thread before spawning
other threads.

other efforts implement similar functionality, but where progpath differs is:

1. absolute API simplicity,
2. simple build integration,
3. number of methods it uses,
4. ease adding new methods, and
5. works after changing dirs!

progpath includes an example program for testing your environment, and should
work everywhere.  [let me know](https://github.com/brlcad/progpath/issues) if
you find an environment that doesn't work!

## Platform CI Status Matrix

The following environments are continuously tested. The badges reflect the current status of the `test.yml` workflow for each specific platform.

| OS Group | Environment | Architecture | Status |
|---|---|---|---|
| **Windows** | Windows Server 2019 | x86_64 | [![Windows 2019](https://img.shields.io/github/actions/workflow/status/brlcad/progpath/test.yml?job=Windows%20%28windows-2019%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/test.yml) |
| | Windows Server 2022 | x86_64 | [![Windows 2022](https://img.shields.io/github/actions/workflow/status/brlcad/progpath/test.yml?job=Windows%20%28windows-2022%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/test.yml) |
| **macOS** | macOS 13 (Intel) | x86_64 | [![macOS 13](https://img.shields.io/github/actions/workflow/status/brlcad/progpath/test.yml?job=macOS%20%28macos-13%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/test.yml) |
| | macOS 14 (Apple Silicon) | arm64 | [![macOS 14](https://img.shields.io/github/actions/workflow/status/brlcad/progpath/test.yml?job=macOS%20%28macos-14%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/test.yml) |
| **Linux** | Ubuntu (Latest) | x86_64 | [![Ubuntu](https://img.shields.io/github/actions/workflow/status/brlcad/progpath/test.yml?job=Linux%20%28ubuntu-latest%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/test.yml) |
| | Alpine | x86_64 | [![Alpine](https://img.shields.io/github/actions/workflow/status/brlcad/progpath/test.yml?job=Linux%20%28alpine%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/test.yml) |
| | Fedora | x86_64 | [![Fedora](https://img.shields.io/github/actions/workflow/status/brlcad/progpath/test.yml?job=Linux%20%28fedora%3Alatest%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/test.yml) |
| | Debian | x86_64 | [![Debian](https://img.shields.io/github/actions/workflow/status/brlcad/progpath/test.yml?job=Linux%20%28debian%3Alatest%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/test.yml) |
| | Arch Linux | x86_64 | [![Arch](https://img.shields.io/github/actions/workflow/status/brlcad/progpath/test.yml?job=Linux%20%28archlinux%3Alatest%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/test.yml) |
| | AlmaLinux | x86_64 | [![AlmaLinux](https://img.shields.io/github/actions/workflow/status/brlcad/progpath/test.yml?job=Linux%20%28almalinux%3Alatest%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/test.yml) |
| **BSD** | FreeBSD 15.0 (sysctl) | x86_64 | [![FreeBSD sysctl](https://img.shields.io/github/actions/workflow/status/brlcad/progpath/test.yml?job=BSD%20%28freebsd%2C%20no%20%2Fproc%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/test.yml) |
| | FreeBSD 15.0 (procfs) | x86_64 | [![FreeBSD procfs](https://img.shields.io/github/actions/workflow/status/brlcad/progpath/test.yml?job=BSD%20%28freebsd%2C%20with%20%2Fproc%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/test.yml) |
| | OpenBSD 7.8 | x86_64 / arm64 | [![OpenBSD](https://img.shields.io/github/actions/workflow/status/brlcad/progpath/test.yml?job=BSD%20%28openbsd%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/test.yml) |
| | NetBSD 10.1 | x86_64 / arm64 | [![NetBSD](https://img.shields.io/github/actions/workflow/status/brlcad/progpath/test.yml?job=BSD%20%28netbsd%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/test.yml) |
| | DragonFlyBSD 6.4.2 | x86_64 | [![DragonFlyBSD](https://img.shields.io/github/actions/workflow/status/brlcad/progpath/test.yml?job=BSD%20%28dragonflybsd%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/test.yml) |
| | MidnightBSD 4.0.4 | x86_64 | [![MidnightBSD](https://img.shields.io/github/actions/workflow/status/brlcad/progpath/test.yml?job=MidnightBSD%20%284.0.4%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/test.yml) |
| **Illumos** | Solaris | x86_64 | [![Solaris](https://img.shields.io/github/actions/workflow/status/brlcad/progpath/test.yml?job=Solaris&label=test)](https://github.com/brlcad/progpath/actions/workflows/test.yml) |
| | OmniOS r151056 | x86_64 | [![OmniOS](https://img.shields.io/github/actions/workflow/status/brlcad/progpath/test.yml?job=OmniOS%20%28r151056%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/test.yml) |
| **Haiku** | Haiku r1beta5 | x86_64 | [![Haiku](https://img.shields.io/github/actions/workflow/status/brlcad/progpath/test.yml?job=Haiku%20%28r1beta5%29&label=test)](https://github.com/brlcad/progpath/actions/workflows/test.yml) |

---
&copy; 2021-2026 Christopher Sean Morrison [@brlcad]
MIT License
