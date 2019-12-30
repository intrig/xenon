cmake_minimum_required(VERSION 3.11)

set (xddl_h ${CMAKE_BINARY_DIR}/include/xenon/xddl.h)
set (xddl_cpp ${CMAKE_CURRENT_BINARY_DIR}/xddl.cpp)
set (xddl_xspx ${CMAKE_CURRENT_SOURCE_DIR}/xddl.xspx)

find_program(
    clang-format
    NAMES "clang-format")

if (clang-format)
    message(STATUS "clang-format found: ${clang-format}")
    set (format-command ${clang-format} -i ${xddl_h} ${xddl_cpp} ${xddl_xspx})
else()
    message(STATUS "clang-format NOT found")
    set (format-command "")
endif()

# This is the lynchpin to the entire system.  To build the library below,
# we must first generate the xddl.cpp and xddl.h files, which depend on the xspx
# program and the xddl.xspx file.

add_custom_command(
    OUTPUT ${xddl_cpp} ${xddl_h}
    COMMAND xspx -H ${xddl_h} -S ${xddl_cpp} ${xddl_xspx}
    COMMAND ${format-command}
    DEPENDS xspx ${xddl_xspx}
    )

if (MSVC)
    set (lib-type STATIC)
else()
    set (lib-type SHARED)
endif()

add_library(xenon ${lib-type}
    lua-5.1.4/src/lua_all.cpp
    xml_parser_base.cpp 
    message.cpp
    xenon.cpp
    xddl.cpp
    ${xddl_h}
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(xenon PUBLIC
        -Wall -Wextra -Wpedantic -fdiagnostics-color=always
        -Wno-implicit-fallthrough
        -Werror
    )
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(xenon PUBLIC
        -fdiagnostics-color=always
        -Weverything
        -Wno-implicit-fallthrough -Wno-keyword-macro
        -Wno-unneeded-internal-declaration
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        -Wno-shadow-field-in-constructor
        -Wno-sign-conversion
        -Wno-implicit-float-conversion

        # These are more informational
        -Wno-padded
        -Wno-weak-vtables
        -Wno-exit-time-destructors
        -Wno-global-constructors
        -Wno-switch-enum

        -Werror
    )
    # Remove all warnings from lua source
    set_source_files_properties(lua-5.1.4/src/lua_all.cpp PROPERTIES COMPILE_OPTIONS
        -Wno-everything)
endif()


target_include_directories(xenon
    PUBLIC ${CMAKE_SOURCE_DIR}/include
           ${CMAKE_BINARY_DIR}/include
    PRIVATE lua-5.1.4/src
)

install(TARGETS xenon
    DESTINATION lib)
