cmake_minimum_required(VERSION 2.8)
include(${CMAKE_SOURCE_DIR}/xenonunit.txt)

add_executable(recref recref_unit.cpp)
add_test(recref recref)

add_executable(msg message_unit.cpp)
target_link_libraries(msg xenon)
add_test(NAME msg 
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND msg)

add_subdirectory(itu)
add_subdirectory(xmlparse)
add_subdirectory(document)
add_subdirectory(script)

add_subdirectory(xddlunit)
