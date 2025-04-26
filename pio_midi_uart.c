#include "pio_midi_uart.h"
#include "midi_uart.pio.h"

// PIO-Programm laden
static inline void midi_uart_program_init(PIO pio, uint sm, uint offset, uint rx_pin, uint tx_pin) {
    pio_sm_config c = midi_uart_program_get_default_config(offset);
    sm_config_set_in_pins(&c, rx_pin);
    sm_config_set_sideset_pins(&c, tx_pin);
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
}

pio_midi_uart_t pio_midi_uart_create(uint rx_pin, uint tx_pin) {
    pio_midi_uart_t uart;
    uart.pio = pio0;
    uart.sm = pio_claim_unused_sm(uart.pio, true);
    uart.offset = pio_add_program(uart.pio, &midi_uart_program);
    midi_uart_program_init(uart.pio, uart.sm, uart.offset, rx_pin, tx_pin);
    return uart;
}
