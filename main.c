#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "tusb.h"
#include "usb_midi_host.h"
#include "pio_midi_uart_lib.h"
#include "pico_pio_usb.h" // Angepasst: Verwende lokales include/-Verzeichnis

// Pin-Zuweisungen
#define DIN_MIDI_RX_1 6  // GPIO 6 für DIN MIDI In 1
#define DIN_MIDI_RX_2 7  // GPIO 7 für DIN MIDI In 2
#define DIN_MIDI_RX_3 8  // GPIO 8 für DIN MIDI In 3
#define DIN_MIDI_RX_4 9  // GPIO 9 für DIN MIDI In 4

#define USB_HOST_DP_1 10 // GPIO 10 für USB Host 1 D+
#define USB_HOST_DM_1 11 // GPIO 11 für USB Host 1 D-
#define USB_HOST_DP_2 12 // GPIO 12 für USB Host 2 D+
#define USB_HOST_DM_2 13 // GPIO 13 für USB Host 2 D-
#define USB_HOST_DP_3 14 // GPIO 14 für USB Host 3 D+
#define USB_HOST_DM_3 15 // GPIO 15 für USB Host 3 D-
#define USB_HOST_DP_4 16 // GPIO 16 für USB Host 4 D+
#define USB_HOST_DM_4 17 // GPIO 17 für USB Host 4 D-

// MIDI-Datenpuffer
#define MIDI_BUFFER_SIZE 128
uint8_t midi_buffer[MIDI_BUFFER_SIZE];

// USB Host Konfiguration
static usb_host_config_t host_config = {
    .dp_pins = {USB_HOST_DP_1, USB_HOST_DP_2, USB_HOST_DP_3, USB_HOST_DP_4},
    .dm_pins = {USB_HOST_DM_1, USB_HOST_DM_2, USB_HOST_DM_3, USB_HOST_DM_4},
    .num_ports = 4,
};

// DIN MIDI Konfiguration
static pio_midi_uart_t *din_midi[4];

// Funktion zum Senden von MIDI-Daten an alle Ausgänge
void send_midi_to_all(uint8_t *data, uint32_t length) {
    // Sende an USB Guest
    tud_midi_write(0, data, length);

    // Sende an DIN MIDI Out
    for (int i = 0; i < 4; i++) {
        pio_midi_uart_write(din_midi[i], data, length);
    }

    // Sende an USB Host
    for (int i = 0; i < host_config.num_ports; i++) {
        usb_midi_host_write(i, data, length);
    }
}

// Core 1: Verarbeitet USB Host und DIN MIDI Eingänge
void core1_entry() {
    while (1) {
        // USB Host Task
        usbh_task();

        // Verarbeite USB Host MIDI Eingänge
        for (int port = 0; port < host_config.num_ports; port++) {
            uint8_t rx_buf[MIDI_BUFFER_SIZE];
            uint32_t rx_len = usb_midi_host_read(port, rx_buf, MIDI_BUFFER_SIZE);
            if (rx_len > 0) {
                send_midi_to_all(rx_buf, rx_len);
            }
        }

        // Verarbeite DIN MIDI Eingänge
        for (int i = 0; i < 4; i++) {
            uint8_t rx_buf[MIDI_BUFFER_SIZE];
            uint32_t rx_len = pio_midi_uart_read(din_midi[i], rx_buf, MIDI_BUFFER_SIZE);
            if (rx_len > 0) {
                send_midi_to_all(rx_buf, rx_len);
            }
        }
    }
}

int main() {
    stdio_init_all();

    // Initialisiere USB Guest (Device)
    tusb_init();

    // Initialisiere USB Host
    usbh_init(&host_config);

    // Initialisiere DIN MIDI
    din_midi[0] = pio_midi_uart_init(0, DIN_MIDI_RX_1, 31250);
    din_midi[1] = pio_midi_uart_init(1, DIN_MIDI_RX_2, 31250);
    din_midi[2] = pio_midi_uart_init(2, DIN_MIDI_RX_3, 31250);
    din_midi[3] = pio_midi_uart_init(3, DIN_MIDI_RX_4, 31250);

    // Starte Core 1 für USB Host und DIN MIDI Verarbeitung
    multicore_launch_core1(core1_entry);

    // Core 0: Verarbeitet USB Guest Eingänge
    while (1) {
        tud_task();
        uint8_t rx_buf[MIDI_BUFFER_SIZE];
        uint32_t rx_len = tud_midi_read(0, rx_buf, MIDI_BUFFER_SIZE);
        if (rx_len > 0) {
            send_midi_to_all(rx_buf, rx_len);
        }
    }

    return 0;
}
