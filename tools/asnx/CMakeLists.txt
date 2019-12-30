cmake_minimum_required(VERSION 3.11)

find_package(BISON)
find_package(FLEX)

if (BISON_FOUND AND FLEX_FOUND)
    BISON_TARGET(MyParser asn1.ypp ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)
    FLEX_TARGET(MyScanner asn1.l  ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)
    ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

    include_directories(${CMAKE_CURRENT_BINARY_DIR})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR})

    add_executable(asnx
      asnast.cpp
      ${BISON_MyParser_OUTPUTS}
      ${FLEX_MyScanner_OUTPUTS}
    )

    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

        target_compile_options(asnx PRIVATE
            -Weverything
            -Werror
        )
        # Remove all warnings from lex and yack fiels
        set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp
            PROPERTIES COMPILE_OPTIONS
            -Wno-everything)
        set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
            PROPERTIES COMPILE_OPTIONS
            -Wno-everything)
    endif()
    target_link_libraries(asnx PUBLIC xenon)
endif()
