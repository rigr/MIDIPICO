cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(midipico C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()
add_subdirectory(lib/tinyusb)
add_executable(midipico
    main.c
    pio_midi_uart.c
)
target_include_directories(midipico PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(midipico pico_stdlib pico_multicore hardware_i2c hardware_adc hardware_pio tinyusb_host)
pico_add_extra_outputs(midipico)
pico_generate_pio_header(midipico ${CMAKE_CURRENT_LIST_DIR}/midi_uart.pio)
