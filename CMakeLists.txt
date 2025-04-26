cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(midipico C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()
add_subdirectory(lib/tinyusb/src)
add_subdirectory(lib/pio_midi_uart_lib)
add_subdirectory(lib/ring_buffer_lib)
add_executable(midipico
    main.c
)
target_include_directories(midipico PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/lib/pio_midi_uart_lib
    ${CMAKE_CURRENT_LIST_DIR}/lib/ring_buffer_lib
    ${CMAKE_CURRENT_LIST_DIR}/lib/tinyusb/src
)
target_link_libraries(midipico
    pico_stdlib
    pico_multicore
    hardware_i2c
    hardware_adc
    hardware_pio
    tinyusb_host
    tinyusb_device
    pio_midi_uart_lib
    ring_buffer_lib
)
pico_add_extra_outputs(midipico)
pico_generate_pio_header(midipico ${CMAKE_CURRENT_LIST_DIR}/midi_uart.pio)
