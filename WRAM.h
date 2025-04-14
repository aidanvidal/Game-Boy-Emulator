#ifndef WRAM_H
#define WRAM_H

#include <cstdint>

typedef uint8_t BYTE;
typedef uint16_t WORD;

class WRAM {
private:
  // On classic gameboy, WRAM is 8KB (0x2000 bytes)
  // but in CGB mode, it can be 32KB (0x8000 bytes)
  // The WRAM banks are 4KB each and there are 8 banks in CGB mode
  // In DMG mode, there is only two banks (0 and 1)
  BYTE RAM[0x8000];
  BYTE WRAMBank; // Current WRAM bank only for CGB mode (Always 1 in DMG mode)
public:
  WRAM();  // Constructor
  ~WRAM(); // Destructor

  // Read and write functions
  BYTE readData(WORD address) const;        // Read data from WRAM
  void writeData(WORD address, BYTE value); // Write data to WRAM

  // Set the current WRAM bank (CGB only)
  void setWRAMBank(WORD bank);
};

#endif