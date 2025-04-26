#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "tusb.h"
#include "pio_midi_uart.h"

// Konfigurationskonstanten
#define MIDI_BUFFER_SIZE 128
#define DIN_MIDI_RX_1 0 // GPIO-Pin für DIN MIDI RX 1
#define DIN_MIDI_RX_2 1 // GPIO-Pin für DIN MIDI RX 2

// Globale Variablen
static uint8_t host1_dev_addr = 0; // Geräteadresse für Host 1

// Callback für MIDI-Daten (Platzhalter)
void tuh_midi_rx_cb(uint8_t dev_addr, uint32_t num_packets) {
    // Wird aufgerufen, wenn MIDI-Daten empfangen werden
    // Hier kannst du die Geräteadresse speichern
    if (dev_addr && !host1_dev_addr) {
        host1_dev_addr = dev_addr;
    }
}

// Core 1: Verarbeitet MIDI-Daten von Host 1
void core1_entry(void) {
    uint8_t rx_buf[MIDI_BUFFER_SIZE];
    while (true) {
        if (host1_dev_addr) {
            uint32_t rx_len = tuh_midi_receive(host1_dev_addr, rx_buf, MIDI_BUFFER_SIZE, true);
            if (rx_len > 0) {
                // MIDI-Daten verarbeiten (z. B. weiterleiten)
                tuh_midi_packet_write(host1_dev_addr, rx_buf, rx_len);
            }
        }
        sleep_ms(1); // CPU entlasten
    }
}

int main() {
    // Standard-Initialisierung
    stdio_init_all();
    
    // TinyUSB initialisieren
    tusb_init();

    // DIN MIDI UARTs initialisieren
    pio_midi_uart_t din_midi[2];
    din_midi[0] = pio_midi_uart_create(DIN_MIDI_RX_1, DIN_MIDI_RX_1);
    din_midi[1] = pio_midi_uart_create(DIN_MIDI_RX_2, DIN_MIDI_RX_2);

    // Core 1 starten
    multicore_launch_core1(core1_entry);

    // Puffer für MIDI-Daten
    uint8_t rx_buf[MIDI_BUFFER_SIZE];

    while (true) {
        // TinyUSB Host-Task ausführen
        tuh_task();

        // USB-MIDI-Daten von zwei Geräten lesen
        for (int dev = 0; dev < 2; dev++) {
            uint32_t rx_len = tuh_midi_receive(dev, rx_buf, MIDI_BUFFER_SIZE, true);
            if (rx_len > 0) {
                // MIDI-Daten verarbeiten (z. B. weiterleiten)
                tuh_midi_packet_write(dev, rx_buf, rx_len);
            }
        }

        // DIN-MIDI-Daten verarbeiten (Platzhalter)
        // Hier deine Logik für din_midi[0] und din_midi[1] einfügen
    }

    return 0;
}
