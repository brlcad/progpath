#include "progpath.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    char pp[4096];
    char *ipwd;
    
    // Test progpath
    memset(pp, 0, sizeof(pp));
    if (progpath(pp, sizeof(pp)) == NULL) {
        fprintf(stderr, "FAIL: progpath returned NULL\n");
        return 1;
    }
    
    if (strlen(pp) == 0) {
        fprintf(stderr, "FAIL: progpath returned empty string\n");
        return 1;
    }
    
    printf("progpath: %s\n", pp);
    
    // Test progipwd
    ipwd = progipwd(NULL, 0);
    if (ipwd == NULL) {
        fprintf(stderr, "FAIL: progipwd returned NULL\n");
        return 1;
    }
    
    if (strlen(ipwd) == 0) {
        fprintf(stderr, "FAIL: progipwd returned empty string\n");
        free(ipwd);
        return 1;
    }
    
    printf("progipwd: %s\n", ipwd);
    free(ipwd);
    
    return 0;
}
