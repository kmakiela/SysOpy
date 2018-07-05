cmake_minimum_required(VERSION 3.5)
project(lab9 C)

set(CMAKE_C_STANDARD 99)
link_libraries(pthread)
add_executable(posix posix.c)
