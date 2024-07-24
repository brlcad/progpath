cmake_minimum_required(VERSION 3.7)

project(progpath)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
if (${CMAKE_VERSION} GREATER_EQUAL "3.12")
  # use CMAKE_REQUIRED_LIBRARIES during testing
  cmake_policy(SET CMP0075 NEW) 
endif (${CMAKE_VERSION} GREATER_EQUAL "3.12")
  

option(PROGPATH_STRICT "Turn on all warnings, treat as errors")
set(PROGPATH_CFLAGS "" CACHE STRING "Specify your own flags")

list(APPEND PP_CFLAGS ${PROGPATH_CFLAGS})
separate_arguments(PP_CFLAGS)

# spaces because older cmake has busted flag list support
string (REPLACE ";" " " PP_LDFLAGS "${PP_CFLAGS}")

if (PROGPATH_STRICT)
# crank up the compilation+linker warning flags here
  set(PP_CFLAGS_MSVC /W4 /WX)
  set(PP_CFLAGS_NOT_MSVC -Wall -Wextra -pedantic -Werror -g -ggdb)
endif (PROGPATH_STRICT)

if (CXX_COMPILER_ID EQUAL "MSVC")
  list(APPEND PP_CFLAGS ${PP_CFLAGS_MSVC})
else (CXX_COMPILER_ID EQUAL "MSVC")
  list(APPEND PP_CFLAGS ${PP_CFLAGS_NOT_MSVC})
endif (CXX_COMPILER_ID EQUAL "MSVC")

include(CheckProgPath)

message(STATUS "Testing for program name facilities: ${PP_CFLAGS}")

check_prog_path(CFLAGS "${PP_CFLAGS}")

configure_file(
  "${PROJECT_SOURCE_DIR}/progpath_config.h.in"
  "${PROJECT_BINARY_DIR}/progpath_config.h"
  )
include_directories("${PROJECT_BINARY_DIR}")

###
# main progpath library, compiled static and dynamic
###
add_library(progpath-static STATIC progpath.h;progpath.c;init.cpp)
target_compile_options(progpath-static PRIVATE ${PP_CFLAGS})

add_library(progpath SHARED progpath.h;progpath.c;init.cpp)
target_compile_options(progpath PRIVATE ${PP_CFLAGS})

include(cmake/TargetLinkOptions.cmake OPTIONAL RESULT_VARIABLE HAVE_TLO)
if (HAVE_TLO)
  # this method of setting link flags requires cmake 3.13+
  target_link_options(progpath PRIVATE ${PP_CFLAGS})
  target_link_options(progpath-static PRIVATE ${PP_CFLAGS})
else (HAVE_TLO)
  set_property(TARGET progpath PROPERTY LINK_FLAGS "${PP_LDFLAGS}")
  set_property(TARGET progpath-static PROPERTY LINK_FLAGS "${PP_LDFLAGS}")
endif (HAVE_TLO)


###
# example program links libprogpath, calls progpath() and progipwd()
###
add_executable(progpath-bin main.c)
target_compile_options(progpath-bin PRIVATE ${PP_CFLAGS})

# link lib, then rename because 'progpath' target is the library
target_link_libraries(progpath-bin LINK_PUBLIC progpath)
set_target_properties(progpath-bin PROPERTIES OUTPUT_NAME progpath)

if (HAVE_TLO)
  # this method of setting link flags requires cmake 3.13+
  target_link_options(progpath-bin PRIVATE ${PP_CFLAGS})
else (HAVE_TLO)
  set_property(TARGET progpath-bin PROPERTY LINK_FLAGS "${PP_LDFLAGS}")
endif (HAVE_TLO)
