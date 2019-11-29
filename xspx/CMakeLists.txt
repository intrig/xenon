cmake_minimum_required(VERSION 3.11)

add_executable(xspx
    xspx.cpp
    xspx_parser.cpp
    ../src/xml_parser_base.cpp)

target_include_directories(xspx PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/include
    )
