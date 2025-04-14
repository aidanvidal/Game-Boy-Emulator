#include "WRAM.h"

WRAM::WRAM() {
  // Initialize WRAM bank to 1
  WRAMBank = 1;
  // Initialize WRAM with zeros
  for (int i = 0; i < 0x2000; i++) {
    RAM[i] = 0;
  }
}

WRAM::~WRAM() {
  // Destructor
}

void WRAM::setWRAMBank(WORD bank) {
  // Set the current WRAM bank (CGB only)
  bank = bank & 0x7;
  if (bank == 0) {
    WRAMBank = 1; // Set to bank 1
  } else {
    WRAMBank = bank; // Set to the specified bank
  }
}

void WRAM::writeData(WORD address, BYTE data) {
  // Write data to WRAM specified by WRAM bank
  int location = address & 0x1FFF; // Get the offset in the current WRAM bank
  // Check if writing to bank 0
  if (location >= 0x1000) {
    location |= WRAMBank << 12; // Apply WRAM bank selection
  }
  RAM[location] = data; // Write data to the specified address
}

BYTE WRAM::readData(WORD address) const {
  // Trying to read WRAM bank
  if (address == 0xFF70) {
    return WRAMBank;
  }

  // Read data from WRAM specified by WRAM bank
  int location = address & 0x1FFF; // Get the offset in the current WRAM bank
  // Check if reading from bank 0
  if (location >= 0x1000) {
    location |= WRAMBank << 12; // Apply WRAM bank selection
  }
  return RAM[location]; // Return the data at the specified address
}