cmake_minimum_required(VERSION 2.8)
include(${CMAKE_SOURCE_DIR}/xenonapp.txt)

add_executable(decode decode.cpp)
target_link_libraries(decode xenon)

add_executable(sibs sibs.cpp)
target_link_libraries(sibs xenon)

add_executable(tut tutorial.cpp)
target_link_libraries(tut xenon)

add_executable(tut2 tutorial2.cpp)
target_link_libraries(tut2 xenon)
