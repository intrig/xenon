cmake_minimum_required(VERSION 2.8)
include (${CMAKE_SOURCE_DIR}/xenonapp.txt)
add_executable(xspx xspx.cpp xspx_parser.cpp ${CMAKE_SOURCE_DIR}/src/xml_parser_base.cpp)
