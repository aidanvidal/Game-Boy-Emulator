#include "Memory.h"
#include <cstdint>

Memory::Memory(const char *filename) {
  // Load game cartridge
  // TODO: Load game cartridge data
  // This will set the CBG mode flag
  // and load the proper MBC
  loadCartridge(filename);

  // Initialize the interrupts
  interrupts = new Interrupts();

  // Initialize the GPU
  gpu = new GPU(interrupts, CBG, this);

  // Initialize the WRAM
  wram = new WRAM();

  // Initialize the APU
  apu = new APU();

  // Initialize the Timers
  timers = new Timers(interrupts);

  // Initialize the Input
  input = new Input();

  // Initialize the high RAM
  for (int i = 0; i < 0x80; i++) {
    highRAM[i] = 0; // Initialize high RAM with zeros
  }

  // Initialize the boot ROM
  bootROM = true; // Set boot ROM flag to true
}

Memory::~Memory() {
  // Destructor
  delete interrupts;
  delete gpu;
  delete wram;
  delete apu;
  delete timers;
  delete input;
}

void Memory::writeData(WORD address, BYTE data) {

  // TODO: Handle MBC (Memory Bank Controller) logic
  // external RAM (also apart of cartridge)

  // VRAM
  if (address >= 0x8000 && address <= 0x9FFF) {
    gpu->writeData(address, data); // Write to VRAM
    return;
  }

  // WRAM
  if (address >= 0xC000 && address <= 0xFDFF) {
    wram->writeData(address, data); // Write to WRAM
    return;
  }

  // OAM
  if (address >= 0xFE00 && address <= 0xFE9F) {
    gpu->writeData(address, data); // Write to OAM
    return;
  }

  // I/O Ports
  // Joypad
  if (address == 0xFF00) {
    input->updateJoypadState(data); // Update joypad state
    return;
  }
  // Serial Transfer
  if (address == 0xFF01 || address == 0xFF02) {
    // TODO: Handle serial transfer
    return;
  }
  // Timer
  if (address >= 0xFF04 && address <= 0xFF07) {
    timers->writeData(address, data); // Write to timer
    return;
  }
  // Interrupts
  if (address == 0xFFFF || address == 0xFF0F) {
    if (address == 0xFFFF) {
      interrupts->writeIE(data); // Write to IE register
    } else {
      interrupts->writeIF(data); // Write to IF register
    }
  }
  // APU
  if (address >= 0xFF10 && address <= 0xFF3F) {
    apu->writeData(address, data); // Write to APU
    return;
  }
  // GPU
  if (address >= 0xFF40 && address <= 0xFF4B) {
    if (address == 0xFF46) {
      OAMDMA = data;    // Set OAM DMA register
      OAMDMATransfer(); // OAM DMA transfer
    } else {
      gpu->writeData(address, data); // Write to GPU
    }
    return;
  }
  // Key 1
  if (address == 0xFF4D) {
    key1 = data; // Set key 1 register
    return;
  }
  // VRAM Bank
  if (address == 0xFF4F) {
    gpu->writeData(address, data); // Set VRAM bank
    return;
  }
  // Boot ROM
  if (address == 0xFF50) {
    if (data != 0) {
      bootROM = false; // Disable boot ROM
    } else {
      bootROM = true;
    }
    return;
  }
  // VRAM DMA
  if (address >= 0xFF51 && address <= 0xFF55) {
    switch (address) {
    case 0xFF51: // HDMA1
      HDMA1 = data;
      break;
    case 0xFF52: // HDMA2
      HDMA2 = data;
      break;
    case 0xFF53: // HDMA3
      HDMA3 = data;
      break;
    case 0xFF54: // HDMA4
      HDMA4 = data;
      break;
    case 0xFF55: // HDMA5
      HDMA5 = data;
      VRAMDMATransfer();
      break;
    }
    return;
  }
  // Palettes
  if (address >= 0xFF68 && address <= 0xFF6B) {
    gpu->writeData(address, data); // Write to background palette
    return;
  }
  // WRAM Bank
  if (address == 0xFF70) {
    wram->setWRAMBank(data); // Set WRAM bank
    return;
  }

  // High RAM
  if (address >= 0xFF80 && address <= 0xFFFE) {
    highRAM[address - 0xFF80] = data; // Write to high RAM
    return;
  }
}

void Memory::OAMDMATransfer() {
  uint16_t readSource = OAMDMA << 8; // Destination address in OAM
  for (int i = 0; i < 0xA0; i++) {
    gpu->writeData(0xFE00 + i, readData(readSource + i));
  }
}

void Memory::VRAMDMATransfer() {
  uint16_t readSource = HDMA1 << 8 | (HDMA2 & 0xF0); // Source address
  uint16_t writeDest = HDMA3 << 8 | (HDMA4 & 0xF0);  // Destination address
  uint16_t length = (HDMA5 & 0x7F) + 1;              // Length of transfer
  // If bit 7 = 1, HBlank DMA, bit 7 = 0 General purpose DMA
  bool hblank = (HDMA5 & 0x80) != 0;
  gpu->setHDMA(length, readSource, writeDest, hblank); // Set HDMA
  if (!hblank) {
    // Preform the general purpose DMA transfer
    for (int i = 0; i < length; i++) {
      gpu->writeData(writeDest + i, readData(readSource + i));
    }
    HDMA5 = 0; // Clear the HDMA5 register
  }
}