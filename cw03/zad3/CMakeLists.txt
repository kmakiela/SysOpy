cmake_minimum_required(VERSION 3.5)
project(zad3)

set(CMAKE_C_STANDARD 99)

add_executable(main main.c)
add_executable(break break.c)
add_executable(inf inf.c)
add_executable(mem mem.c)


#add_custom_target(test1 ALL COMMAND ./main polecenia 100 100)
#add_dependencies(test1 main break inf mem)
