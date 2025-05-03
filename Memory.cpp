#include "Memory.h"
#include "APU/APU.h"
#include "Cartridges/MBC1.h"
#include "Cartridges/MBC3.h"
#include "Cartridges/MBC5.h"
#include "Cartridges/NoMBC.h"
#include <cstdint>
#include <fstream>
#include <iostream>

Memory::Memory(const std::string filename) {
  loadCartridge(filename);
  interrupts = new Interrupts();
  gpu = new GPU(interrupts, CBG, this);
  wram = new WRAM();
  apu = new APU();
  timers = new Timers(interrupts);
  input = new Input(interrupts);
  memset(highRAM, 0, sizeof(highRAM));
  bootROM = false;
}

Memory::Memory(Cartridge *cartridge, Interrupts *interrupts, Timers *timers,
               GPU *gpu, Input *input, APU *apu, WRAM *wram, bool CBG)
    : cartridge(cartridge), interrupts(interrupts), timers(timers), gpu(gpu),
      input(input), apu(apu), wram(wram), CBG(CBG) {
  memset(highRAM, 0, sizeof(highRAM));
  bootROM = false;
}

Memory::~Memory() {
  delete interrupts;
  delete gpu;
  delete wram;
  delete apu;
  delete timers;
  delete input;
  delete cartridge;
}

void Memory::writeByte(WORD address, BYTE data) {

  // TODO: Handle MBC (Memory Bank Controller) logic
  // external RAM (also apart of cartridge)

  if (address >= 0x0000 && address <= 0x7FFF) {
    cartridge->writeData(address, data); // Write to cartridge ROM
    return;
  }

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

BYTE Memory::readByte(WORD address) const {
  // Read from cartridge ROM (and handle MBC logic)
  if (address <= 0x7FFF) {
    if (bootROM && address < 0x0100)
      return 0xFF; // Boot ROM placeholder
    return cartridge->readData(address);
  }

  // Read from VRAM
  if (address >= 0x8000 && address <= 0x9FFF) {
    return gpu->readData(address);
  }

  // Read from Cartridge RAM
  if (address >= 0xA000 && address <= 0xBFFF) {
    return cartridge->readData(address);
  }

  // Read from WRAM
  if (address >= 0xC000 && address <= 0xFDFF) {
    return wram->readData(address);
  }

  // Read from OAM
  if (address >= 0xFE00 && address <= 0xFE9F) {
    return gpu->readData(address);
  }

  // Read from I/O Ports
  if (address >= 0xFF00 && address <= 0xFF7F) {
    // Joypad
    if (address == 0xFF00) {
      return input->readJoypadState();
    }

    // Serial Transfer
    if (address == 0xFF01 || address == 0xFF02) {
      // TODO: Handle serial transfer
      return 0xFF;
    }

    // Timer
    if (address >= 0xFF04 && address <= 0xFF07) {
      return timers->readData(address);
    }

    // Interrupts
    if (address == 0xFF0F) {
      return interrupts->readIF();
    }

    // APU
    if (address >= 0xFF10 && address <= 0xFF3F) {
      return apu->getData(address);
    }

    // GPU registers
    if (address >= 0xFF40 && address <= 0xFF4B) {
      return gpu->readData(address);
    }

    // Key 1
    if (address == 0xFF4D) {
      return key1;
    }

    // VRAM Bank
    if (address == 0xFF4F) {
      return gpu->readData(address);
    }

    // Boot ROM toggle
    if (address == 0xFF50) {
      return bootROM ? 0x00 : 0x01;
    }

    // VRAM DMA registers
    if (address >= 0xFF51 && address <= 0xFF55) {
      switch (address) {
      case 0xFF51:
        return HDMA1;
      case 0xFF52:
        return HDMA2;
      case 0xFF53:
        return HDMA3;
      case 0xFF54:
        return HDMA4;
      case 0xFF55:
        return HDMA5;
      default:
        return 0xFF;
      }
    }

    // Palettes
    if (address >= 0xFF68 && address <= 0xFF6B) {
      return gpu->readData(address);
    }

    // WRAM Bank
    if (address == 0xFF70) {
      return wram->readData(address);
    }

    // Return 0xFF for unmapped I/O registers
    return 0xFF;
  }

  // Read from High RAM
  if (address >= 0xFF80 && address <= 0xFFFE) {
    return highRAM[address - 0xFF80];
  }

  // Interrupt Enable Register
  if (address == 0xFFFF) {
    return interrupts->readIE();
  }

  // Default return for unmapped memory
  return 0xFF;
}

WORD Memory::readWord(WORD address) const {
  BYTE low = readByte(address);
  BYTE high = readByte(address + 1);
  return (high << 8) | low;
}

void Memory::writeWord(WORD address, WORD value) {
  BYTE low = value & 0xFF;
  BYTE high = (value >> 8) & 0xFF;
  writeByte(address, low);
  writeByte(address + 1, high);
}

void Memory::writeByteNoProtect(WORD address, BYTE value) {
  writeByte(address, value); // Write without protection
}

void Memory::OAMDMATransfer() {
  uint16_t readSource = OAMDMA << 8; // Destination address in OAM
  for (int i = 0; i < 0xA0; i++) {
    gpu->writeData(0xFE00 + i, readByte(readSource + i));
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
      gpu->writeData(writeDest + i, readByte(readSource + i));
    }
    HDMA5 = 0; // Clear the HDMA5 register
  }
}

