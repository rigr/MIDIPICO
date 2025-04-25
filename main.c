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

// USB Host Konfigurationen (zwei Ports)
static pio_usb_configuration_t host_config1 = {
    .pin_dp = USB_HOST_DP_1,
    .pio_tx_num = 0,
    .sm_tx = 0,
    .tx_ch = 0,
    .pio_rx_num = 0,
    .sm_rx = 1,
    .sm_eop = 2,
    .alarm_pool = NULL,
    .debug_pin_rx = -1,
    .debug_pin_eop = -1,
    .skip_alarm_pool = false,
    .pinout = PIO_USB_PINOUT_DPDM,
};

static pio_usb_configuration_t host_config2 = {
    .pin_dp = USB_HOST_DP_2,
    .pio_tx_num = 1,
    .sm_tx = 0,
    .tx_ch = 1,
    .pio_rx_num = 1,
    .sm_rx = 1,
    .sm_eop = 2,
    .alarm_pool = NULL,
    .debug_pin_rx = -1,
    .debug_pin_eop = -1,
    .skip_alarm_pool = false,
    .pinout = PIO_USB_PINOUT_DPDM,
};

// DIN MIDI Konfiguration
static pio_midi_uart_t *din_midi[4];

// Geräte-Tracking für USB-Host-Ports
static uint8_t host1_dev_addr = 0;
static uint8_t host2_dev_addr = 0;

// Callback für Geräte-Mount
void tuh_midi_mount_cb(uint8_t dev_addr, uint8_t in_ep, uint8_t out_ep, uint8_t num_cables_rx, uint16_t num_cables_tx) {
    printf("MIDI device mounted, dev_addr=%d\n", dev_addr);
    if (host1_dev_addr == 0) {
        host1_dev_addr = dev_addr;
        printf("Assigned to Host Port 1\n");
    } else if (host2_dev_addr == 0) {
        host2_dev_addr = dev_addr;
        printf("Assigned to Host Port 2\n");
    } else {
        printf("No free host port available\n");
    }
}

// Callback für Geräte-Unmount
void tuh_midi_umount_cb(uint8_t dev_addr) {
    printf("MIDI device unmounted, dev_addr=%d\n", dev_addr);
    if (host1_dev_addr == dev_addr) {
        host1_dev_addr = 0;
    } else if (host2_dev_addr == dev_addr) {
        host2_dev_addr = 0;
    }
}

// Funktion zum Senden von MIDI-Daten an alle Ausgänge
void send_midi_to_all(uint8_t *data, uint32_t length) {
    // Sende an USB Guest
    tud_midi_write(data, length);

    // Sende an DIN MIDI Out
    for (int i = 0; i < 4; i++) {
        pio_midi_uart_write(din_midi[i], data, length);
    }

    // Sende an USB Host Ports
    if (host1_dev_addr) {
        tuh_midi_write(host1_dev_addr, data, length);
    }
    if (host2_dev_addr) {
        tuh_midi_write(host2_dev_addr, data, length);
    }
}

// Core 1: Verarbeitet USB Host und DIN MIDI Eingänge
void core1_entry() {
    while (1) {
        // USB Host Task
        tuh_task();

        // Verarbeite USB Host MIDI Eingänge (zwei Ports)
        uint8_t rx_buf[MIDI_BUFFER_SIZE];
        if (host1_dev_addr) {
            uint32_t rx_len = tuh_midi_read(host1_dev_addr, rx_buf, MIDI_BUFFER_SIZE);
            if (rx_len > 0) {
                printf("MIDI data received from USB Host Port 1: %d bytes\n", rx_len);
                send_midi_to_all(rx_buf, rx_len);
            }
        }
        if (host2_dev_addr) {
            uint32_t rx_len = tuh_midi_read(host2_dev_addr, rx_buf, MIDI_BUFFER_SIZE);
            if (rx_len > 0) {
                printf("MIDI data received from USB Host Port 2: %d bytes\n", rx_len);
                send_midi_to_all(rx_buf, rx_len);
            }
        }

        // Verarbeite DIN MIDI Eingänge
        for (int i = 0; i < 4; i++) {
            uint32_t rx_len = pio_midi_uart_read(din_midi[i], rx_buf, MIDI_BUFFER_SIZE);
            if (rx_len > 0) {
                printf("MIDI data received from DIN MIDI %d: %d bytes\n", i, rx_len);
                send_midi_to_all(rx_buf, rx_len);
            }
        }
    }
}

int main() {
    stdio_init_all();

    // Debugging: Bestätige Initialisierung
    printf("Initializing TinyUSB and USB Host...\n");

    // Initialisiere PIO-USB für zwei Ports
    usb_device_t *host1 = pio_usb_host_init(&host_config1);
    if (!host1) {
        printf("Failed to initialize USB Host Port 1\n");
        while (1);
    }
    usb_device_t *host2 = pio_usb_host_init(&host_config2);
    if (!host2) {
        printf("Failed to initialize USB Host Port 2\n");
        while (1);
    }

    // Initialisiere TinyUSB (Host und Device)
    tusb_init();

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
        uint32_t rx_len = tud_midi_read(rx_buf, MIDI_BUFFER_SIZE);
        if (rx_len > 0) {
            printf("MIDI data received from USB Guest: %d bytes\n", rx_len);
            send_midi_to_all(rx_buf, rx_len);
        }
    }

    return 0;
}
