add_library(lsm6dsox lsm6dsox.cpp)

target_link_libraries(lsm6dsox
    pico_stdlib
    hardware_i2c
)