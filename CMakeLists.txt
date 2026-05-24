cmake_minimum_required(VERSION 3.10)

project(progpath VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
include(GNUInstallDirs)
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

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  list(APPEND PP_CFLAGS ${PP_CFLAGS_MSVC})
else ()
  list(APPEND PP_CFLAGS ${PP_CFLAGS_NOT_MSVC})
endif ()

# identify the environment
cmake_host_system_information(RESULT os_name QUERY OS_NAME)
cmake_host_system_information(RESULT os_release QUERY OS_RELEASE)
cmake_host_system_information(RESULT os_platform QUERY OS_PLATFORM)
message(STATUS "OS: ${os_name} ${os_release} ${os_platform}")

cmake_host_system_information(RESULT PRETTY_NAME QUERY DISTRIB_PRETTY_NAME)
message(STATUS "${PRETTY_NAME}")

cmake_host_system_information(RESULT DISTRO QUERY DISTRIB_INFO)
foreach(VAR IN LISTS DISTRO)
  message(STATUS "${VAR}=`${${VAR}}`")
endforeach()

if (CMAKE_C_COMPILER_ID STREQUAL "SunPro" OR CMAKE_CXX_COMPILER_ID STREQUAL "SunPro")
  # SunPro C compiler doesn't automatically link the C++ runtime when linking C executables against C++ libraries
  link_libraries(Crun)
endif ()

# MSVC warns on standard CRT entry points such as getenv(), strncpy(),
# fopen(), and _open() unless the secure-CRT opt-out is defined before
# compilation.  Set it once here so it applies uniformly to the library
# and test targets without introducing source-level platform branches.
if (MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif ()

include(CheckProgPath)

message(STATUS "Testing for program name facilities: ${PP_CFLAGS}")

check_prog_path(CFLAGS "${PP_CFLAGS}")

configure_file(
  "${PROJECT_SOURCE_DIR}/progpath.h.in"
  "${PROJECT_BINARY_DIR}/progpath.h"
  @ONLY
  )

# cross-reference HAVE_* symbols across config template, cmake checks,
# and source code — warns at configure time if any are inconsistent
validate_config_symbols(
  "${PROJECT_SOURCE_DIR}/progpath.h.in"
  "${PROJECT_SOURCE_DIR}/CheckProgPath.cmake"
  "${PROJECT_SOURCE_DIR}/progpath.h.in"
)

###
# main progpath library, compiled static and dynamic
###
add_library(progpath-static STATIC
  "${PROJECT_BINARY_DIR}/progpath.h"
  progpath.cpp
)
target_compile_definitions(progpath-static PUBLIC PROGPATH_STATIC)
target_compile_options(progpath-static PRIVATE ${PP_CFLAGS})
target_include_directories(progpath-static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_library(progpath SHARED
  "${PROJECT_BINARY_DIR}/progpath.h"
  progpath.cpp
)
target_compile_definitions(progpath PRIVATE PROGPATH_EXPORTS)
set_target_properties(progpath PROPERTIES C_VISIBILITY_PRESET hidden CXX_VISIBILITY_PRESET hidden)
target_compile_options(progpath PRIVATE ${PP_CFLAGS})
target_include_directories(progpath PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

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
# Installation and Export
###
install(TARGETS progpath progpath-static
    EXPORT progpathTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/progpath.h"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT progpathTargets
    FILE progpathTargets.cmake
    NAMESPACE progpath::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/progpath
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/progpathConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/progpathConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/progpathConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/progpath
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/progpathConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/progpathConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/progpath
)

configure_file(progpath.pc.in progpath.pc @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/progpath.pc"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

# GNUInstallDirs defines a portable man directory when the platform
# uses one.  Keep the install rule simple and data-only so it remains
# harmless on platforms where man pages are less common.
if (DEFINED CMAKE_INSTALL_MANDIR AND NOT CMAKE_INSTALL_MANDIR STREQUAL "")
  install(FILES progpath.3
      DESTINATION "${CMAKE_INSTALL_MANDIR}/man3"
  )
endif ()

install(FILES
    README.md
    CHANGELOG.md
    INTEGRATION.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)

###
# example program uses the single-header implementation directly
###
add_executable(progpath-bin demo.cpp)
target_compile_options(progpath-bin PRIVATE ${PP_CFLAGS})
target_include_directories(progpath-bin PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)
set_target_properties(progpath-bin PROPERTIES OUTPUT_NAME progpath)

if (HAVE_TLO)
  # this method of setting link flags requires cmake 3.13+
  target_link_options(progpath-bin PRIVATE ${PP_CFLAGS})
else (HAVE_TLO)
  set_property(TARGET progpath-bin PROPERTY LINK_FLAGS "${PP_LDFLAGS}")
endif (HAVE_TLO)

enable_testing()

# CTest that fails if validate_config_symbols() found mismatches
add_test(NAME validate_config_symbols
  COMMAND ${CMAKE_COMMAND} -E echo "HAVE_* symbols are consistent"
)
if (PROGPATH_CONFIG_WARNINGS)
  # override to always fail when warnings exist
  set_tests_properties(validate_config_symbols PROPERTIES WILL_FAIL true)
endif()

add_subdirectory(tests)
