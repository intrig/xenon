cmake_minimum_required(VERSION 3.12)
enable_testing()

add_executable(spec specunit.cpp fieldtypes.cpp)
target_link_libraries(spec PUBLIC xenon)

add_test(NAME spec COMMAND spec
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

if (NOT APPLE AND NOT MSVC)
    add_executable(load loadunit.cpp)
    target_link_libraries(load PUBLIC xenon -lstdc++fs)

    add_test(NAME load_test COMMAND load
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()
