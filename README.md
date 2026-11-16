# CYD MIDI Controller

Touchscreen Bluetooth MIDI controller for the ESP32-2432S028R "Cheap Yellow Display" (CYD).

Special thanks to Brian Lough for putting together the resources on this board. Check out his repo for more examples: https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display

## Features

### 10 Interactive Modes

- **KEYS** - Virtual piano keyboard with scale and key controls
- **BEATS** - 16-step sequencer with 4 tracks and tempo control
- **ZEN** - Ambient bouncing ball mode for generative music
- **DROP** - Physics-based ball drop with customizable platforms
- **RNG** - Random music generator for creative exploration
- **XY PAD** - Touch-controlled X/Y pad for real-time parameter control
- **ARP** - Arpeggiator with chord-based patterns
- **GRID** - Grid piano with 4ths layout for unique playing style
- **CHORD** - Auto-chord mode with diatonic chord progressions
- **LFO** - Low-frequency oscillator for modulation effects

### Core Features

- **Bluetooth MIDI** - Wireless connection to DAWs and music software
- **Touchscreen Interface** - Intuitive visual controls optimized for the CYD display
- **Real-time Control** - Low-latency MIDI output
- **Visual Feedback** - Responsive graphics

## What You Need

- **ESP32-2432S028R (CYD)** - ~$15 from AliExpress/Amazon
- Arduino IDE with ESP32 support

## Installation

### 1. Add ESP32 Board Support
1. Go to `File` → `Preferences`
2. Add to "Additional Boards Manager URLs":
   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
3. Go to `Tools` → `Board` → `Boards Manager`
4. Search "ESP32" and install "ESP32 by Espressif Systems"

### 2. Install Libraries
In Arduino IDE Library Manager, install:
- `TFT_eSPI` by Bodmer
- `XPT2046_Touchscreen` by Paul Stoffregen

### 3. Configure TFT_eSPI
Replace the `libraries/TFT_eSPI/User_Setup.h` with the `User_Setup.h` from the repo.

### 4. Upload Code
1. Clone this repo, or download the ZIP and unzip it. The folder must be named `CYD-MIDI-Controller`
   (rename `CYD-MIDI-Controller-main` if you downloaded the ZIP), with all the `.h` files inside it.
2. Open `CYD-MIDI-Controller.ino` from that folder
3. Select board: `ESP32 Dev Module`
4. Connect CYD and upload
(Lower Upload Speed to `115200` if the sketch isn't uploading)

Tested with "ESP32 by Espressif Systems" 3.3.2.

### 5. Connect
"CYD MIDI" is a Bluetooth MIDI device:
- **Mac**: Audio MIDI Setup → `Window` → `Show MIDI Studio` → the Bluetooth icon → `Connect` next to
  "CYD MIDI". Then select "CYD MIDI" as a MIDI input in your DAW.
- **iPhone/iPad**: connect from your music app's Bluetooth MIDI settings (GarageBand, AUM, Loopy Pro,
  Cubasis...), not from the iPhone's Bluetooth settings.
- **Windows**: many Windows DAWs need a bridge to use Bluetooth MIDI:
  - [MIDIberry](https://apps.microsoft.com/detail/9n39720h2m05): pair "CYD MIDI" in Windows Bluetooth
    settings, route it in MIDIberry to a virtual MIDI port (such as loopMIDI), then select that port in your DAW.
  - [Perfect Bluetooth MIDI for Windows](https://github.com/mayerwin/Perfect-Bluetooth-MIDI-For-Windows):
    follow its setup, connect "CYD MIDI" in it, then select its MIDI port in your DAW.

## Troubleshooting

- **`keyboard_mode.h: No such file or directory`**: the `.ino` was opened on its own. Open it from the
  `CYD-MIDI-Controller` folder with all the `.h` files (see step 4)
- **Upload Speed**: Lower it to `115200` if the sketch isn't uploading
- **Blank screen**: Check TFT_eSPI pin configuration
- **No touch**: Verify touchscreen library installation
- **No Bluetooth**: Restart the CYD, then connect as in step 5 (on Mac and iPhone/iPad from MIDI settings,
  not Bluetooth settings)

## License

Open source - see MIT license file for details.
