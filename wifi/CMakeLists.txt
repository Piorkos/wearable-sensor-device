add_library(wifi 
    WiFi.cpp
    utility/wifi_drv.cpp
    utility/spi_drv.cpp)

target_link_libraries(wifi 
    pico_stdlib
    hardware_spi)