cmake_minimum_required(VERSION 2.8)
enable_testing()
add_test(NAME basic
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND xv basic.xv)
add_test(NAME brass
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND xv --xddl_path=${CMAKE_SOURCE_DIR}/xddl icd_brass.xv)
add_test(NAME gold
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND xv --xddl_path=${CMAKE_SOURCE_DIR}/xddl icd_gold.xv)
