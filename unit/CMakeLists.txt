cmake_minimum_required(VERSION 3.12)
enable_testing()

add_executable(recref recref_unit.cpp)
target_link_libraries(recref PUBLIC xenon)

add_executable(message message_unit.cpp)
target_link_libraries(message PUBLIC xenon)

add_test(NAME recref COMMAND recref)
add_test(NAME message COMMAND message
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(itu)
add_subdirectory(xmlparse)
add_subdirectory(document)
add_subdirectory(script)
add_subdirectory(xddlunit)
