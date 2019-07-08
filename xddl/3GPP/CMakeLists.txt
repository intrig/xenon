cmake_minimum_required(VERSION 2.8)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/TS-36.331.xddl
    DEPENDS 36.331.asn 36.331-sub.txt asnx xml-patch
    COMMAND asnx -t -o 36.331-temp.xddl ${CMAKE_CURRENT_SOURCE_DIR}/36.331.asn
    COMMAND xml-patch -o ${CMAKE_CURRENT_SOURCE_DIR}/TS-36.331.xddl 36.331-temp.xddl ${CMAKE_CURRENT_SOURCE_DIR}/36.331-sub.txt
)

add_custom_target(36_331 ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/TS-36.331.xddl)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/TS-25.331.xddl
    DEPENDS 25.331.asn 25.331-sub.txt asnx xml-patch
    COMMAND asnx -t -o 25.331-temp.xddl ${CMAKE_CURRENT_SOURCE_DIR}/25.331.asn
    COMMAND xml-patch -o ${CMAKE_CURRENT_SOURCE_DIR}/TS-25.331.xddl 25.331-temp.xddl ${CMAKE_CURRENT_SOURCE_DIR}/25.331-sub.txt
)

add_custom_target(25_331 ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/TS-25.331.xddl)
