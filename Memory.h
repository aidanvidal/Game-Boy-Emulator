#ifndef MEMORY_H
#define MEMORY_H

#include "APU/APU.h"
#include "GPU.h"
#include "Interrupts.h"
#include "Timers.h"
#include "WRAM.h"
#include <cstdint>

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;

class Memory {
private:
  // Memory map
  // TODO: Added MBC memory map and cartridge logic
  BYTE highRAM[0x7F];     // High RAM (0xFF80 - 0xFFFF)
  WRAM *wram;             // WRAM object
  GPU *gpu;               // GPU object
  Interrupts *interrupts; // Interrupts object
  APU *apu;               // APU object
  Timers *timers;         // Timers object
  bool CBG;               // CGB mode flag
  bool bootROM;           // Boot ROM flag
  BYTE key1;              // Key1 register (CGB only) used for double speed mode
  // Helper functions
  void OAMDMATransfer(BYTE source); // OAM DMA transfer

public:
  Memory(const char *filename); // Constructor
  ~Memory();                    // Destructor

  // Read and write functions
  BYTE readData(WORD address) const;        // Read data from memory
  void writeData(WORD address, BYTE value); // Write data to memory
  void loadCartridge(const char *filename); // Load cartridge data
  void saveCartridge(const char *filename); // Save cartridge data
};
#endif