#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "tusb.h"
#include "usb_midi_host.h"
#include "pio_midi_uart_lib.h"
#include "pio_usb.h"

// Pin-Zuweisungen
#define DIN_MIDI_RX_1 6  // GPIO 6 für DIN MIDI In 1
#define DIN_MIDI_RX_2 7  // GPIO 7 für DIN MIDI In 2
#define DIN_MIDI_RX_3 8  // GPIO 8 für DIN MIDI In 3
#define DIN_MIDI_RX_4 9  // GPIO 9 für DIN MIDI In 4

#define USB_HOST_DP_1 10 // GPIO 10 für USB Host 1 D+
#define USB_HOST_DP_2 12 // GPIO 12 für USB Host 2 D+

// MIDI-Datenpuffer
#define MIDI_BUFFER_SIZE 128
uint8_t midi_buffer[MIDI_BUFFER_SIZE];

// USB Host Konfigurationen (nur zwei Ports möglich)
static pio_usb_configuration_t host_config1 = {
    .pin_dp = USB_HOST_DP_1,
    .pio_tx_num = 0,
    .sm_tx = 0,
    .tx_ch = 0,
    .pio_rx_num = 1,
    .sm_rx = 0,
    .sm_eop = 1,
    .alarm_pool = NULL,
    .debug_pin_rx = -1,
    .debug_pin_eop = -1,
    .skip_alarm_pool = false,
    .pinout = PIO_USB_PINOUT_DPDM,
};

static pio_usb_configuration_t host_config2 = {
    .pin_dp = USB_HOST_DP_2,
    .pio_tx_num = 0,
    .sm_tx = 1,
    .tx_ch = 1,
    .pio_rx_num = 1,
    .sm_rx = 2,
    .sm_eop = 3,
    .alarm_pool = NULL,
    .debug_pin_rx = -1,
    .debug_pin_eop = -1,
    .skip_alarm_pool = false,
    .pinout = PIO_USB_PINOUT_DPDM,
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

    // Sende an USB Host (nur zwei Ports)
    for (int port = 0; port < 2; port++) {
        usb_midi_host_write(port, data, length);
    }
}

// Core 1: Verarbeitet USB Host und DIN MIDI Eingänge
void core1_entry() {
    while (1) {
        // USB Host Task
        tuh_task();

        // Verarbeite USB Host MIDI Eingänge (nur zwei Ports)
        for (int port = 0; port < 2; port++) {
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

    // Initialisiere USB Host (zwei Ports)
    tuh_configure(0, TUH_CFGID_RPI_PIO_USB_CONFIGURATION, &host_config1);
    tuh_configure(1, TUH_CFGID_RPI_PIO_USB_CONFIGURATION, &host_config2);
    tuh_init(0);
    tuh_init(1);

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
