#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "tusb.h"
#include "class/midi/midi_host.h"
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
static pio_midi_uart_t* din_midi[4]; // DIN-MIDI-Anschlüsse (Zeiger-Array)

// Hilfsfunktion: Konvertiert MIDI-Daten in 4-Byte-Pakete
static void send_midi_packet(uint8_t dev_addr, uint8_t cable_num, uint8_t *data, uint32_t len) {
    for (uint32_t i = 0; i < len; i += 3) {
        if (i + 2 >= len) break; // Unvollständige Nachricht ignorieren
        uint8_t packet[4] = { (cable_num << 4) | 0x0B, data[i], data[i + 1], data[i + 2] }; // CIN=0xB (Note On/Off)
        if (dev_addr) {
            tuh_midi_packet_write(dev_addr, packet); // idx: uint8_t, packet: uint8_t const[4]
        } else {
            tud_midi_packet_write(packet); // packet: uint8_t const[4]
        }
    }
}

// Callback für MIDI-Daten (Host-Modus)
void tuh_midi_rx_cb(uint8_t dev_addr, uint32_t num_packets) {
    if (dev_addr && !host1_dev_addr) {
        host1_dev_addr = dev_addr;
    } else if (dev_addr && !host2_dev_addr && dev_addr != host1_dev_addr) {
        host2_dev_addr = dev_addr;
    }
}

// Core 1: Verarbeitet MIDI-Daten von Host 2 und DIN-MIDI
void core1_entry(void) {
    uint8_t rx_buf[MIDI_BUFFER_SIZE];
    uint8_t packet[4];
    while (true) {
        // Host 2
        if (host2_dev_addr) {
            uint32_t rx_len = tuh_midi_stream_read(host2_dev_addr, 0, rx_buf, MIDI_BUFFER_SIZE);
            // dev_addr: uint8_t, cable_num: uint8_t, buf: uint8_t*, len: uint32_t
            if (rx_len > 0) {
                send_midi_packet(0, 0, rx_buf, rx_len); // Zum Device
                for (int i = 0; i < 4; i++) {
                    pio_midi_uart_write_tx_buffer(din_midi[i], rx_buf, rx_len);
                    // midi_uart: pio_midi_uart_t*, buffer: uint8_t*, len: uint32_t
                    pio_midi_uart_drain_tx_buffer(din_midi[i]); // midi_uart: pio_midi_uart_t*
                }
            }
        }
        // DIN-MIDI
        for (int i = 0; i < 4; i++) {
            uint32_t rx_len = pio_midi_uart_poll_rx_buffer(din_midi[i], rx_buf, MIDI_BUFFER_SIZE);
            // midi_uart: pio_midi_uart_t*, buffer: uint8_t*, buffer_max_len: uint32_t
            if (rx_len > 0) {
                if (host1_dev_addr) send_midi_packet(host1_dev_addr, 0, rx_buf, rx_len);
                if (host2_dev_addr) send_midi_packet(host2_dev_addr, 0, rx_buf, rx_len);
                send_midi_packet(0, 0, rx_buf, rx_len);
            }
        }
        // USB-MIDI-Device (Empfang)
        while (tud_midi_packet_read(packet)) { // packet: uint8_t[4]
            if (host1_dev_addr) tuh_midi_packet_write(host1_dev_addr, packet);
            if (host2_dev_addr) tuh_midi_packet_write(host2_dev_addr, packet);
            for (int i = 0; i < 4; i++) {
                pio_midi_uart_write_tx_buffer(din_midi[i], &packet[1], 3);
                pio_midi_uart_drain_tx_buffer(din_midi[i]);
            }
        }
        sleep_ms(1);
    }
}

int main() {
    stdio_init_all();
    tusb_init();

    // DIN MIDI UARTs initialisieren (4 Anschlüsse)
    din_midi[0] = pio_midi_uart_create(DIN_MIDI_RX_1, DIN_MIDI_TX_1); // rx_gpio: uint, tx_gpio: uint
    din_midi[1] = pio_midi_uart_create(DIN_MIDI_RX_2, DIN_MIDI_TX_2);
    din_midi[2] = pio_midi_uart_create(DIN_MIDI_RX_3, DIN_MIDI_TX_3);
    din_midi[3] = pio_midi_uart_create(DIN_MIDI_RX_4, DIN_MIDI_TX_4);

    // Core 1 starten
    multicore_launch_core1(core1_entry);

    // Puffer für MIDI-Daten
    uint8_t rx_buf[MIDI_BUFFER_SIZE];
    uint8_t packet[4];

    while (true) {
        tuh_task();
        tud_task();

        // USB-MIDI-Daten von Host 1 lesen
        if (host1_dev_addr) {
            uint32_t rx_len = tuh_midi_stream_read(host1_dev_addr, 0, rx_buf, MIDI_BUFFER_SIZE);
            if (rx_len > 0) {
                send_midi_packet(0, 0, rx_buf, rx_len); // Zum Device
                for (int i = 0; i < 4; i++) {
                    pio_midi_uart_write_tx_buffer(din_midi[i], rx_buf, rx_len);
                    pio_midi_uart_drain_tx_buffer(din_midi[i]);
                }
            }
        }
        // USB-MIDI-Device (Empfang)
        while (tud_midi_packet_read(packet)) {
            if (host1_dev_addr) tuh_midi_packet_write(host1_dev_addr, packet);
            if (host2_dev_addr) tuh_midi_packet_write(host2_dev_addr, packet);
            for (int i = 0; i < 4; i++) {
                pio_midi_uart_write_tx_buffer(din_midi[i], &packet[1], 3);
                pio_midi_uart_drain_tx_buffer(din_midi[i]);
            }
        }
    }

    return 0;
}
