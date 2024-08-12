add_library(drivers lsm303d.cpp pa1010d.cpp storage.cpp lsm6dsox.cpp)

target_link_libraries(drivers
    pico_stdlib
    hardware_i2c
    hardware_flash
    hardware_sync
)