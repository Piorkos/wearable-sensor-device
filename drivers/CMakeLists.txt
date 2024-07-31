add_library(drivers lsm303d.cpp pa1010d.cpp)

target_link_libraries(drivers
    pico_stdlib
    hardware_i2c
)