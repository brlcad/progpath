
project(progpath)

cmake_minimum_required(VERSION 3.12)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
  
include(CheckProgPath)

message(STATUS "Testing for program name facilities")

check_prog_path()

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

target_compile_options(progpath PRIVATE
   $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
   $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror -g -ggdb>
)
target_link_options(progpath PRIVATE
   $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
   $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror -g -ggdb>
)
target_compile_options(progpath-static PRIVATE
   $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
   $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror -g -ggdb>
)
target_link_options(progpath-static PRIVATE
   $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
   $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror -g -ggdb>
)


###
# example program links libprogpath, calls progpath() and progipwd()
###
add_executable(progpath-bin main.c)
set_target_properties(progpath-bin PROPERTIES OUTPUT_NAME progpath)
target_link_libraries(progpath-bin LINK_PUBLIC progpath)
#target_include_directories(progpath-bin PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(progpath-bin PRIVATE
   $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
   $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror -g -ggdb>
)
target_link_options(progpath-bin PRIVATE
   $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
   $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror -g -ggdb>
)
