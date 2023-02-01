add_executable(halikOS
        main.cpp
        )

# pull in common dependencies
target_link_libraries(halikOS 
        pico_stdlib 
        hardware_flash
        hardware_sync
        )

# enable usb output, disable uart output
pico_enable_stdio_usb(halikOS 1)
pico_enable_stdio_uart(halikOS 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(halikOS)

# add url via pico_set_program_url
example_auto_set_url(halikOS)


add_subdirectory(pico-ssd1306)
add_subdirectory(lsm6dsox)
target_link_libraries(halikOS
  pico_ssd1306
  lsm6dsox
  hardware_spi
  hardware_i2c)
