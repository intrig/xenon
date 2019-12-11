cmake_minimum_required(VERSION 3.11)
enable_testing()

add_test(
    NAME basic
    COMMAND xenon-xv --xddl_path="${CMAKE_SOURCE_DIR}/xddl" basic.xv
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_test(
    NAME brass
    COMMAND xenon-xv --xddl_path="${CMAKE_SOURCE_DIR}/xddl" icd_brass.xv
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_test(
    NAME gold
    COMMAND xenon-xv --xddl_path="${CMAKE_SOURCE_DIR}/xddl" icd_gold.xv
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(gold
    PROPERTIES
    ENVIRONMENT XDDL_PATH=${CMAKE_SOURCE_DIR}/xddl
)
