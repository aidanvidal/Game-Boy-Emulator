#include "NoMBC.h"

NoMBC::NoMBC(BYTE *romData, int size) : rom(romData), romSize(size) {
  // Constructor
}

NoMBC::~NoMBC() {
  // Destructor
  free(rom); // Free the allocated memory for ROM data
}

BYTE NoMBC::readData(WORD address) const {
  // Read data from ROM
  return rom[address & 0x7FFF]; // Mask address to 15 bits
}