/*                      P R O G P A T H . H
 * progpath
 *
 * Copyright (c) 2021 Christopher Sean Morrison
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
