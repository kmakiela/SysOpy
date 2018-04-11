cmake_minimum_required(VERSION 3.9)
project(ZAD1 C)

set(CMAKE_C_STANDARD 99)


add_library(zad1s STATIC library.c library.h)
add_library(zad1d SHARED library.c library.h)
