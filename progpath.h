#ifndef PROGPATH_H
#define PROGPATH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>


/**
 * get the absolute filesystem path to an application's binary
 *
 * A path string will be written to the provided 'buf' buffer at most
 * 'len'-1 bytes to ensure nul-termination.  If 'buf' is NULL, memory
 * will be dynamically allocated via calloc() and caller is
 * responsible for calling free().
 */
extern char *progpath(char *buf, size_t len);


/**
 * get absolute path to an application's initial working directory
 *
 * This obtains the current directory of an application on entry to
 * main().  A path string will be written to the provided 'buf' buffer
 * at most 'len'-1 bytes to ensure nul-termination.  If 'buf' is NULL,
 * memory will be dynamically allocated via calloc() and caller is
 * responsible for calling free().
 */
extern char *progipwd(char *buf, size_t len);


#ifdef __cplusplus
}
#endif

#endif /* PROGPATH_H */
