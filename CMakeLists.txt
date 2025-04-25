cmake_minimum_required(VERSION 3.13)

# Lade pico_sdk_import.cmake aus dem Projektverzeichnis
include(pico_sdk_import.cmake)

project(midipico C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

# Pfade zu den Bibliotheken (relativ zu CMAKE_CURRENT_SOURCE_DIR)
set(USB_MIDI_HOST_PATH ${CMAKE_CURRENT_SOURCE_DIR}/usb_midi_host)
set(PIO_MIDI_UART_PATH ${CMAKE_CURRENT_SOURCE_DIR}/pio_midi_uart_lib)
set(PIO_USB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/pico-sdk/lib/pico_pio_usb)

# Prüfe, ob Bibliotheksverzeichnisse existieren
if (NOT EXISTS ${USB_MIDI_HOST_PATH})
    message(FATAL_ERROR "usb_midi_host directory not found at ${USB_MIDI_HOST_PATH}")
endif()
if (NOT EXISTS ${PIO_MIDI_UART_PATH})
    message(FATAL_ERROR "pio_midi_uart_lib directory not found at ${PIO_MIDI_UART_PATH}")
endif()
if (NOT EXISTS ${PIO_USB_PATH})
    message(FATAL_ERROR "pico_pio_usb directory not found at ${PIO_USB_PATH}")
endif()

# Erstelle das ausführbare Programm
add_executable(midipico
    main.c
    pio_usb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/pico-sdk/lib/tinyusb/src/tusb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/pico-sdk/lib/tinyusb/src/common/tusb_fifo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/pico-sdk/lib/tinyusb/src/portable/raspberrypi/rp2040/dcd_rp2040.c
    ${CMAKE_CURRENT_SOURCE_DIR}/pico-sdk/lib/tinyusb/src/portable/raspberrypi/rp2040/hcd_rp2040.c
    ${CMAKE_CURRENT_SOURCE_DIR}/pico-sdk/lib/tinyusb/src/portable/raspberrypi/rp2040/rp2040_usb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/pico-sdk/lib/tinyusb/src/class/midi/midi_device.c
    ${CMAKE_CURRENT_SOURCE_DIR}/pico-sdk/lib/tinyusb/src/device/usbd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/pico-sdk/lib/tinyusb/src/host/usbh.c
    ${CMAKE_CURRENT_SOURCE_DIR}/pico-sdk/lib/tinyusb/src/device/usbd_control.c
    ${CMAKE_CURRENT_SOURCE_DIR}/pico-sdk/lib/tinyusb/src/host/usbh_control.c
)

# Generiere PIO-Header für usb_tx.pio
pico_generate_pio_header(midipico ${CMAKE_CURRENT_SOURCE_DIR}/usb_tx.pio)

# Include-Verzeichnisse für Bibliotheken
target_include_directories(midipico PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${USB_MIDI_HOST_PATH}
    ${PIO_MIDI_UART_PATH}
    ${PIO_USB_PATH}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/pico-sdk/lib/tinyusb/src
)

# Debugging: Include-Pfade ausgeben
get_target_property(INCLUDE_DIRS midipico INCLUDE_DIRECTORIES)
message("Include-Pfade für midipico: ${INCLUDE_DIRS}")

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
)

pico_enable_stdio_usb(midipico 0)
pico_enable_stdio_uart(midipico 1)
pico_add_extra_outputs(midipico)
