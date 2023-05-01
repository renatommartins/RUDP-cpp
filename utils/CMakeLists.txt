add_library(utils
        crc32.cpp
        portability.cpp)

target_include_directories(utils PRIVATE include/utils)
target_include_directories(utils INTERFACE include)