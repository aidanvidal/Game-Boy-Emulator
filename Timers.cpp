#include "Timers.h"

Timers::Timers()
    : DIV(0), TIMA(0), TMA(0), TAC(0), timaCounter(0), TIMA_Enabled(false),
      timaCycles(0), divCounter(0) {
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
void Timers::writeDIV() {
  DIV = 0; // Write to the DIV register, any write resets it to 0
}
void Timers::writeTIMA(uint8_t value) {
  TIMA = value; // Write to the TIMA register
}
void Timers::writeTMA(uint8_t value) {
  TMA = value; // Write to the TMA register
}
void Timers::writeTAC(uint8_t value) {
  TAC = value;                        // Write to the TAC register
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
}
uint8_t Timers::readDIV() {
  return DIV; // Read from the DIV register
}
uint8_t Timers::readTIMA() {
  return TIMA; // Read from the TIMA register
}
uint8_t Timers::readTMA() {
  return TMA; // Read from the TMA register
}
uint8_t Timers::readTAC() {
  return TAC; // Read from the TAC register
}
void Timers::resetDIV() {
  DIV = 0; // Reset the DIV register
}
void Timers::resetTIMA() {
  TIMA = 0; // Reset the TIMA register
}
void Timers::resetTMA() {
  TMA = 0; // Reset the TMA register
}
void Timers::resetTAC() {
  TAC = 0; // Reset the TAC register
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
    while(timaCounter >= timaCycles){
        timaCounter -= timaCycles; // Reset the cycle counter
        TIMA++; // Increment the TIMA register
        if(TIMA == 0){
            // If TIMA overflows, set it to TMA
            TIMA = TMA;
            // Request an interrupt
            //TODO: Set interrupt flag
        }
    }
  }
}