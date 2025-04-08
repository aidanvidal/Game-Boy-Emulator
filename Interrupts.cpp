#include "Interrupts.h"

Interrupts::Interrupts() : IE(0), IF(0), IME(false) {
  // Constructor implementation
}

void Interrupts::resetInterrupts() {
  IE = 0;      // Reset the IE register
  IF = 0;      // Reset the IF register
  IME = false; // Reset the IME flag
}

void Interrupts::writeIE(BYTE value) {
  IE = value; // Write to the IE register
}

void Interrupts::writeIF(BYTE value) {
  IF = value; // Write to the IF register
}

BYTE Interrupts::readIE() const {
  return IE; // Read from the IE register
}

BYTE Interrupts::readIF() const {
  return IF; // Read from the IF register
}

void Interrupts::setIME(bool enable) {
  IME = enable; // Set the IME flag
}

bool Interrupts::getIME() const {
  return IME; // Get the IME flag
}

void Interrupts::handleInterrupts() {
  // TODO: Implement interrupt handling logic
}

// Write to specific interrupt flags
void Interrupts::setVBlankFlag(bool value) {
  IF = (IF & 0xFE) | (value ? 1 : 0); // Set V-Blank flag
}
void Interrupts::setLCDStatFlag(bool value) {
  IF = (IF & 0xFD) | (value ? 2 : 0); // Set LCD STAT flag
}
void Interrupts::setTimerFlag(bool value) {
  IF = (IF & 0xFB) | (value ? 4 : 0); // Set Timer flag
}
void Interrupts::setSerialFlag(bool value) {
  IF = (IF & 0xF7) | (value ? 8 : 0); // Set Serial flag
}
void Interrupts::setJoypadFlag(bool value) {
  IF = (IF & 0xEF) | (value ? 16 : 0); // Set Joypad flag
}
// Read specific interrupt flags
BYTE Interrupts::readVBlankFlag() const { return IF & 0x01; }  // V-Blank
BYTE Interrupts::readLCDStatFlag() const { return IF & 0x02; } // LCD STAT
BYTE Interrupts::readTimerFlag() const { return IF & 0x04; }   // Timer
BYTE Interrupts::readSerialFlag() const { return IF & 0x08; }  // Serial
BYTE Interrupts::readJoypadFlag() const { return IF & 0x10; }  // Joypad
