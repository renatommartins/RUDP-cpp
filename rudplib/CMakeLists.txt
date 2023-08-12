add_library(rudplib STATIC
        Packet.cpp
        Server.cpp
        Client.cpp
        UdpTranceiver.cpp
        WinSock2Support.cpp)

if(WIN32)
target_sources(rudplib PRIVATE WinSock2Support.cpp)
endif ()

target_compile_features(rudplib PRIVATE cxx_std_20)

target_include_directories(rudplib PRIVATE include/rudplib)
target_include_directories(rudplib INTERFACE include)

if(WIN32)
    target_link_libraries(rudplib PRIVATE ws2_32)
endif ()

target_link_libraries(rudplib PRIVATE
        utils)