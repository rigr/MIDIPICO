cmake_minimum_required(VERSION 3.13)

# Lade pico_sdk_import.cmake aus dem Projektverzeichnis
include(pico_sdk_import.cmake)

project(midipico C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

# Pfade zu den Bibliotheken
set(USB_MIDI_HOST_PATH ${CMAKE_CURRENT_SOURCE_DIR}/usb_midi_host)
set(PIO_MIDI_UART_PATH ${CMAKE_CURRENT_SOURCE_DIR}/pio_midi_uart_lib)
set(PICO_PIO_USB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/pico-sdk/lib/pico_pio_usb)

# Prüfe, ob Bibliotheksverzeichnisse existieren
if (NOT EXISTS ${USB_MIDI_HOST_PATH})
    message(FATAL_ERROR "usb_midi_host directory not found at ${USB_MIDI_HOST_PATH}")
endif()
if (NOT EXISTS ${PIO_MIDI_UART_PATH})
    message(FATAL_ERROR "pio_midi_uart_lib directory not found at ${PIO_MIDI_UART_PATH}")
endif()
if (NOT EXISTS ${PICO_PIO_USB_PATH})
    message(FATAL_ERROR "pico_pio_usb directory not found at ${PICO_PIO_USB_PATH}")
endif()

add_executable(midipico
    main.c
)

# Include-Verzeichnisse für Bibliotheken
target_include_directories(midipico PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${USB_MIDI_HOST_PATH}
    ${PIO_MIDI_UART_PATH}
    ${PICO_PIO_USB_PATH}
    ${CMAKE_CURRENT_LIST_DIR} # Für tusb_config.h
)

# Bibliotheken hinzufügen
add_subdirectory(${USB_MIDI_HOST_PATH} usb_midi_host)
add_subdirectory(${PIO_MIDI_UART_PATH} pio_midi_uart_lib)
add_subdirectory(${PICO_PIO_USB_PATH} pico_pio_usb)

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
