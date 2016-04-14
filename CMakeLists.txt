cmake_minimum_required(VERSION 2.8)

if(WIN32)
#set(CMAKE_PREFIX_PATH C:/cygwin64/home/games/work/xenon/win) 
set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/win) 

endif()

add_subdirectory(tools)
add_subdirectory(src)
add_subdirectory(unit)
add_subdirectory(xddl)
add_subdirectory(examples)
enable_testing()
