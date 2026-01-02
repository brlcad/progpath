
add_executable(test_basic test_basic.c)
target_link_libraries(test_basic progpath-static)
target_include_directories(test_basic PRIVATE ${PROJECT_SOURCE_DIR})
add_test(NAME test_basic COMMAND test_basic)
