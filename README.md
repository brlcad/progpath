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
     % mkdir .build && cd .build && cmake .. && make  # creates 'progpath'
     % cd ../..  # change dir just for fun, then run 'progpath' binary
     % progpath/.build/progpath
     Program executable is [ /Users/morrison/progpath/.build/progpath ]
    Initial working dir is [ /Users/morrison ]
```

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

---
&copy; 2021-2026 Christopher Sean Morrison [@brlcad]
MIT License
