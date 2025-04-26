#ifndef PIO_MIDI_UART_H
#define PIO_MIDI_UART_H

#include "pico/stdlib.h"
#include "hardware/pio.h"

// Typ für MIDI-UART-Instanz
typedef struct {
    PIO pio;
    uint sm;
    uint offset;
} pio_midi_uart_t;

// Funktion zum Erstellen einer MIDI-UART-Instanz
pio_midi_uart_t pio_midi_uart_create(uint rx_pin, uint tx_pin);

#endif
