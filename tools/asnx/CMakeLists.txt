cmake_minimum_required(VERSION 3.12)

find_package(BISON)
find_package(FLEX)

BISON_TARGET(MyParser asn1.ypp ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)
FLEX_TARGET(MyScanner asn1.l  ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)
ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_compile_options(-Wno-unneeded-internal-declaration)
add_executable(asnx
  asnast.cpp
  ${BISON_MyParser_OUTPUTS}
  ${FLEX_MyScanner_OUTPUTS}
)
target_link_libraries(asnx PUBLIC xenon)
