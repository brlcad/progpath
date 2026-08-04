

add_executable(test_basic test_basic.c)
target_link_libraries(test_basic progpath-static)
target_include_directories(test_basic PRIVATE ${PROJECT_SOURCE_DIR})
add_test(NAME test_basic COMMAND test_basic)

add_executable(test_api test_api.c)
target_link_libraries(test_api progpath-static)
target_include_directories(test_api PRIVATE ${PROJECT_SOURCE_DIR})
add_test(NAME test_api COMMAND test_api)

add_executable(test_cpp test_cpp.cpp)
target_link_libraries(test_cpp progpath-static)
target_include_directories(test_cpp PRIVATE ${PROJECT_SOURCE_DIR})
add_test(NAME test_cpp COMMAND test_cpp)

add_executable(test_correctness test_correctness.c)
target_link_libraries(test_correctness progpath-static)
target_include_directories(test_correctness PRIVATE ${PROJECT_SOURCE_DIR})
add_test(NAME test_correctness COMMAND test_correctness)

add_executable(test_method_priority test_method_priority.c)
target_link_libraries(test_method_priority progpath-static)
target_include_directories(test_method_priority PRIVATE ${PROJECT_SOURCE_DIR})
add_test(NAME test_method_priority COMMAND test_method_priority)
set_tests_properties(test_method_priority PROPERTIES
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  ENVIRONMENT "PROGPATH_DEBUG=0;PWD=${CMAKE_CURRENT_BINARY_DIR}"
)

add_executable(test_resolve_path test_resolve_path.cpp)
target_compile_definitions(test_resolve_path PRIVATE PROGPATH_STATIC)
target_include_directories(test_resolve_path PRIVATE
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
)
add_test(NAME test_resolve_path COMMAND test_resolve_path)

add_executable(test_single_header_cpp test_single_header_cpp.cpp)
target_include_directories(test_single_header_cpp PRIVATE ${PROJECT_BINARY_DIR})
add_test(NAME test_single_header_cpp COMMAND test_single_header_cpp)
set_tests_properties(test_single_header_cpp PROPERTIES
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  ENVIRONMENT "PWD=${CMAKE_CURRENT_BINARY_DIR}"
)

add_executable(test_single_header_split
  test_single_header_split_c.c
  test_single_header_impl.cpp
)
target_include_directories(test_single_header_split PRIVATE ${PROJECT_BINARY_DIR})
add_test(NAME test_single_header_split COMMAND test_single_header_split)
set_tests_properties(test_single_header_split PROPERTIES
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  ENVIRONMENT "PWD=${CMAKE_CURRENT_BINARY_DIR}"
)

add_executable(test_single_header_c test_single_header_c.c)
target_include_directories(test_single_header_c PRIVATE ${PROJECT_BINARY_DIR})
add_test(NAME test_single_header_c COMMAND test_single_header_c)
set_tests_properties(test_single_header_c PROPERTIES
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  ENVIRONMENT "PWD=${CMAKE_CURRENT_BINARY_DIR}"
)

# Validate progpath binary actually reports absolute paths.  Runs the
# demo program and checks the output.  Also sets PROGPATH_DEBUG=3 so
# failures produce useful output even without --output-on-failure.
add_test(NAME progpath_bin_output
  COMMAND ${CMAKE_COMMAND} -E env PROGPATH_DEBUG=3
    $<TARGET_FILE:progpath-bin>
)
set_tests_properties(progpath_bin_output PROPERTIES
  PASS_REGULAR_EXPRESSION "Program executable is \\[ ([A-Za-z]:\\\\|/)"
  FAIL_REGULAR_EXPRESSION  "ERROR:"
)

add_executable(test_c_init test_c_init.c)
target_link_libraries(test_c_init progpath-static)
target_include_directories(test_c_init PRIVATE ${PROJECT_SOURCE_DIR})
add_test(NAME test_c_init COMMAND test_c_init)
set_tests_properties(test_c_init PROPERTIES
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  ENVIRONMENT "PWD=${CMAKE_CURRENT_BINARY_DIR}"
)

if (PROGPATH_UTF8)
  set(PROGPATH_UTF8_TEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/utf8-测试")
  file(MAKE_DIRECTORY "${PROGPATH_UTF8_TEST_DIR}")

  add_executable(test_utf8_compiled test_utf8.c)
  target_link_libraries(test_utf8_compiled progpath-static)
  target_include_directories(test_utf8_compiled PRIVATE ${PROJECT_SOURCE_DIR})

  add_executable(test_utf8_header test_utf8.c)
  target_compile_definitions(test_utf8_header PRIVATE PROGPATH_UTF8_HEADER_ONLY)
  target_include_directories(test_utf8_header PRIVATE ${PROJECT_BINARY_DIR})

  foreach(_utf8_target test_utf8_compiled test_utf8_header)
    set_target_properties(${_utf8_target} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${PROGPATH_UTF8_TEST_DIR}"
      RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROGPATH_UTF8_TEST_DIR}"
      RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROGPATH_UTF8_TEST_DIR}"
      RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${PROGPATH_UTF8_TEST_DIR}"
      RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${PROGPATH_UTF8_TEST_DIR}"
    )
    add_test(NAME ${_utf8_target} COMMAND ${_utf8_target})
    set_tests_properties(${_utf8_target} PROPERTIES
      WORKING_DIRECTORY "${PROGPATH_UTF8_TEST_DIR}"
      ENVIRONMENT "PWD=${PROGPATH_UTF8_TEST_DIR}"
    )
  endforeach()
endif ()
