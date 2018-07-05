add_executable(main main.c)
add_executable(gen filter_generator.c)
target_link_libraries(main pthread)
target_link_libraries(main m)
