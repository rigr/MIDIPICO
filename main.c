#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "tusb.h"               // bindet tusb_config.h und alle TinyUSB-Headers ein
#include "usb_midi_host.h"      // Host-Treiber (tuh_*)
#include "pio_midi_uart_lib.h"
#include "pio_usb.h"            // pio_usb_host_init() und usb_device_t kommen hierher

// Pin-Zuweisungen
#define DIN_MIDI_RX_1    6
#define DIN_MIDI_RX_2    7
#define DIN_MIDI_RX_3    8
#define DIN_MIDI_RX_4    9
#define USB_HOST_DP_1   10
#define USB_HOST_DP_2   12

// MIDI-Puffer
#define MIDI_BUFFER_SIZE 128

// USB-Host-Konfigurationen
static pio_usb_configuration_t host_config1 = {
    .pin_dp      = USB_HOST_DP_1,
    .pio_tx_num  = 0,
    .sm_tx       = 0,
    .tx_ch       = 0,
    .pio_rx_num  = 0,
    .sm_rx       = 1,
    .sm_eop      = 2,
    .alarm_pool  = NULL,
    .debug_pin_rx= -1,
    .debug_pin_eop=-1,
    .skip_alarm_pool=false,
    .pinout      = PIO_USB_PINOUT_DPDM,
};
static pio_usb_configuration_t host_config2 = {
    .pin_dp      = USB_HOST_DP_2,
    .pio_tx_num  = 1,
    .sm_tx       = 0,
    .tx_ch       = 1,
    .pio_rx_num  = 1,
    .sm_rx       = 1,
    .sm_eop      = 2,
    .alarm_pool  = NULL,
    .debug_pin_rx= -1,
    .debug_pin_eop=-1,
    .skip_alarm_pool=false,
    .pinout      = PIO_USB_PINOUT_DPDM,
};

// DIN-MIDI per PIO
static pio_midi_uart_t *din_midi[4];

// Aktive Host-Geräteadressen
static uint8_t host1_dev_addr = 0;
static uint8_t host2_dev_addr = 0;

// Mount-Callback (5 Parameter, wie im Header deklariert)
void tuh_midi_mount_cb(uint8_t dev_addr,
                       uint8_t in_ep,
                       uint8_t out_ep,
                       uint8_t num_cables_rx,
                       uint16_t num_cables_tx)
{
    printf("MIDI device mounted, dev_addr=%u\n", dev_addr);
    if (!host1_dev_addr) {
        host1_dev_addr = dev_addr;
        printf("Assigned to Host Port 1\n");
    } else if (!host2_dev_addr) {
        host2_dev_addr = dev_addr;
        printf("Assigned to Host Port 2\n");
    } else {
        printf("No free host port available\n");
    }
}

// Unmount-Callback jetzt mit 2 Parametern (dev_addr, instance)
void tuh_midi_umount_cb(uint8_t dev_addr, uint8_t instance)
{
    printf("MIDI device unmounted, dev_addr=%u, instance=%u\n", dev_addr, instance);
    if (host1_dev_addr == dev_addr) host1_dev_addr = 0;
    else if (host2_dev_addr == dev_addr) host2_dev_addr = 0;
}

// Verteilt einen empfangenen MIDI-Block an alle Ausgänge
void send_midi_to_all(uint8_t *data, uint32_t length)
{
    // USB-Device (Guest)
    tud_midi_write(data, length);

    // DIN-MIDI
    for (int i = 0; i < 4; i++) {
        pio_midi_uart_write(din_midi[i], data, length);
    }

    // USB-Host-Ports
    if (host1_dev_addr) tuh_midi_write(host1_dev_addr, data, length);
    if (host2_dev_addr) tuh_midi_write(host2_dev_addr, data, length);
}

// Core 1: Polling für USB-Host + DIN-MIDI
void core1_entry(void)
{
    while (1) {
        tuh_task();

        uint8_t rx_buf[MIDI_BUFFER_SIZE];
        // Host 1
        if (host1_dev_addr) {
            uint32_t rx_len = tuh_midi_read(host1_dev_addr, rx_buf, MIDI_BUFFER_SIZE);
            if (rx_len) send_midi_to_all(rx_buf, rx_len);
        }
        // Host 2
        if (host2_dev_addr) {
            uint32_t rx_len = tuh_midi_read(host2_dev_addr, rx_buf, MIDI_BUFFER_SIZE);
            if (rx_len) send_midi_to_all(rx_buf, rx_len);
        }
        // DIN-MIDI
        for (int i = 0; i < 4; i++) {
            uint32_t rx_len = pio_midi_uart_read(din_midi[i], rx_buf, MIDI_BUFFER_SIZE);
            if (rx_len) send_midi_to_all(rx_buf, rx_len);
        }
    }
}

int main(void)
{
    stdio_init_all();
    printf("Initializing TinyUSB and USB Host...\n");

    // USB-Host starten
    if (!pio_usb_host_init(&host_config1)) {
        printf("Failed to init USB Host Port 1\n"); while (1);
    }
    if (!pio_usb_host_init(&host_config2)) {
        printf("Failed to init USB Host Port 2\n"); while (1);
    }

    // TinyUSB (Host & Device)
    tusb_init();

    // DIN-MIDI
    din_midi[0] = pio_midi_uart_init(0, DIN_MIDI_RX_1, 31250);
    din_midi[1] = pio_midi_uart_init(1, DIN_MIDI_RX_2, 31250);
    din_midi[2] = pio_midi_uart_init(2, DIN_MIDI_RX_3, 31250);
    din_midi[3] = pio_midi_uart_init(3, DIN_MIDI_RX_4, 31250);

    // Core 1 starten
    multicore_launch_core1(core1_entry);

    // Core 0: Polling für USB-Device (Guest)
    while (1) {
        tud_task();
        uint8_t rx_buf[MIDI_BUFFER_SIZE];
        uint32_t rx_len = tud_midi_read(rx_buf, MIDI_BUFFER_SIZE);
        if (rx_len) send_midi_to_all(rx_buf, rx_len);
    }

    return 0;
}
