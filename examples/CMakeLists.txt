cmake_minimum_required(VERSION 3.12)

add_executable(decode decode.cpp)
target_link_libraries(decode PUBLIC xenon)

add_executable(sibs sibs.cpp)
target_link_libraries(sibs PUBLIC xenon)

add_executable(tutorial tutorial.cpp)
target_link_libraries(tutorial PUBLIC xenon)

add_executable(tutorial2 tutorial2.cpp)
target_link_libraries(tutorial2 PUBLIC xenon)
