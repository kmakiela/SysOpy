cmake_minimum_required(VERSION 3.5)
project(zad3a)

set(CMAKE_C_STANDARD 99)


add_library(libstatic STATIC library.c library.h)
add_library(libshared SHARED library.c library.h)

add_executable(zad2static zad2.c)
target_link_libraries (zad2static libstatic)

add_executable(zad2shared zad2.c)
target_link_libraries (zad2shared libshared)

add_executable(zad2d zad2dll.c)
target_link_libraries(zad2d ${CMAKE_DL_LIBS})

