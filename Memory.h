#ifndef MEMORY_H
#define MEMORY_H

#include "APU/APU.h"
#include "Cartridges/Cartridge.h"
#include "GPU.h"
#include "Input.h"
#include "Interrupts.h"
#include "Timers.h"
#include "WRAM.h"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;

class Memory {
private:
  // Memory map
  Cartridge *cartridge;   // Pointer to the cartridge
  BYTE highRAM[0x7F];     // High RAM (0xFF80 - 0xFFFF)
  WRAM *wram;             // WRAM object
  Interrupts *interrupts; // Interrupts object
  APU *apu;               // APU object
  Timers *timers;         // Timers object
  Input *input;           // Input object
  bool bootROM;           // Boot ROM flag
  BYTE key1;              // Key1 register (CGB only)

  // DMA Registers
  BYTE OAMDMA;
  BYTE HDMA1, HDMA2, HDMA3, HDMA4, HDMA5;

  void OAMDMATransfer();
  void VRAMDMATransfer();

public:
  Memory(const std::string filename);
  Memory(Cartridge *cartridge, Interrupts *interrupts, Timers *timers, GPU *gpu,
         Input *input, APU *apu, WRAM *wram, bool CBG);
  ~Memory();
  bool CBG; // CGB mode flag

  // Memory operations
  BYTE readByte(WORD address) const;
  void writeByte(WORD address, BYTE value);
  WORD readWord(WORD address) const;
  void writeWord(WORD address, WORD value);
  void writeByteNoProtect(WORD address, BYTE value); // For testing
  void loadCartridge(const std::string filename);
  void updateCycles(int cycles);
  void updateTimers(int cycles);
  void renderGPU(SDL_Renderer *ren);
  GPU *gpu;               // GPU object

};

#endif