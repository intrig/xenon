cmake_minimum_required(VERSION 2.8)
include(${CMAKE_SOURCE_DIR}/xenonapp.txt)

find_package(BISON)
find_package(FLEX)

BISON_TARGET(MyParser asn1.ypp ${CMAKE_CURRENT_BINARY_DIR}/asn1.tab.cc)
FLEX_TARGET(MyScanner asn1.l  ${CMAKE_CURRENT_BINARY_DIR}/lexer.cc)
ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_executable(asnx
  asnast.cpp
  ${BISON_MyParser_OUTPUTS}
  ${FLEX_MyScanner_OUTPUTS})
target_link_libraries(asnx xenon)

