#include "Timers.h"

Timers::Timers(Interrupts &interrupts)
    : DIV(0), TIMA(0), TMA(0), TAC(0), timaCounter(0), TIMA_Enabled(false),
      timaCycles(0), divCounter(0), interrupts(&interrupts) {
  // Constructor implementation
}

void Timers::resetTimers() {
  DIV = 0;
  TIMA = 0;
  TMA = 0;
  TAC = 0;
  timaCounter = 0;
  TIMA_Enabled = false;
  timaCycles = 0;
  divCounter = 0; // Reset the divider counter
}
void Timers::writeData(WORD address, uint8_t value) {
  switch (address) {
  case 0xFF04: // DIV
    DIV = 0;   // Writing to DIV resets it to 0
    break;
  case 0xFF05:    // TIMA
    TIMA = value; // Write directly to the TIMA register
    break;
  case 0xFF06:   // TMA
    TMA = value; // Write directly to the TMA register
    break;
  case 0xFF07:                          // TAC
    TAC = value;                        // Write directly to the TAC register
    TIMA_Enabled = (value & 0x04) != 0; // Check if the timer is enabled
    switch (value & 0x03) {
    case 0: // 4096 Hz
      timaCycles = 4194304 / 4096;
      break;
    case 1: // 262144 Hz
      timaCycles = 4194304 / 262144;
      break;
    case 2: // 65536 Hz
      timaCycles = 4194304 / 65536;
      break;
    case 3: // 16384 Hz
      timaCycles = 4194304 / 16384;
      break;
    }
    break;
  default:
    break; // Invalid address
  }
}

uint8_t Timers::readData(WORD address) const {
  switch (address) {
  case 0xFF04:   // DIV
    return DIV;  // Read directly from the DIV register
  case 0xFF05:   // TIMA
    return TIMA; // Read directly from the TIMA register
  case 0xFF06:   // TMA
    return TMA;  // Read directly from the TMA register
  case 0xFF07:   // TAC
    return TAC;  // Read directly from the TAC register
  default:
    return 0xFF; // Invalid address, return default value
  }
}

void Timers::updateTimers(WORD cycles) {
  // Increment the DIV register every 256 cycles
  divCounter += cycles;
  if (divCounter >= 256) {
    divCounter -= 256; // Reset the cycle counter
    DIV++;             // Increment the DIV register
  }

  if (TIMA_Enabled) {
    timaCounter += cycles;
    while (timaCounter >= timaCycles) {
      timaCounter -= timaCycles; // Reset the cycle counter
      TIMA++;                    // Increment the TIMA register
      if (TIMA == 0) {
        // If TIMA overflows, set it to TMA
        TIMA = TMA;
        // Request an interrupt
        interrupts->setTimerFlag(true); // Set the timer interrupt flag
      }
    }
  }
}