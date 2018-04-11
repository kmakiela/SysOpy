cmake_minimum_required(VERSION 3.5)
project(ZAD1 C)

set(CMAKE_C_STANDARD 99)


add_library(libs STATIC library.c library.h)
add_executable(zad2 zad2.c)
target_link_libraries (zad2 libs)
