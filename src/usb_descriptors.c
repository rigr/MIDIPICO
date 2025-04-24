#include "tusb.h"

// USB-Deskriptoren
#define USBD_VID 0x2E8A // Raspberry Pi
#define USBD_PID 0x000A // Generischer Pico PID
#define USBD_MANUFACTURER "Raspberry Pi"
#define USBD_PRODUCT "MIDIPICO"

// Geräte-Deskriptor
const tusb_desc_device_t desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = USBD_VID,
    .idProduct = USBD_PID,
    .bcdDevice = 0x0100,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x00,
    .bNumConfigurations = 0x01
};

// Konfigurations-Deskriptor
uint8_t const desc_configuration[] = {
    // Config length, type, total length, num interfaces, config ID, string index, attributes, power
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, TUD_CONFIG_DESC_LEN + TUD_MIDI_DESC_LEN, 0x00, 100),

    // MIDI-Interface
    TUD_MIDI_DESCRIPTOR(0, 0, 0x01, 0x02, 64, 1)
};

// String-Deskriptoren
const char *string_desc_arr[] = {
    (const char[]){0x09, 0x04}, // 0: Englisch (US)
    USBD_MANUFACTURER,          // 1: Hersteller
    USBD_PRODUCT,               // 2: Produkt
    NULL                        // 3: Seriennummer (keine)
};

// Callback für String-Deskriptoren
const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    static uint16_t desc_str[32];
    uint8_t chr_count;

    if (index == 0) {
        memcpy(desc_str, string_desc_arr[0], 4);
        return desc_str;
    }

    const char *str = string_desc_arr[index];
    chr_count = strlen(str);
    if (chr_count > 31) chr_count = 31;

    // Konvertiere ASCII zu UTF-16
    for (uint8_t i = 0; i < chr_count; i++) {
        desc_str[i + 1] = str[i];
    }
    desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

    return desc_str;
}

// Callback für Geräte-Deskriptor
uint8_t const *tud_descriptor_device_cb(void) {
    return (uint8_t const *)&desc_device;
}

// Callback für Konfigurations-Deskriptor
uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
    return desc_configuration;
}
