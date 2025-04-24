cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(midipico C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(midipico
    src/main.c
    src/usb_descriptors.c
    src/midi_routing.c
)

target_include_directories(midipico PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/lib/usb_midi_host
    ${CMAKE_CURRENT_LIST_DIR}/lib/pio_midi_uart_lib
    ${CMAKE_CURRENT_LIST_DIR}/lib/ring_buffer_lib
    ${CMAKE_CURRENT_LIST_DIR}/lib/midi_device_multistream
)

target_link_libraries(midipico PRIVATE
    pico_stdlib
    pico_multicore
    hardware_pio
    tinyusb_device
    tinyusb_host
    usb_midi_host
    pio_midi_uart_lib
    ring_buffer_lib
    midi_device_multistream
)

pico_add_extra_outputs(midipico)
pico_enable_stdio_usb(midipico 0)
pico_enable_stdio_uart(midipico 1)
