#include <progpath.h>
#include <stdio.h>
#include <string.h>

int main() {
    char path[4096];
    char ipwd[4096];

    if (!progpath(path, sizeof(path))) {
        fprintf(stderr, "progpath() failed\n");
        return 1;
    }
    if (!progipwd(ipwd, sizeof(ipwd))) {
        fprintf(stderr, "progipwd() failed\n");
        return 1;
    }

    printf("Installed progpath test success!\n");
    printf("Path: %s\n", path);
    printf("IPWD: %s\n", ipwd);

    return 0;
}
