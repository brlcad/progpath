
project(progpath)

cmake_minimum_required(VERSION 3.12)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
  
set(PROGPATH_CFLAGS
   $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
   $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror -g -ggdb>
   )

include(CheckProgPath)

message(STATUS "Testing for program name facilities")

check_prog_path(CFLAGS "${PROGPATH_CFLAGS}")

configure_file(
  "${PROJECT_SOURCE_DIR}/progpath_config.h.in"
  "${PROJECT_BINARY_DIR}/progpath_config.h"
  )
include_directories("${PROJECT_BINARY_DIR}")

###
# main progpath library, compiled static and dynamic
###
add_library(progpath-static STATIC progpath.h;progpath.c;init.cpp)
add_library(progpath SHARED progpath.h;progpath.c;init.cpp)

target_compile_options(progpath PRIVATE ${PROGPATH_CFLAGS})
target_link_options(progpath PRIVATE ${PROGPATH_CFLAGS})

target_compile_options(progpath-static PRIVATE ${PROGPATH_CFLAGS})
target_link_options(progpath-static PRIVATE ${PROGPATH_CFLAGS})


###
# example program links libprogpath, calls progpath() and progipwd()
###
add_executable(progpath-bin main.c)
set_target_properties(progpath-bin PROPERTIES OUTPUT_NAME progpath)
target_link_libraries(progpath-bin LINK_PUBLIC progpath)
#target_include_directories(progpath-bin PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(progpath-bin PRIVATE ${PROGPATH_CFLAGS})
target_link_options(progpath-bin PRIVATE ${PROGPATH_CFLAGS})
