add_library(components buttons_controller.cpp screen_controller.cpp tracker.cpp)

# add_subdirectory(../drivers)
target_link_libraries(components
    pico_stdlib
    drivers)