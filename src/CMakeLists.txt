cmake_minimum_required(VERSION 3.11)

set (SRC ${CMAKE_CURRENT_SOURCE_DIR})

# This is the lynchpin rule to the entire system.  To build the library below,
# we must first generate the xddl.cpp and xddl.h files, which depend on the xspx
# program and the xddl.xspx file.
add_custom_command(
    OUTPUT xddl.cpp ../include/xenon/xddl.h
    COMMAND xspx -H ../include/xenon/xddl.h -S xddl.cpp ${SRC}/xddl.xspx
    COMMENT generating xddl.cpp
    DEPENDS xspx xddl.xspx
)

if (MSVC)
add_library(xenon STATIC
    lua-5.1.4/src/lua_all.cpp
    xml_parser_base.cpp 
    message.cpp
    xenon.cpp
    xddl.cpp
)
else()
add_library(xenon SHARED
    lua-5.1.4/src/lua_all.cpp
    xml_parser_base.cpp
    message.cpp
    xenon.cpp
    xddl.cpp
	)
endif()

if (NOT MSVC)
    target_compile_options(xenon
        PUBLIC -Wall -Wextra -Werror -fdiagnostics-color=always
            -Wno-unneeded-internal-declaration -Wno-implicit-fallthrough)
endif ()

target_include_directories(xenon
    PUBLIC ${CMAKE_SOURCE_DIR}/include
           ${CMAKE_BINARY_DIR}/include
    PRIVATE lua-5.1.4/src
)

install(TARGETS xenon
    DESTINATION lib)
