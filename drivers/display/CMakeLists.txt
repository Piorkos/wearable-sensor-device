add_library(display display.cpp sharp_mip_display.cpp)

target_link_libraries(display
    pico_stdlib
    hardware_spi
)