add_library(utils STATIC
        crc32.cpp
        portability.cpp
        time.cpp)

target_compile_features(utils PRIVATE cxx_std_23)

target_include_directories(utils PRIVATE include/utils)
target_include_directories(utils INTERFACE include)