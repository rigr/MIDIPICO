#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

//------------------------------------------------------------------------------
// Common OPT settings
//------------------------------------------------------------------------------
#define CFG_TUSB_MCU        OPT_MCU_RP2040
#define CFG_TUSB_OS         OPT_OS_PICO

//------------------------------------------------------------------------------
// Host / Device Enable
//------------------------------------------------------------------------------
#define CFG_TUD_ENABLED     1     // TinyUSB device stack
#define CFG_TUH_ENABLED     1     // TinyUSB host stack

//------------------------------------------------------------------------------
// Device Class activate
//------------------------------------------------------------------------------
#define CFG_TUD_MIDI        1     // MIDI Device (USB + UART)
// andere Device-Klassen ggf. deaktivieren:
// #define CFG_TUD_CDC      0
// #define CFG_TUD_HID      0

//------------------------------------------------------------------------------
// Host Class activate
//------------------------------------------------------------------------------
#define CFG_TUH_MIDI        1     // MIDI Host
// andere Host-Klassen ggf. deaktivieren:
// #define CFG_TUH_CDC      0
// #define CFG_TUH_HID      0

//------------------------------------------------------------------------------
// Debug
//------------------------------------------------------------------------------
#define CFG_TUSB_DEBUG      0

//------------------------------------------------------------------------------
// FIFO sizes, etc.
//------------------------------------------------------------------------------
// ggf. anpassen, wenn Du große SysEx-Pakete über Host schicken willst:
// #define CFG_TUH_MIDI_RX_BUFSIZE  256
// #define CFG_TUH_MIDI_TX_BUFSIZE  256

#endif /* _TUSB_CONFIG_H_ */
