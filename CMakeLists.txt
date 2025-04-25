cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(midipico C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(midipico
    main.c
)

target_include_directories(midipico PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(midipico
    pico_stdlib
    pico_multicore
    tinyusb_device
    tinyusb_host
    usb_midi_host
    pio_midi_uart_lib
    pico_pio_usb
)

pico_enable_stdio_usb(midipico 0)
pico_enable_stdio_uart(midipico 1)
pico_add_extra_outputs(midipico)
