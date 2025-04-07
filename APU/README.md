# Game Boy APU Emulator

A cycle-accurate implementation of the Nintendo Game Boy's Audio Processing Unit (APU) with SDL audio output.

## Features

- **Four Channel Emulation**:
  - Channel 1: Square wave with sweep
  - Channel 2: Square wave
  - Channel 3: Wave pattern (4-bit DAC)
  - Channel 4: Noise (LFSR-based)

- **Accurate Timing**:
  - 512Hz frame sequencer
  - 256Hz length counters
  - 128Hz frequency sweep
  - 64Hz volume envelopes

- **SDL Audio Integration**:
  - 44.1kHz sample rate
  - 32-bit float samples
  - Dynamic buffering
  - Automatic underrun protection

## Requirements

- C++17 compatible compiler
- SDL2 library
- CMake (for building)

## Building

```bash
make APU_emulator
```

## TODO
- Fix some niche issues and bugs that arise during further testing

---

Check out my [blog post](https://aidanvidal.github.io/posts/GameBoy_Dev_APU.html) for more detailed information.