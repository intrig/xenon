cmake_minimum_required(VERSION 3.11)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

add_executable(decodeperf decode.cpp)
target_link_libraries(decodeperf PUBLIC xenon ${FSLIB})

add_executable(loadperf loadperf.cpp)
target_link_libraries(loadperf PUBLIC xenon ${FSLIB})

endif()
