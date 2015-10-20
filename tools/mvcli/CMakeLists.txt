cmake_minimum_required(VERSION 2.8)
include(${CMAKE_SOURCE_DIR}/xenonapp.txt)
add_executable(mvcli mvcli.cpp)
target_link_libraries(mvcli xenon)
