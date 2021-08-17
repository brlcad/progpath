
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

add_library(progpath-static STATIC progpath.c)
add_library(progpath SHARED progpath.c)

add_executable(progpath-bin progpath.h;progpath.c)
set_target_properties(progpath-bin PROPERTIES OUTPUT_NAME progpath)
target_link_libraries(progpath-bin LINK_PUBLIC progpath)
target_compile_definitions(progpath-bin PUBLIC BUILD_BINARY)
target_include_directories(progpath-bin PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# if (${BSD_LIBRARY})
#    target_link_libraries(progpath PRIVATE ${BSD_LIBRARY})
# endif (${BSD_LIBRARY})
# if (${C_LIBRARY})
#    target_link_libraries(progpath PRIVATE ${C_LIBRARY})
# endif (${C_LIBRARY})

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
target_compile_options(progpath-bin PRIVATE
   $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
   $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror -g -ggdb>
)
target_link_options(progpath-bin PRIVATE
   $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
   $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror -g -ggdb>
)
