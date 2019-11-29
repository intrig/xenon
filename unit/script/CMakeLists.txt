cmake_minimum_required(VERSION 3.11)
enable_testing()

add_executable(script scriptunit.cpp)
target_link_libraries(script PUBLIC xenon)

add_test(NAME script COMMAND script)
