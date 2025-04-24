#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "tusb.h"
#include "pio_usb.h"
#include "pio_midi_uart_lib.h"
#include "midi_routing.h"

// Pin-Belegung
#define USB_HOST1_DP 2
#define USB_HOST2_DP 4
#define USB_HOST3_DP 6
#define USB_HOST4_DP 8
#define MIDI_DIN1_RX 10
#define MIDI_DIN1_TX 11
#define MIDI_DIN2_RX 12
#define MIDI_DIN2_TX 13
#define MIDI_DIN3_RX 14
#define MIDI_DIN3_TX 15
#define MIDI_DIN4_RX 16
#define MIDI_DIN4_TX 17

// USB-Host-Konfiguration
static pio_usb_configuration_t pio_usb_config = {
    .pin_dp = USB_HOST1_DP,
    .pio_idx = 0,
    .sm_idx = 0,
    .endpoint_pool_size = 8
};

// MIDI-UART-Konfiguration
static midi_uart_t midi_din[4];

// USB-Host-Instanz
static usb_host_t usb_host[4];

// Funktion zum Initialisieren der USB-Host-Schnittstellen
void init_usb_host() {
    for (int i = 0; i < 4; i++) {
        pio_usb_config.pin_dp = USB_HOST1_DP + i * 2;
        usb_host[i] = pio_usb_host_init(&pio_usb_config);
        pio_usb_config.sm_idx += 4;
    }
}

// Funktion zum Initialisieren der DIN-MIDI-Schnittstellen
void init_din_midi() {
    uint rx_pins[4] = {MIDI_DIN1_RX, MIDI_DIN2_RX, MIDI_DIN3_RX, MIDI_DIN4_RX};
    uint tx_pins[4] = {MIDI_DIN1_TX, MIDI_DIN2_TX, MIDI_DIN3_TX, MIDI_DIN4_TX};
    for (int i = 0; i < 4; i++) {
        midi_din[i] = midi_uart_init(PIO0, rx_pins[i], tx_pins[i], MIDI_UART_LIB_BAUD_RATE);
    }
}

// Hauptfunktion für Core 1 (USB-Host-Verarbeitung)
void core1_main() {
    sleep_ms(10);
    pio_usb_host_run();
}

// Hauptprogramm
int main() {
    stdio_init_all();
    multicore_launch_core1(core1_main);

    // Initialisiere TinyUSB
    tusb_init();

    // Initialisiere USB-Host-Schnittstellen
    init_usb_host();

    // Initialisiere DIN-MIDI-Schnittstellen
    init_din_midi();

    // Initialisiere MIDI-Routing
    midi_routing_init();

    while (1) {
        // Verarbeite USB-Device-Aufgaben
        tud_task();

        // Verarbeite USB-Host-Aufgaben
        for (int i = 0; i < 4; i++) {
            pio_usb_host_task(&usb_host[i]);
        }

        // Verarbeite MIDI-Daten
        midi_routing_process();
    }

    return 0;
}

// TinyUSB Callbacks
void tuh_midi_mount_cb(uint8_t dev_addr, uint8_t in_ep, uint8_t out_ep, uint8_t num_cables_rx, uint16_t num_cables_tx) {
    midi_routing_add_device(dev_addr, in_ep, out_ep);
}

void tuh_midi_rx_cb(uint8_t dev_addr, uint32_t num_packets) {
    uint8_t buffer[128];
    uint32_t len = tud_midi_stream_read(buffer, sizeof(buffer));
    if (len > 0) {
        midi_routing_forward(buffer, len, dev_addr);
    }
}

void tud_midi_tx_cb(uint8_t cable_num) {
    // Nicht benötigt, da wir nur weiterleiten
}
