cmake_minimum_required(VERSION 3.10)
project(test_install_consumer C CXX)

# find the installed progpath package
# when running in CI, CMAKE_PREFIX_PATH will be set to the install prefix
find_package(progpath REQUIRED)
get_target_property(PROGPATH_INCLUDE_DIRS progpath::progpath INTERFACE_INCLUDE_DIRECTORIES)

add_executable(test_consumer test_consumer.c)
target_link_libraries(test_consumer progpath::progpath-static)

add_executable(test_header_consumer test_header_consumer.cpp)
target_include_directories(test_header_consumer PRIVATE ${PROGPATH_INCLUDE_DIRS})

enable_testing()
add_test(NAME test_installed_package COMMAND test_consumer)
add_test(NAME test_installed_header COMMAND test_header_consumer)
