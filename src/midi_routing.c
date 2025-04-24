#include "midi_routing.h"
#include "tusb.h"
#include "pio_midi_uart_lib.h"

#define MAX_DEVICES 8
#define MAX_DIN_PORTS 4

typedef struct {
    uint8_t dev_addr;
    uint8_t in_ep;
    uint8_t out_ep;
    bool active;
} midi_device_t;

static midi_device_t devices[MAX_DEVICES];
static uint8_t device_count = 0;
static midi_uart_t *midi_din_ports[MAX_DIN_PORTS];

void midi_routing_init(void) {
    device_count = 0;
    for (int i = 0; i < MAX_DEVICES; i++) {
        devices[i].active = false;
    }
    // Externe Referenz zu DIN-Ports aus main.c
    extern midi_uart_t midi_din[4];
    for (int i = 0; i < MAX_DIN_PORTS; i++) {
        midi_din_ports[i] = &midi_din[i];
    }
}

void midi_routing_add_device(uint8_t dev_addr, uint8_t in_ep, uint8_t out_ep) {
    if (device_count < MAX_DEVICES) {
        devices[device_count].dev_addr = dev_addr;
        devices[device_count].in_ep = in_ep;
        devices[device_count].out_ep = out_ep;
        devices[device_count].active = true;
        device_count++;
    }
}

void midi_routing_forward(uint8_t *buffer, uint32_t len, uint8_t source_dev) {
    // Sende an alle USB-Ausgänge außer der Quelle
    for (int i = 0; i < device_count; i++) {
        if (devices[i].active && devices[i].dev_addr != source_dev) {
            tud_midi_stream_write(devices[i].out_ep, buffer, len);
        }
    }
    // Sende an alle DIN-MIDI-Ausgänge
    for (int i = 0; i < MAX_DIN_PORTS; i++) {
        midi_uart_write(midi_din_ports[i], buffer, len);
    }
    // Sende an USB-Guest-Ausgang
    tud_midi_stream_write(0, buffer, len);
}

void midi_routing_process(void) {
    uint8_t buffer[128];
    uint32_t len;

    // Verarbeite DIN-MIDI-Eingänge
    for (int i = 0; i < MAX_DIN_PORTS; i++) {
        len = midi_uart_read(midi_din_ports[i], buffer, sizeof(buffer));
        if (len > 0) {
            // Sende an alle USB-Ausgänge
            for (int j = 0; j < device_count; j++) {
                if (devices[j].active) {
                    tud_midi_stream_write(devices[j].out_ep, buffer, len);
                }
            }
            // Sende an andere DIN-MIDI-Ausgänge
            for (int j = 0; j < MAX_DIN_PORTS; j++) {
                if (j != i) {
                    midi_uart_write(midi_din_ports[j], buffer, len);
                }
            }
            // Sende an USB-Guest-Ausgang
            tud_midi_stream_write(0, buffer, len);
        }
    }
}
