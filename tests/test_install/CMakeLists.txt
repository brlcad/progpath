cmake_minimum_required(VERSION 3.10)
project(test_install_consumer C)

# find the installed progpath package
# when running in CI, CMAKE_PREFIX_PATH will be set to the install prefix
find_package(progpath REQUIRED)

add_executable(test_consumer test_consumer.c)
target_link_libraries(test_consumer progpath::progpath-static)

enable_testing()
add_test(NAME test_installed_package COMMAND test_consumer)

# optional: also test shared if available
# target_link_libraries(test_consumer progpath::progpath)
