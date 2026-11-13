# Game-Boy-Emulator
Lightweight DMG GameBoy Emulator written in C
<br>
## Core Features
### CPU
Fully working and bit-accurate CPU tested using Blargg's [cpu_instrs](https://github.com/aquach/gameboy/tree/master/test-roms/blargg/cpu_instrs) and [instr_timing](https://github.com/aquach/gameboy/tree/master/test-roms/blargg/instr_timing) test ROMs. The CPU features:
* Full SM83 instruction set execution
* Robust interrupt handling
* 4.19 MHz T-cycle clock

### Memory
Full 64KiB system bus accurate to original GameBoy hardware, tested using Blargg's [mem_timing](https://github.com/aquach/gameboy/tree/master/test-roms/blargg/mem_timing) and [mem_timing-2](https://github.com/aquach/gameboy/tree/master/test-roms/blargg/mem_timing-2) test ROMs. Includes:
* Cartridge ROM/RAM/switchable banks
* Support for no-MBC, MBC1, MBC3, MBC5 cartridges
* RAM enable, ROM bank, RAM bank registers
* Object Attribute Memory DMA
* Embedded boot ROM (scrolling Nintendo logo)

### Pixel Processing Unit
Detailed implementation of the GameBoy PPU, featuring:
* Complete 4-mode PPU cycle implementation
* Full tile data addressing
* Accurate sprite rendering

### Audio Processing Unit
Complete 4-channel APU containing implementations for:
* Duty for square-wave channels 1 and 2
* Sweep for channel 1
* Wave RAM (for custom wave data) for wave channel 3
* Envelope for square-wave channels 1 and 2, as well as noise channel 4
* Length counters, frequency, triggers
* Master volume, stereo panning, mixer, DC blocker
* 32768 Hz 2-speaker stereo output

### Input
Input handling for both keyboard and controller, with the following mappings:
#### Keyboard
* **D-Pad mapping:** WASD or Arrow Keys
* **B/A buttons mapping:** J/K or NumPad 1/2
* **Start/Select buttons mapping:** Enter/Tab or NumPad-Enter/Numpad-Plus
#### Controller
* **D-Pad mapping:** D-Pad or left analog stick
* **B/A buttons mapping:** South/East or West/North buttons
* **Start/Select Buttons:** Controller-native start/select buttons

### Frontend
Lightweight frontend implemented with SDL3 features:
* Nearest-Neighbor scaling for crisp visuals
* Letterbox representation for fullscreen mode
* Cursor hiding
* Fullscreen toggle with `F11`

### Persistence
Always-powered SRAM hardware emulated by saving SRAM data on-disk as a `.sav` file


## Installation
1. Make sure you have SDL3 installed
2. Clone the repository
```
git clone https://github.com/Shaj2311/Game-Boy-Emulator
cd ./Game-Boy-Emulator
```
3. Compile the binary
```
make all
```

## Usage
Use the following to load a cartridge
```
./gb <Path to Cartridge>
```
Use the following for a detailed help message including usage and controls
```
./gb -h
./gb --help
```

## References
The following documentation was used for reference during development:
- [Pandocs](https://gbdev.io/pandocs/) was used as the main reference. It contains comprehensive information about GameBoy architecture, register layouts, instruction opcodes, internal workings and quirks.
- [RGBDS documentation](https://rgbds.gbdev.io/docs/v1.0.1/) was used when building the instruction set implementations. It details the functionality of every opcode in the SM83 instruction set, explaining what it does and what flags are affected.
