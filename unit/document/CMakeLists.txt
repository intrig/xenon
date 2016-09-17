cmake_minimum_required(VERSION 2.8)
include(${CMAKE_SOURCE_DIR}/xenonunit.txt)
enable_testing()
add_executable(spec specunit.cpp fieldtypes.cpp)
target_link_libraries(spec xenon)
add_test(NAME spec
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND spec)

find_package(Boost 1.54.0 COMPONENTS system filesystem)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    message("adding load test")
    add_executable(load loadunit.cpp)
    target_link_libraries(load xenon)
    target_link_libraries(load ${Boost_LIBRARIES})
    add_test(NAME load
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND load)
endif()
