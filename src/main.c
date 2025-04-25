#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "tusb.h"
#include "usb_midi_host.h"
#include "pio_midi_uart_lib.h"
#include "pio_usb.h"

// USB-Guest-Deskriptoren für "MIDIPICO"
#define USB_VID 0x2E8A // Raspberry Pi VID
#define USB_PID 0x000A // Generisches PID
static const char *midi_device_name = "MIDIPICO";

static const tusb_desc_device_t device_descriptor = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = USB_VID,
    .idProduct = USB_PID,
    .bcdDevice = 0x0100,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01
};

// String-Deskriptoren
static char *string_desc_arr[] = {
    (char[]){0x09, 0x04}, // Sprache: US-Englisch
    "Raspberry Pi",        // Hersteller
    midi_device_name,      // Produktname
    "1"                    // Seriennummer
};

static uint8_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    static uint8_t desc_str[32];
    const char *str;
    if (index == 0) {
        memcpy(desc_str, string_desc_arr[0], 4);
        return desc_str;
    }
    str = string_desc_arr[index];
    uint8_t chr_count = strlen(str);
    desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);
    for (uint8_t i = 0; i < chr_count; i++) {
        desc_str[2 + i * 2] = str[i];
        desc_str[3 + i * 2] = 0;
    }
    return desc_str;
}

// MIDI-Datenpuffer
#define MIDI_BUFFER_SIZE 128
static uint8_t midi_buffer[4][MIDI_BUFFER_SIZE]; // Puffer für 4 Eingänge
static volatile uint8_t buffer_pos[4] = {0};
static volatile bool buffer_ready[4] = {false};

// USB-Host-Instanz
usb_midi_host_t usb_midi_hosts[4];

// DIN-MIDI-Instanz
pio_midi_uart_t din_midi[4];

// Callback für USB-MIDI-Daten
void usb_midi_rx_callback(uint8_t host_idx, uint8_t *data, uint16_t len) {
    if (host_idx < 4 && buffer_pos[host_idx] + len < MIDI_BUFFER_SIZE) {
        memcpy(&midi_buffer[host_idx][buffer_pos[host_idx]], data, len);
        buffer_pos[host_idx] += len;
        buffer_ready[host_idx] = true;
    }
}

// Callback für DIN-MIDI-Daten
void din_midi_rx_callback(uint8_t din_idx, uint8_t *data, uint16_t len) {
    if (din_idx < 4 && buffer_pos[din_idx] + len < MIDI_BUFFER_SIZE) {
        memcpy(&midi_buffer[din_idx][buffer_pos[din_idx]], data, len);
        buffer_pos[din_idx] += len;
        buffer_ready[din_idx] = true;
    }
}

// Funktion zum Weiterleiten der MIDI-Daten
void forward_midi_data() {
    for (uint8_t src = 0; src < 4; src++) {
        if (buffer_ready[src]) {
            // An alle USB-Hosts
            for (uint8_t dst = 0; dst < 4; dst++) {
                if (dst != src) {
                    usb_midi_host_write(&usb_midi_hosts[dst], midi_buffer[src], buffer_pos[src]);
                }
            }
            // An alle DIN-MIDI
            for (uint8_t dst = 0; dst < 4; dst++) {
                pio_midi_uart_write(&din_midi[dst], midi_buffer[src], buffer_pos[src]);
            }
            // An USB-Guest
            tud_midi_stream_write(0, midi_buffer[src], buffer_pos[src]);
            buffer_pos[src] = 0;
            buffer_ready[src] = false;
        }
    }
}

void core1_entry() {
    // USB-Host-Initialisierung mit PIO-USB
    pio_usb_configuration_t pio_cfg = PIO_USB_DEFAULT_CONFIG;
    pio_cfg.pin_dp = 2; // Beispiel-Pin, anpassen an deine Hardware
    pio_usb_host_init(&pio_cfg);

    while (true) {
        pio_usb_host_task();
        forward_midi_data();
    }
}

int main() {
    stdio_init_all();
    tusb_init();

    // DIN-MIDI-Initialisierung
    for (uint8_t i = 0; i < 4; i++) {
        pio_midi_uart_init(&din_midi[i], pio0, 6 + i, 31250, din_midi_rx_callback, i); // Pins 6-9
    }

    // USB-MIDI-Host-Initialisierung
    for (uint8_t i = 0; i < 4; i++) {
        usb_midi_host_init(&usb_midi_hosts[i], i, usb_midi_rx_callback);
    }

    multicore_launch_core1(core1_entry);

    while (true) {
        tud_task();
        forward_midi_data();
    }
}
