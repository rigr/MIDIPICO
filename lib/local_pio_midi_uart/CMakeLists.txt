cmake_minimum_required(VERSION 3.13)

add_library(pio_midi_uart_lib STATIC
    pio_midi_uart_lib.c
    pio_midi_uart.pio
)
target_include_directories(pio_midi_uart_lib PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(pio_midi_uart_lib PUBLIC
    pico_stdlib
    hardware_pio
    hardware_gpio
    ring_buffer_lib
)
pico_generate_pio_header(pio_midi_uart_lib ${CMAKE_CURRENT_LIST_DIR}/pio_midi_uart.pio)