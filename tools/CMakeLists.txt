cmake_minimum_required(VERSION 3.12)
add_executable(idm idm.cpp)
target_link_libraries(idm PUBLIC xenon)

add_executable(xml-patch xmlpatch.cpp)
target_link_libraries(xml-patch PUBLIC xenon)

add_executable(xml-pp  xmlpp.cpp)
target_link_libraries(xml-pp PUBLIC xenon)

add_executable(xv xv.cpp)
target_link_libraries(xv PUBLIC xenon)

# Only process asn.1 on Linux.
# TODO: Move flex/bison dependencies here
if (NOT APPLE AND NOT MSVC)
    add_subdirectory(asnx)

    add_executable(asn-strip asnstrip.cpp)
    target_link_libraries(asn-strip PUBLIC xenon -lstdc++fs)

    # We're using experimental/filesytems and it doesn't work on Travis currently.
    # TODO: eventually remove this dependency
    add_executable(procadoc procadoc.cpp)
    target_link_libraries(procadoc PUBLIC xenon -lstdc++fs)
endif()
