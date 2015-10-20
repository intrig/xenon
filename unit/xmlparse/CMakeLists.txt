cmake_minimum_required(VERSION 2.8)
include(${CMAKE_SOURCE_DIR}/xenonunit.txt)
enable_testing()
add_executable(xmlparse xmlparseunit.cpp)
target_link_libraries(xmlparse xenon)
add_test(NAME xmlparse 
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND xmlparse)
