cmake_minimum_required(VERSION 3.2)
project(rudplib)

set(CMAKE_CXX_STANDARD 20)

if(WIN32)
    target_link_libraries(rudplib PRIVATE ws2_32)
endif ()

add_subdirectory(rudplib)
add_subdirectory(utils)
add_subdirectory(example01)