void Memory::loadCartridge(const std::string filename) {
  std::ifstream romFile(filename, std::ios::binary | std::ios::ate);
  if (!romFile.is_open()) {
    std::cerr << "Error opening ROM file." << std::endl;
    exit(1);
  }
  unsigned int romSize = romFile.tellg();
  BYTE *romData = new BYTE[romSize];
  romFile.seekg(0);
  romFile.read(reinterpret_cast<char *>(romData), romSize);
  romFile.close();

  // Print out ROM header information

  // Title
  BYTE title[16];
  for (int i = 0; i < 16; i++) {
    title[i] = romData[0x0134 + i]; // Read the title from the ROM header
  }
  std::cout << "Title: ";
  for (int i = 0; i < 16; i++) {
    std::cout << title[i]; // Print the title
  }
  std::cout << std::endl;

  // CGB Flag
  BYTE cgbFlag = romData[0x0143]; // Read the CGB flag from the ROM header
  if (cgbFlag == 0xC0 || cgbFlag == 0x80) {
    CBG = true; // Set CBG mode flag
    std::cout << "CGB Mode: Yes" << std::endl;
  } else {
    CBG = false; // Set CBG mode flag
    std::cout << "CGB Mode: No" << std::endl;
  }

  // ROM Size
  BYTE romSizeFlag =
      romData[0x0148]; // Read the ROM size flag from the ROM header
  int romSizeValue = 0;
  if (romSizeFlag <= 0x07) {
    romSizeValue = 32 << romSizeFlag; // Calculate ROM size
  } else {
    romSizeValue = 0; // Invalid ROM size
  }
  if (romSizeValue != 0) {
    std::cout << "ROM Size: " << romSizeValue << " KB" << std::endl;
  } else {
    std::cout << "Invalid ROM Size" << std::endl;
  }

  // RAM Size
  BYTE ramSizeFlag =
      romData[0x0149]; // Read the RAM size flag from the ROM header
  // RAM size values
  // The 0x800 is there because apparently the 1 slot was "Listed in various
  // unofficial docs as 2 KiB" but officially it is listed as unused.
  int ramSizes[6] = {0, 0x800, 0x2000, 0x8000, 0x20000, 0x80000};
  int ramSizeValue = 0;
  switch (ramSizeFlag) {
  case 0x00:
    ramSizeValue = 0; // No RAM
    break;
  case 0x01:
    ramSizeValue = 2; // Unused (kind of)
    break;
  case 0x02:
    ramSizeValue = 8; // 8 KB
    break;
  case 0x03:
    ramSizeValue = 32; // 32 KB
    break;
  case 0x04:
    ramSizeValue = 128; // 128 KB
    break;
  case 0x05:
    ramSizeValue = 64; // 64 KB
    break;
  default:
    ramSizeValue = 0; // Invalid RAM size
    break;
  }
  if (ramSizeValue != 0) {
    std::cout << "RAM Size: " << ramSizeValue << " KB" << std::endl;
  } else {
    std::cout << "Invalid RAM Size" << std::endl;
  }
  ramSizeValue =
      ramSizes[ramSizeFlag]; // Set the actual RAM size value in Bytes

  // Battery Things
  std::string batteryPath = filename;
  batteryPath.replace(batteryPath.end() - 4, batteryPath.end(), ".sav");

  // MBC Type
  BYTE mbcType = romData[0x0147]; // Read the MBC type from the ROM header
  std::cout << "MBC Type: ";

  // Switch case for MBC type
  switch (mbcType) {
  case 0x00:                                 // No MBC
    cartridge = new NOMBC(romData, romSize); // Create NoMBC object
    std::cout << "No MBC" << std::endl;
    break;
  case 0x01:                                   // MBC1
    cartridge = new MBC1(romData, romSize, 0); // Create MBC1 object
    std::cout << "MBC1" << std::endl;
    break;
  case 0x02: // MBC1 + RAM
    cartridge =
        new MBC1(romData, romSize, ramSizeValue); // Create MBC1 + RAM object
    std::cout << "MBC1 + RAM" << std::endl;
    break;
  case 0x03: // MBC1 + RAM + BATTERY
    std::cout << "MBC1 + RAM + BATTERY" << std::endl;
    cartridge = new MBC1(romData, romSize,
                         ramSizeValue); // Create MBC1 + RAM + BATTERY object
    cartridge->setBatteryLocation(batteryPath); // Set battery location
    break;
  case 0x0F: // MBC3 + TIMER + BATTERY
    std::cout << "MBC3 + TIMER + BATTERY" << std::endl;
    cartridge = new MBC3(romData, romSize, 0,
                         true); // Create MBC3 + TIMER + BATTERY object
    cartridge->setBatteryLocation(batteryPath); // Set battery location
    break;
  case 0x10: // MBC3 + TIMER
    std::cout << "MBC3 + TIMER" << std::endl;
    cartridge = new MBC3(romData, romSize, 0,
                         true); // Create MBC3 + TIMER object
    break;
  case 0x11: // MBC3
    std::cout << "MBC3" << std::endl;
    cartridge = new MBC3(romData, romSize, 0,
                         false); // Create MBC3 object
    break;
  case 0x12: // MBC3 + RAM
    std::cout << "MBC3 + RAM" << std::endl;
    cartridge = new MBC3(romData, romSize, ramSizeValue,
                         false); // Create MBC3 + RAM object
    break;
  case 0x13: // MBC3 + RAM + BATTERY
    std::cout << "MBC3 + RAM + BATTERY" << std::endl;
    cartridge = new MBC3(romData, romSize, ramSizeValue,
                         false); // Create MBC3 + RAM + BATTERY object
    cartridge->setBatteryLocation(batteryPath); // Set battery location
    break;
  case 0x19: // MBC5
    std::cout << "MBC5" << std::endl;
    cartridge = new MBC5(romData, romSize, 0); // Create MBC5 object
    break;
  case 0x1A: // MBC5 + RAM
    std::cout << "MBC5 + RAM" << std::endl;
    cartridge = new MBC5(romData, romSize, ramSizeValue); // Create MBC5 + RAM
                                                          // object
    break;
  case 0x1B: // MBC5 + RAM + BATTERY
    std::cout << "MBC5 + RAM + BATTERY" << std::endl;
    cartridge = new MBC5(romData, romSize, ramSizeValue); // Create MBC5 + RAM
                                                          // + BATTERY object
    cartridge->setBatteryLocation(batteryPath); // Set battery location
    break;
  case 0x1C: // MBC5 + RUMBLE
    std::cout << "MBC5 + RUMBLE" << std::endl;
    cartridge = new MBC5(romData, romSize, 0); // Create MBC5 + RUMBLE object
    break;
  case 0x1D: // MBC5 + RUMBLE + RAM
    std::cout << "MBC5 + RUMBLE + RAM" << std::endl;
    cartridge =
        new MBC5(romData, romSize, ramSizeValue); // Create MBC5 + RUMBLE
                                                  // + RAM object
    break;
  case 0x1E: // MBC5 + RUMBLE + RAM + BATTERY
    std::cout << "MBC5 + RUMBLE + RAM + BATTERY" << std::endl;
    cartridge =
        new MBC5(romData, romSize, ramSizeValue); // Create MBC5 + RUMBLE +
                                                  // RAM + BATTERY object
    cartridge->setBatteryLocation(batteryPath);   // Set battery location
    break;
  default:
    std::cout << "Unknown MBC Type" << std::endl;
    break;
  }
}

void Memory::updateCycles(int cycles) {
  gpu->updateGPU(cycles);           // Update GPU cycles
  apu->updateChannelTimers(cycles); // Update APU channel timers
  apu->apuStep(cycles);             // Update APU
  apu->getAudioSample(cycles); // Get APU audio sample
}
void Memory::updateTimers(int cycles) { timers->updateTimers(cycles); }
void Memory::renderGPU(SDL_Renderer *ren) {
  gpu->renderFrame(ren); // Render GPU frame
}