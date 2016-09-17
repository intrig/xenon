cmake_minimum_required(VERSION 2.8)
include(${CMAKE_SOURCE_DIR}/xenonapp.txt)

# Tools with just one source file are in this directory.  Otherwise they are in a subdirectory.
add_subdirectory(xspx)
add_subdirectory(asnx)

add_executable(asn-strip asnstrip.cpp)
target_link_libraries(asn-strip xenon)

add_executable(idm idm.cpp)
target_link_libraries(idm xenon)

add_executable(xml-patch xmlpatch.cpp)
target_link_libraries(xml-patch xenon)

add_executable(xml-pp xmlpp.cpp)
target_link_libraries(xml-pp xenon)

add_executable(xv xv.cpp)
target_link_libraries(xv xenon)

add_executable(recombobulator recombobulator.cpp)
target_link_libraries(recombobulator xenon)
