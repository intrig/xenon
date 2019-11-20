cmake_minimum_required(VERSION 3.12)
enable_testing()

add_executable(xmlparse xmlparseunit.cpp)
target_link_libraries(xmlparse PUBLIC xenon)

add_test(NAME xmlparse COMMAND xmlparse
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
