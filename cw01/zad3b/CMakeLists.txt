cmake_minimum_required(VERSION 3.5)
project(zad3a)

set(CMAKE_C_STANDARD 99)


add_library(libstatic STATIC library.c library.h)
add_library(libshared SHARED library.c library.h)
add_executable(zad2static zad2.c)
add_executable(zad2shared zad2.c)
add_executable(zad2d zad2dll.c)
target_link_libraries (zad2shared libshared)
target_link_libraries (zad2static libstatic)
target_link_libraries(zad2d ${CMAKE_DL_LIBS})
set_target_properties(zad2static zad2shared zad2d PROPERTIES COMPILE_FLAGS -O)


add_library(libstatic2 STATIC library.c library.h)
add_library(libshared2 SHARED library.c library.h)
add_executable(zad2static2 zad2.c)
add_executable(zad2shared2 zad2.c)
add_executable(zad2d2 zad2dll.c)
target_link_libraries (zad2shared2 libshared3)
target_link_libraries (zad2static2 libstatic3)
target_link_libraries(zad2d2 ${CMAKE_DL_LIBS})
set_target_properties(zad2static2 zad2shared2 zad2d2 PROPERTIES COMPILE_FLAGS -O2)


add_library(libstatic3 STATIC library.c library.h)
add_library(libshared3 SHARED library.c library.h)
add_executable(zad2static3 zad2.c)
add_executable(zad2shared3 zad2.c)
add_executable(zad2d3 zad2dll.c)
target_link_libraries (zad2shared3 libshared3)
target_link_libraries (zad2static3 libstatic3)
target_link_libraries(zad2d3 ${CMAKE_DL_LIBS})
set_target_properties(zad2static3 zad2shared3 zad2d3 PROPERTIES COMPILE_FLAGS -O3)

add_custom_target(test1 ALL COMMAND echo Static Library -O >> raport2.txt COMMAND ./zad2static)
add_custom_target(test2 ALL COMMAND echo Dynamic Library -O >> raport2.txt COMMAND ./zad2d)
add_custom_target(test3 ALL COMMAND echo Shared Library -O >> raport2.txt COMMAND ./zad2shared)
add_dependencies(test1 zad2static)
add_dependencies(test2 zad2d test1)
add_dependencies(test3 zad2shared test2)

add_custom_target(test12 ALL COMMAND echo Static Library -O2 >> raport2.txt COMMAND ./zad2static2)
add_custom_target(test22 ALL COMMAND echo Dynamic Library -O2 >> raport2.txt COMMAND ./zad2d2)
add_custom_target(test32 ALL COMMAND echo Shared Library -O2 >> raport2.txt COMMAND ./zad2shared2)
add_dependencies(test12 zad2static2 test3)
add_dependencies(test22 zad2d2 test12)
add_dependencies(test32 zad2shared2 test22)

add_custom_target(test13 ALL COMMAND echo Static Library -O3 >> raport2.txt COMMAND ./zad2static3)
add_custom_target(test23 ALL COMMAND echo Dynamic Library -O3 >> raport2.txt COMMAND ./zad2d3)
add_custom_target(test33 ALL COMMAND echo Shared Library -O3 >> raport2.txt COMMAND ./zad2shared3)
add_dependencies(test13 zad2static3 test32)
add_dependencies(test23 zad2d3 test13)
add_dependencies(test33 zad2shared3 test23)
