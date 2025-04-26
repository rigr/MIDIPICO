# MIDIPICO

MIDIPICO ist ein MIDI-Interface-Projekt für den Raspberry Pi Pico, das zwei USB-MIDI-Host-Schnittstellen, eine USB-device-Schnittstelle und vier DIN-MIDI-Schnittstellen bereitstellt. Es leitet MIDI-Daten von allen Eingängen an alle Ausgänge weiter. Der Build-Prozess erfolgt über GitHub Actions, und die Firmware wird als `.uf2`-Datei generiert.

## Pinbelegung

Die folgende Tabelle beschreibt die Pinbelegung für die Anschlüsse auf dem Raspberry Pi Pico:

| Funktion                    | GPIO-Pin | Beschreibung                                      |
|-----------------------------|----------|--------------------------------------------------|
| **DIN MIDI In 1**           | GPIO 6   | Eingabe für DIN-MIDI-Schnittstelle 1 (Optokoppler erforderlich) |
| **DIN MIDI In 2**           | GPIO 7   | Eingabe für DIN-MIDI-Schnittstelle 2 (Optokoppler erforderlich) |
| **DIN MIDI In 3**           | GPIO 8   | Eingabe für DIN-MIDI-Schnittstelle 3 (Optokoppler erforderlich) |
| **DIN MIDI In 4**           | GPIO 9   | Eingabe für DIN-MIDI-Schnittstelle 4 (Optokoppler erforderlich) |
| **USB Host 1 D+**           | GPIO 10  | D+ für USB-MIDI-Host-Schnittstelle 1             |
| **USB Host 1 D-**           | GPIO 11  | D- für USB-MIDI-Host-Schnittstelle 1             |
| **USB Host 2 D+**           | GPIO 12  | D+ für USB-MIDI-Host-Schnittstelle 2             |
| **USB Host 2 D-**           | GPIO 13  | D- für USB-MIDI-Host-Schnittstelle 2             |
| **USB Host 3 D+**           | GPIO 14  | D+ für USB-MIDI-Host-Schnittstelle 3             |
| **USB Host 3 D-**           | GPIO 15  | D- für USB-MIDI-Host-Schnittstelle 3             |
| **USB Host 4 D+**           | GPIO 16  | D+ für USB-MIDI-Host-Schnittstelle 4             |
| **USB Host 4 D-**           | GPIO 17  | D- für USB-MIDI-Host-Schnittstelle 4             |

### Hinweise zur Hardware
- **USB-Host-Schnittstellen**: Verbinden Sie die USB-Buchsen mit den angegebenen GPIO-Pins (D+/D-). Stellen Sie sicher, dass angeschlossene Geräte extern mit 5 V versorgt werden, da der Pico keine ausreichende Stromversorgung für USB-Geräte bietet.
- **DIN-MIDI-Schnittstellen**: Verwenden Sie Optokoppler für die MIDI-Eingänge (GPIO 6-9), um den Pico zu schützen. Für MIDI-Ausgänge sind entsprechende Schaltungen erforderlich (z. B. mit einem PNP-Transistor oder einem MIDI-Treiber-IC).
- **USB-Guest-Schnittstelle**: Der Pico erscheint als USB-MIDI-Gerät mit dem Namen „MIDIPICO“, wenn er über USB mit einem Host (z. B. einem Computer) verbunden ist.

## Installation der Firmware
1. Laden Sie die `midipico.uf2`-Datei aus den GitHub Actions-Artefakten herunter.
2. Halten Sie den **BOOTSEL**-Knopf am Raspberry Pi Pico gedrückt und verbinden Sie ihn über USB mit einem Computer.
3. Kopieren Sie die `.uf2`-Datei auf das erscheinende Laufwerk des Pico.
4. Der Pico startet automatisch mit der neuen Firmware.

## Build-Prozess
Der Build-Prozess wird über GitHub Actions automatisiert. Die notwendigen Bibliotheken (`pico-sdk`, `usb_midi_host`, `pio_midi_uart_lib`, `ring_buffer_lib`, `Pico-PIO-USB`) werden während des Builds geklont, und die Firmware wird als `midipico.uf2` ausgegeben.

Für Details zum Build-Prozess siehe `.github/workflows/build.yml`.
