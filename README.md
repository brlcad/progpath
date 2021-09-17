# progpath
simple C/C++ library for getting initial running program executable paths, 
encapsulating platform-specific details for getting the path to the current
running program executable or it's initial working directory.

let's get right to it:

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

version of that is in the repo, compiled by default, so can see it in action:

```shell
     % git clone https://github.com/brlcad/progpath.git && cd progpath
     % mkdir .build && cd .build && cmake .. && make  # creates 'progpath'
     % cd ../..  # change dir just for fun, then run 'progpath' binary
     % progpath/.build/progpath
     Program executable is [ /Users/morrison/progpath/.build/progpath ]
    Initial working dir is [ /Users/morrison ]
```

other efforts implement similar functionality, but where progpath differs is:

1. absolute API simplicity,
2. number of methods employed,
3. simplified integration,
4. ease of extension, and
5. works after dir changes!

progpath works on nearly all platform environments and includes an example
program to check yours.  It's actively tested on BSD, Linux, Mac OS X, AIX, 
Haiku, Windows, ... [let me know](https://github.com/brlcad/progpath/issues)
if you find an environment that doesn't work!

---
&copy; 2021 Christopher Sean Morrison [@brlcad]
MIT License
