cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(main C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
pico_sdk_init()

add_executable(main
        main.cpp
        lib/hardware/button.cpp
        lib/hardware/oled.cpp
        lib/software/serializer.cpp
        lib/software/hex.cpp
)

pico_enable_stdio_usb(main 1)

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(main pico_stdlib)
target_link_libraries(main pico_multicore)
target_link_libraries(main pico_stdio_usb)
target_link_libraries(main hardware_gpio)
target_link_libraries(main hardware_adc)
target_link_libraries(main hardware_pwm)
target_link_libraries(main hardware_uart)
target_link_libraries(main hardware_spi)
target_link_libraries(main hardware_i2c)
target_link_libraries(main hardware_irq)
target_link_libraries(main hardware_timer)

# create map/bin/hex file etc.
pico_add_extra_outputs(main)