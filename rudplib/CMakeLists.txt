add_library(rudplib OBJECT
        Packet.cpp
        Server.cpp
        Client.cpp
        UdpTranceiver.cpp
        MockTranceiver.cpp
        NetworkEndpoint.cpp)

target_compile_features(rudplib PRIVATE cxx_std_23)
set_property(TARGET rudplib PROPERTY COMPILE_WARNING_AS_ERROR ON)

target_include_directories(rudplib PRIVATE include/rudplib)
target_include_directories(rudplib INTERFACE include)

if(WIN32)
    target_link_libraries(rudplib PRIVATE ws2_32)
endif ()

target_link_libraries(rudplib PUBLIC
        utils)