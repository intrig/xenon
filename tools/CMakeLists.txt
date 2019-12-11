cmake_minimum_required(VERSION 3.11)
add_executable(xenon-dm idm.cpp)
target_link_libraries(xenon-dm PUBLIC xenon)

add_executable(xml-patch xmlpatch.cpp)
target_link_libraries(xml-patch PUBLIC xenon)

add_executable(xml-pp  xmlpp.cpp)
target_link_libraries(xml-pp PUBLIC xenon)

add_executable(xenon-xv xv.cpp)
target_link_libraries(xenon-xv PUBLIC xenon)

add_executable(recombobulator recombobulator.cpp)
target_link_libraries(recombobulator PUBLIC xenon)

add_subdirectory(asnx)

if (NOT APPLE AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
add_executable(asn-strip asnstrip.cpp)
target_link_libraries(asn-strip PUBLIC xenon ${FSLIB})

add_executable(procadoc procadoc.cpp)
target_link_libraries(procadoc PUBLIC xenon ${FSLIB})
endif()
install(TARGETS xenon-dm xenon-xv DESTINATION bin)
