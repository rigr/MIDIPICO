cmake_minimum_required(VERSION 3.13)

# Pfad zum pico-sdk setzen (wird im Workflow definiert)
set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
include(${PICO_SDK_PATH}/pico_sdk_import.cmake)

project(midipico C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

# Pfade zu den Bibliotheken
set(USB_MIDI_HOST_PATH ${CMAKE_SOURCE_DIR}/usb_midi_host)
set(PIO_MIDI_UART_PATH ${CMAKE_SOURCE_DIR}/pio_midi_uart_lib)

add_executable(midipico
    main.c
)

# Include-Verzeichnisse für Bibliotheken
target_include_directories(midipico PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${USB_MIDI_HOST_PATH}
    ${PIO_MIDI_UART_PATH}
)

# Bibliotheken hinzufügen
add_subdirectory(${USB_MIDI_HOST_PATH} usb_midi_host)
add_subdirectory(${PIO_MIDI_UART_PATH} pio_midi_uart_lib)

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
