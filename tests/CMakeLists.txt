

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

add_executable(test_resolve_path test_resolve_path.cpp)
target_include_directories(test_resolve_path PRIVATE
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
)
add_test(NAME test_resolve_path COMMAND test_resolve_path)

# Validate progpath binary actually reports absolute paths.  Runs the
# demo program and checks the output.  Also sets PROGPATH_DEBUG=3 so
# failures produce useful output even without --output-on-failure.
add_test(NAME progpath_bin_output
  COMMAND ${CMAKE_COMMAND} -E env PROGPATH_DEBUG=3
    $<TARGET_FILE:progpath-bin>
)
set_tests_properties(progpath_bin_output PROPERTIES
  PASS_REGULAR_EXPRESSION "Program executable is \\[ /"
  FAIL_REGULAR_EXPRESSION  "ERROR:"
)
