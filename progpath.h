/*                      P R O G P A T H . H
 * progpath
 *
 * Copyright (c) 2021-2026 Christopher Sean Morrison
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

#define PROGPATH_VERSION "1.0.0"

/**
 * @brief Get the absolute filesystem path to the application's binary.
 *
 * A path string will be written to the provided 'buf' buffer. It will write
 * at most 'len'-1 bytes to ensure null-termination.
 *
 * If 'buf' is NULL, memory will be dynamically allocated via calloc() and
 * the caller is responsible for calling free().
 *
 * @param buf Buffer to write the path to, or NULL to allocate memory.
 * @param len Size of the buffer in bytes.
 * @return Pointer to the buffer containing the path (either 'buf' or allocated), or NULL on failure.
 */
extern char *progpath(char *buf, size_t len);

/**
 * @brief Get the absolute path to the application's initial working directory.
 *
 * This obtains the current directory of the application as it was on entry to
 * main().
 *
 * A path string will be written to the provided 'buf' buffer. It will write
 * at most 'len'-1 bytes to ensure null-termination.
 *
 * If 'buf' is NULL, memory will be dynamically allocated via calloc() and
 * the caller is responsible for calling free().
 *
 * @param buf Buffer to write the path to, or NULL to allocate memory.
 * @param len Size of the buffer in bytes.
 * @return Pointer to the buffer containing the path (either 'buf' or allocated), or NULL on failure.
 */
extern char *progipwd(char *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* PROGPATH_H */
