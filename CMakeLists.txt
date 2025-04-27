cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(midipico C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

# Subdirectories
add_subdirectory(lib/tinyusb/src)
add_subdirectory(lib/local_pio_midi_uart)
add_subdirectory(lib/ring_buffer_lib)

# Executable
add_executable(midipico
    main.c
)

# Include directories
target_include_directories(midipico PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/lib/local_pio_midi_uart
    ${CMAKE_CURRENT_LIST_DIR}/lib/ring_buffer_lib
    ${CMAKE_CURRENT_LIST_DIR}/lib/tinyusb/src
)

# Link libraries
target_link_libraries(midipico PUBLIC
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

# Explicit dependency
add_dependencies(midipico pio_midi_uart_lib)

# Generate PIO header
pico_generate_pio_header(midipico ${CMAKE_CURRENT_LIST_DIR}/lib/local_pio_midi_uart/pio_midi_uart.pio)

# Generate extra outputs
pico_add_extra_outputs(midipico)
