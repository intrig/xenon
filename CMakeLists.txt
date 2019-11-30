cmake_minimum_required(VERSION 3.11)
project(xenon VERSION 2.3.0)
configure_file(config.h.in include/xenon/config.h)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (APPLE)
    include_directories(SYSTEM "/usr/local/include")
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(FSLIB -lstdc++fs)
endif ()

add_subdirectory(xspx)
add_subdirectory(src)
add_subdirectory(tools)
add_subdirectory(unit)
add_subdirectory(examples)
add_subdirectory(perf)

#set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)

install(DIRECTORY include/xenon
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
    PATTERN "ict/*/*" EXCLUDE
)

install(DIRECTORY ${CMAKE_BINARY_DIR}/include/xenon
    DESTINATION include
)

install(DIRECTORY xddl
    DESTINATION share
    FILES_MATCHING PATTERN "*.xddl"
)
