add_library(rudplib
        packet.cpp
        server.cpp
        client.cpp)

target_include_directories(rudplib PRIVATE include/rudplib)
target_include_directories(rudplib INTERFACE include)

if(WIN32)
    target_link_libraries(rudplib PRIVATE ws2_32)
endif ()

target_link_libraries(rudplib PRIVATE
        utils)