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
)

# Add include directories
target_include_directories(main PRIVATE lib/hardware)

# --- Add SSD1306 / local lib C sources so symbols get linked ---
# Prefer lib/hardware/ssd1306.c, fallback to lib/pico-ssd1306/ssd1306.c,
# else add any .c under lib/ recursively.
set(SSD1306_PRIMARY "${CMAKE_SOURCE_DIR}/lib/hardware/ssd1306.c")
set(SSD1306_ALT     "${CMAKE_SOURCE_DIR}/lib/pico-ssd1306/ssd1306.c")

if(EXISTS "${SSD1306_PRIMARY}")
    target_sources(main PRIVATE "${SSD1306_PRIMARY}")
elseif(EXISTS "${SSD1306_ALT}")
    target_include_directories(main PRIVATE "${CMAKE_SOURCE_DIR}/lib/pico-ssd1306")
    target_sources(main PRIVATE "${SSD1306_ALT}")
else()
    file(GLOB_RECURSE LIB_C_SRC "${CMAKE_SOURCE_DIR}/lib/*.c")
    if(LIB_C_SRC)
        target_sources(main PRIVATE ${LIB_C_SRC})
    else()
        message(WARNING "No SSD1306 C implementation found. Put ssd1306.c in lib/hardware or lib/pico-ssd1306, or add your C sources under lib/")
    endif()
endif()
# --- end SSD1306 / lib inclusion ---

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