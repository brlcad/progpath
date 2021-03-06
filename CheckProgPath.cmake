#                C H E C K P R O G P A T H . C M A K E
# progpath
#
# Copyright (c) 2021 Christopher Sean Morrison
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation files
# (the "Software"), to deal in the Software without restriction,
# including without limitation the rights to use, copy, modify, merge,
# publish, distribute, sublicense, and/or sell copies of the Software,
# and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
# BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
###

include(CheckIncludeFile)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(CheckCSourceCompiles)
include(CheckStructHasMember)

function(CHECK_PROG_PATH)

  set (cflags CFLAGS)
  cmake_parse_arguments(CHECK_PP "" "" "${cflags}" ${ARGN})

  # make sure application cflags are used during feature testing
  if (CHECK_PP_CFLAGS)
    set (CMAKE_REQUIRED_FLAGS ${CHECK_PP_CFLAGS})
  endif (CHECK_PP_CFLAGS)

  find_library(BSD_LIBRARY bsd)
  if (BSD_LIBRARY)
    set(PP_LIBRARIES ${PP_LIBRARIES} ${BSD_LIBRARY})
    set(CMAKE_REQUIRED_LIBRARIES "${BSD_LIBRARY}")
  endif (BSD_LIBRARY)

  find_library(C_LIBRARY c)
  if (C_LIBRARY)
    set(PP_LIBRARIES ${PP_LIBRARIES} ${C_LIBRARY})
    set(CMAKE_REQUIRED_LIBRARIES "${C_LIBRARY}")
  endif (C_LIBRARY)

  set(PROGPATH_LIBRARIES ${PP_LIBRARIES} CACHE STRING "ProgPath libraries")
  mark_as_advanced(PROGPATH_LIBRARIES)

  # sys/sysctl.h requires special attention because several platforms
  # don't present it usably out of the box without other types and
  # headers.
  check_c_source_compiles("typedef void *rusage_info_t;\ntypedef unsigned char u_char;\ntypedef unsigned int u_int;\ntypedef unsigned long u_long;\ntypedef unsigned short u_short;\n#define SOCK_MAXADDRLEN 255\n#include <sys/types.h>\n#include <sys/sysctl.h>\nint main() { return 0; }" HAVE_SYS_SYSCTL_H)
  # check_include_file("sys/sysctl.h" HAVE_SYS_SYSCTL_H)

  # headers with potentially relevant API
  check_include_file("FindDirectory.h" HAVE_FINDDIRECTORY_H)
  check_include_file("dlfcn.h" HAVE_DLFCN_H)
  check_include_file("fcntl.h" HAVE_FCNTL_H)
  check_include_file("mach-o/dyld.h" HAVE_MACH_O_DYLD_H)
  check_include_file("procinfo.h" HAVE_PROCINFO_H)
  check_include_file("sys/auxv.h" HAVE_SYS_AUXV_H)
  check_include_file("sys/ioctl.h" HAVE_SYS_IOCTL_H)
  check_include_file("sys/param.h" HAVE_SYS_PARAM_H)
  check_include_file("sys/procfs.h" HAVE_SYS_PROCFS_H)
  check_include_file("sys/types.h" HAVE_SYS_TYPES_H)
  check_include_file("sys/wait.h" HAVE_SYS_WAIT_H)
  check_include_file("unistd.h" HAVE_UNISTD_H)
  check_include_file("windows.h" HAVE_WINDOWS_H)

  # global variables
  check_function_exists(__argv HAVE_DECL__ARGV)
  check_symbol_exists(__argv stdlib.h HAVE_DECL__ARGV2)
  check_function_exists(__progname HAVE_DECL___PROGNAME)
  check_symbol_exists(__progname stdlib.h HAVE_DECL___PROGNAME2)
  check_function_exists(__progname_full HAVE_DECL___PROGNAME_FULL)
  check_symbol_exists(__progname_full stdlib.h HAVE_DECL___PROGNAME_FULL2)
  check_function_exists(program_invocation_name HAVE_DECL_PROGRAM_INVOCATION_NAME)
  check_symbol_exists(program_invocation_name errno.h HAVE_DECL_PROGRAM_INVOCATION_NAME2)
  check_function_exists(program_invocation_short_name HAVE_DECL_PROGRAM_INVOCATION_SHORT_NAME)
  check_symbol_exists(program_invocation_short_name errno.h HAVE_DECL_PROGRAM_INVOCATION_SHORT_NAME2)

  # functions
  check_function_exists(GetModuleFileName HAVE_GETMODULEFILENAME)
  check_function_exists(_NSGetExecutablePath HAVE__NSGETEXECUTABLEPATH)
  check_function_exists(_get_pgmptr HAVE__GET_PGMPTR)
  check_function_exists(_getcwd HAVE__GETCWD)
  check_function_exists(dladdr HAVE_DLADDR)
  check_function_exists(dlsym HAVE_DLSYM)
  check_function_exists(find_path HAVE_FIND_PATH)
  check_function_exists(getauxval HAVE_GETAUXVAL)
  check_function_exists(getcwd HAVE_GETCWD)
  check_function_exists(getexecname HAVE_GETEXECNAME)
  check_function_exists(getprocs HAVE_GETPROCS)
  check_function_exists(getprocs64 HAVE_GETPROCS64)
  check_function_exists(getprogname HAVE_GETPROGNAME)
  check_function_exists(proc_pidpath HAVE_PROC_PIDPATH)
  check_function_exists(read HAVE_READ)
  check_function_exists(readlink HAVE_READLINK)
  check_function_exists(realpath HAVE_REALPATH)
  check_function_exists(sysctl HAVE_SYSCTL)
  check_function_exists(sysctlbyname HAVE_SYSCTLBYNAME)

  # sysctl-style symbols
  check_symbol_exists(CTL_KERN "sys/types.h;sys/sysctl.h" HAVE_DECL_CTL_KERN)
  check_symbol_exists(KERN_PROC "sys/types.h;sys/sysctl.h" HAVE_DECL_KERN_PROC)
  check_symbol_exists(KERN_PROCARGS2 "sys/types.h;sys/sysctl.h" HAVE_DECL_KERN_PROCARGS2)
  check_symbol_exists(KERN_PROCNAME "sys/types.h;sys/sysctl.h" HAVE_DECL_KERN_PROCNAME)
  check_symbol_exists(KERN_PROC_ARGS "sys/types.h;sys/sysctl.h" HAVE_DECL_KERN_PROC_ARGS)
  check_symbol_exists(KERN_PROC_ARGV "sys/types.h;sys/sysctl.h" HAVE_DECL_KERN_PROC_ARGV)
  check_symbol_exists(KERN_PROC_PATHNAME "sys/types.h;sys/sysctl.h" HAVE_DECL_KERN_PROC_PATHNAME)
  check_symbol_exists(PIOCPSINFO "sys/ioctl.h" HAVE_DECL_PIOCPSINFO)

  # procfs structures
  check_struct_has_member("struct psinfo" pr_argv sys/procfs.h HAVE_STRUCT_PSINFO)
  check_struct_has_member("struct prpsinfo" pr_fname sys/procfs.h HAVE_STRUCT_PRPSINFO)
  
endfunction(CHECK_PROG_PATH)
