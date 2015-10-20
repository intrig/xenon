cmake_minimum_required(VERSION 2.8)
include(${CMAKE_SOURCE_DIR}/xenonunit.txt)
enable_testing()
add_executable(spec specunit.cpp fieldtypes.cpp)
target_link_libraries(spec xenon)
add_test(NAME spec
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND spec)
