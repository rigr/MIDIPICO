#ifndef TUSB_CONFIG_H
#define TUSB_CONFIG_H

// Enable both Host and Device modes
#define CFG_TUSB_RHPORT0_MODE (OPT_MODE_HOST | OPT_MODE_DEVICE)
#define CFG_TUSB_OS OPT_OS_PICO

// USB Device configuration
#define CFG_TUD_ENABLED 1
#define CFG_TUD_MIDI 1
#define CFG_TUD_MIDI_RX_BUFSIZE 128
#define CFG_TUD_MIDI_TX_BUFSIZE 128

// USB Host configuration
#define CFG_TUH_ENABLED 1
#define CFG_TUH_MAX_SPEED OPT_MODE_FULL_SPEED
#define CFG_TUH_MIDI 2 // Support only 2 MIDI devices
#define CFG_TUH_MIDI_RX_BUFSIZE 128
#define CFG_TUH_MIDI_TX_BUFSIZE 128

// Common configuration
#define CFG_TUSB_MCU OPT_MCU_RP2040
#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN __attribute__((aligned(4)))

#endif // TUSB_CONFIG_H
