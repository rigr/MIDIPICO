#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "tusb.h"
#include "pio_midi_uart_lib.h"

// Konfigurationskonstanten
#define MIDI_BUFFER_SIZE 128
#define DIN_MIDI_RX_1 0 // GPIO-Pin für DIN MIDI RX 1
#define DIN_MIDI_TX_1 4 // GPIO-Pin für DIN MIDI TX 1
#define DIN_MIDI_RX_2 1 // GPIO-Pin für DIN MIDI RX 2
#define DIN_MIDI_TX_2 5 // GPIO-Pin für DIN MIDI TX 2
#define DIN_MIDI_RX_3 2 // GPIO-Pin für DIN MIDI RX 3
#define DIN_MIDI_TX_3 6 // GPIO-Pin für DIN MIDI TX 3
#define DIN_MIDI_RX_4 3 // GPIO-Pin für DIN MIDI RX 4
#define DIN_MIDI_TX_4 7 // GPIO-Pin für DIN MIDI TX 4

// Globale Variablen
static uint8_t host1_dev_addr = 0; // Geräteadresse für Host 1
static uint8_t host2_dev_addr = 0; // Geräteadresse für Host 2
static pio_midi_uart_t din_midi[4]; // DIN-MIDI-Anschlüsse

// Callback für MIDI-Daten (Host-Modus)
void tuh_midi_rx_cb(uint8_t dev_addr, uint32_t num_packets) {
    if (dev_addr && !host1_dev_addr) {
        host1_dev_addr = dev_addr;
    } else if (dev_addr && !host2_dev_addr && dev_addr != host1_dev_addr) {
        host2_dev_addr = dev_addr;
    }
}

// Callback für MIDI-Daten (Device-Modus)
void tud_midi_rx_cb(uint8_t port, uint8_t *midi_data, uint32_t length) {
    if (host1_dev_addr) {
        tuh_midi_packet_write(host1_dev_addr, midi_data, length);
    }
    if (host2_dev_addr) {
        tuh_midi_packet_write(host2_dev_addr, midi_data, length);
    }
    for (int i = 0; i < 4; i++) {
        pio_midi_uart_write_tx_buffer(&din_midi[i], midi_data, length);
        pio_midi_uart_drain_tx_buffer(&din_midi[i]);
    }
}

// Core 1: Verarbeitet MIDI-Daten von Host 2 und DIN-MIDI
void core1_entry(void) {
    uint8_t rx_buf[MIDI_BUFFER_SIZE];
    while (true) {
        // Host 2
        if (host2_dev_addr) {
            uint32_t rx_len = tuh_midi_receive(host2_dev_addr, rx_buf, MIDI_BUFFER_SIZE, true);
            if (rx_len > 0) {
                tud_midi_packet_write(0, rx_buf, rx_len);
                for (int i = 0; i < 4; i++) {
                    pio_midi_uart_write_tx_buffer(&din_midi[i], rx_buf, rx_len);
                    pio_midi_uart_drain_tx_buffer(&din_midi[i]);
                }
            }
        }
        // DIN-MIDI
        for (int i = 0; i < 4; i++) {
            uint32_t rx_len = pio_midi_uart_poll_rx_buffer(&din_midi[i], rx_buf, MIDI_BUFFER_SIZE);
            if (rx_len > 0) {
                if (host1_dev_addr) tuh_midi_packet_write(host1_dev_addr, rx_buf, rx_len);
                if (host2_dev_addr) tuh_midi_packet_write(host2_dev_addr, rx_buf, rx_len);
                tud_midi_packet_write(0, rx_buf, rx_len);
            }
        }
        sleep_ms(1);
    }
}

int main() {
    stdio_init_all();
    tusb_init();

    // DIN MIDI UARTs initialisieren (4 Anschlüsse)
    din_midi[0] = pio_midi_uart_create(DIN_MIDI_RX_1, DIN_MIDI_TX_1);
    din_midi[1] = pio_midi_uart_create(DIN_MIDI_RX_2, DIN_MIDI_TX_2);
    din_midi[2] = pio_midi_uart_create(DIN_MIDI_RX_3, DIN_MIDI_TX_3);
    din_midi[3] = pio_midi_uart_create(DIN_MIDI_RX_4, DIN_MIDI_TX_4);

    // Core 1 starten
    multicore_launch_core1(core1_entry);

    // Puffer für MIDI-Daten
    uint8_t rx_buf[MIDI_BUFFER_SIZE];

    while (true) {
        tuh_task();
        tud_task();

        // USB-MIDI-Daten von Host 1 lesen
        if (host1_dev_addr) {
            uint32_t rx_len = tuh_midi_receive(host1_dev_addr, rx_buf, MIDI_BUFFER_SIZE, true);
            if (rx_len > 0) {
                tud_midi_packet_write(0, rx_buf, rx_len);
                for (int i = 0; i < 4; i++) {
                    pio_midi_uart_write_tx_buffer(&din_midi[i], rx_buf, rx_len);
                    pio_midi_uart_drain_tx_buffer(&din_midi[i]);
                }
            }
        }
    }

    return 0;
}
