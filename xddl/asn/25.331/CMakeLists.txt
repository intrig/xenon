cmake_minimum_required(VERSION 2.8)
add_definitions(-std=c++11)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/../../3GPP/TS-25.331.xddl
    DEPENDS 25.331.asn 25.331-sub.txt asnx xml-patch
    COMMAND asnx -t -o temp.xddl ${CMAKE_CURRENT_SOURCE_DIR}/25.331.asn
    COMMAND xml-patch -o${CMAKE_CURRENT_SOURCE_DIR}/../../3GPP/TS-25.331.xddl temp.xddl ${CMAKE_CURRENT_SOURCE_DIR}/25.331-sub.txt
)

add_custom_target(25_331 ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../../3GPP/TS-25.331.xddl)
