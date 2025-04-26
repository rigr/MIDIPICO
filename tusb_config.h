#ifndef TUSB_CONFIG_H
#define TUSB_CONFIG_H

#define CFG_TUSB_RHPORT0_MODE (OPT_MODE_HOST | OPT_MODE_DEVICE)
#define CFG_TUH_ENABLED       1
#define CFG_TUD_ENABLED       1
#define CFG_TUH_MIDI          2
#define CFG_TUD_MIDI          1
#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN   __attribute__((aligned(4)))

#endif
