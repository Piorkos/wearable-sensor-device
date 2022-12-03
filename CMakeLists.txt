add_executable(running_data_collector
        main.cpp
        )

# pull in common dependencies
target_link_libraries(running_data_collector 
        pico_stdlib 
        hardware_flash
        hardware_sync
        )

# enable usb output, disable uart output
pico_enable_stdio_usb(running_data_collector 1)
pico_enable_stdio_uart(running_data_collector 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(running_data_collector)

# add url via pico_set_program_url
example_auto_set_url(running_data_collector)


add_subdirectory(pico-ssd1306)
target_link_libraries(running_data_collector
  pico_ssd1306
  # you will also need hardware i2c library for communication with the display
  hardware_i2c)