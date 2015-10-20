cmake_minimum_required(VERSION 2.8)
add_definitions(-std=c++11)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/../../3GPP/TS-36.331.xddl
    DEPENDS 36.331.asn substitute.txt asnx xml-patch
    COMMAND asnx -t ${CMAKE_CURRENT_SOURCE_DIR}/36.331.asn > temp.xddl
    COMMAND xml-patch temp.xddl ${CMAKE_CURRENT_SOURCE_DIR}/substitute.txt > ${CMAKE_CURRENT_SOURCE_DIR}/../../3GPP/TS-36.331.xddl
)

add_custom_target(36_331 ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../../3GPP/TS-36.331.xddl)
