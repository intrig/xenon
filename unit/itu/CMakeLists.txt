cmake_minimum_required(VERSION 3.12)
enable_testing()

add_executable(itu ituunit.cpp)
target_link_libraries(itu PUBLIC xenon)

add_test(NAME itu COMMAND itu)
