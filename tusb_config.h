#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#define CFG_TUSB_MCU        OPT_MCU_RP2040
#define CFG_TUSB_OS         OPT_OS_PICO

// Device + Host Stack
#define CFG_TUD_ENABLED     1
#define CFG_TUH_ENABLED     1

// Device-Klassen
#define CFG_TUD_MIDI        1    // MIDI Device (tud_midi_*)
// z.B. CDC, HID usw. hier deaktivieren, falls nicht gebraucht
// #define CFG_TUD_CDC      0
// #define CFG_TUD_HID      0

// Host-Klassen
#define CFG_TUH_MIDI        1    // MIDI Host (tuh_midi_*)
// #define CFG_TUH_CDC      0
// #define CFG_TUH_HID      0

#define CFG_TUSB_DEBUG      0

// ggf. Puffergrößen anpassen
// #define CFG_TUH_MIDI_RX_BUFSIZE  256
// #define CFG_TUH_MIDI_TX_BUFSIZE  256

#endif /* _TUSB_CONFIG_H_ */
