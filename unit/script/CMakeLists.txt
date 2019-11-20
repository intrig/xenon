cmake_minimum_required(VERSION 3.12)
enable_testing()

add_executable(script scriptunit.cpp)
target_link_libraries(script PUBLIC xenon)

add_test(NAME script COMMAND script)
