#ifndef TUSB_CONFIG_H
#define TUSB_CONFIG_H

// USB Host-Modus aktivieren
#define CFG_TUSB_RHPORT0_MODE OPT_MODE_HOST
#define CFG_TUH_ENABLED       1

// Zwei USB-MIDI-Geräte
#define CFG_TUH_MIDI          2

// Standard-Puffergrößen
#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN   __attribute__((aligned(4)))

#endif
