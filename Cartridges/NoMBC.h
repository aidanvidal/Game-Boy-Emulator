#ifndef NOMBC_H
#define NOMBC_H
#include "Cartridge.h"

class NoMBC : public Cartridge {
private:
  BYTE *rom;   // Pointer to ROM data
  int romSize; // Size of the ROM
public:
  // Constructor
  NoMBC(BYTE *romData, int size);
  ~NoMBC();

  // Read and write functions
  BYTE readData(WORD address) const override; // Read data from memory
  void writeData(WORD address, BYTE data) override {
    // No write operation for NoMBC
  } // Write data to memory
};

#endif