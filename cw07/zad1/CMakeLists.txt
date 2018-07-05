cmake_minimum_required(VERSION 3.5)
project(zad1 C)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

add_executable(barber barber.c)
add_executable(client client.c)


