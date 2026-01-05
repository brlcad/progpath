

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
