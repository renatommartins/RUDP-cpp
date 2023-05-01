add_executable(example01 main.cpp)

target_include_directories(example01 PRIVATE include/example01)

target_link_libraries(example01 PRIVATE rudplib)