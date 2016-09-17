cmake_minimum_required(VERSION 2.8)
include(${CMAKE_SOURCE_DIR}/xenonapp.txt)
include_directories(lua-5.1.4/src)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/o) 

add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/include/xenon/xddl.h ${CMAKE_SOURCE_DIR}/src/xddl.cpp
    COMMAND xspx -H t -S xddl.cpp  ${CMAKE_CURRENT_SOURCE_DIR}/xddl.xspx
    COMMAND ${CMAKE_COMMAND} -E copy_if_different t ${CMAKE_SOURCE_DIR}/include/xenon/xddl.h
    COMMAND ${CMAKE_COMMAND} -E copy_if_different xddl.cpp ${CMAKE_SOURCE_DIR}/src/xddl.cpp
    DEPENDS xspx
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/xddl.xspx
    VERBATIM)

add_library(xenon ${CMAKE_SOURCE_DIR}/include/xenon/xddl.h lua-5.1.4/src/lua_all.cpp xml_parser_base.cpp message.cpp xenon.cpp xddl.cpp)

set_target_properties(xenon PROPERTIES POSITION_INDEPENDENT_CODE ON)
