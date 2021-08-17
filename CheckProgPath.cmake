
include(CheckIncludeFile)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(CheckCSourceCompiles)


function(CHECK_PROG_PATH)

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

  check_c_source_compiles("typedef void *rusage_info_t;\ntypedef unsigned char u_char;\ntypedef unsigned int u_int;\ntypedef unsigned long u_long;\ntypedef unsigned short u_short;\n#define SOCK_MAXADDRLEN 255\n#include <sys/types.h>\n#include <sys/sysctl.h>\nint main() { return 0; }" HAVE_SYS_SYSCTL_H)
  #check_include_file("sys/sysctl.h" HAVE_SYS_SYSCTL_H)

  check_include_file("FindDirectory.h" HAVE_FINDDIRECTORY_H)
  check_include_file("dlfcn.h" HAVE_DLFCN_H)
  check_include_file("mach-o/dyld.h" HAVE_MACH_O_DYLD_H)
  check_include_file("procinfo.h" HAVE_PROCINFO_H)
  check_include_file("sys/param.h" HAVE_SYS_PARAM_H)
  check_include_file("sys/procfs.h" HAVE_SYS_PROCFS_H)
  check_include_file("sys/types.h" HAVE_SYS_TYPES_H)
  check_include_file("sys/wait.h" HAVE_SYS_WAIT_H)
  check_include_file("unistd.h" HAVE_UNISTD_H)
  check_include_file("windows.h" HAVE_WINDOWS_H)

  check_function_exists(dladdr HAVE_DLADDR)
  check_function_exists(getprocs HAVE_GETPROCS)
  check_function_exists(getprocs64 HAVE_GETPROCS64)
  check_function_exists(getprogname HAVE_GETPROGNAME)
  check_function_exists(getexecname HAVE_GETEXECNAME)
  check_function_exists(proc_pidpath HAVE_PROC_PIDPATH)
  check_function_exists(GetModuleFileName HAVE_GETMODULEFILENAME)
  check_function_exists(sysctl HAVE_SYSCTL)
  check_function_exists(sysctlbyname HAVE_SYSCTLBYNAME)
  check_function_exists(readlink HAVE_READLINK)
  check_function_exists(find_path HAVE_FIND_PATH)
  check_function_exists(_NSGetExecutablePath HAVE__NSGETEXECUTABLEPATH)
  check_function_exists(program_invocation_name HAVE_DECL_PROGRAM_INVOCATION_NAME)
  check_function_exists(program_invocation_short_name HAVE_DECL_PROGRAM_INVOCATION_SHORT_NAME)
  check_function_exists(__argv HAVE_DECL__ARGV)
  check_function_exists(__progname HAVE_DECL___PROGNAME)
  check_function_exists(__progname_full HAVE_DECL___PROGNAME_FULL)

  check_symbol_exists(CTL_KERN "sys/types.h;sys/sysctl.h" HAVE_DECL_CTL_KERN)
  check_symbol_exists(KERN_PROC "sys/types.h;sys/sysctl.h" HAVE_DECL_KERN_PROC)
  check_symbol_exists(KERN_PROCNAME "sys/types.h;sys/sysctl.h" HAVE_DECL_KERN_PROCNAME)
  check_symbol_exists(KERN_PROC_ARGS "sys/types.h;sys/sysctl.h" HAVE_DECL_KERN_PROC_ARGS)
  check_symbol_exists(KERN_PROC_ARGV "sys/types.h;sys/sysctl.h" HAVE_DECL_KERN_PROC_ARGV)
  check_symbol_exists(KERN_PROC_PATHNAME "sys/types.h;sys/sysctl.h" HAVE_DECL_KERN_PROC_PATHNAME)
  check_symbol_exists(__argv stdlib.h HAVE_DECL__ARGV2)
  check_symbol_exists(__progname stdlib.h HAVE_DECL___PROGNAME2)
  check_symbol_exists(__progname_full stdlib.h HAVE_DECL___PROGNAME_FULL2)

endfunction(CHECK_PROG_PATH)
