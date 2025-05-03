#include "MBC1.h"

MBC1::MBC1(uint8_t *romData, unsigned int romSize, unsigned int ramSize)
    : rom(romData), romSize(romSize), ramSize(ramSize) {
  romBankNumber = 1; // ROM bank 0 is fixed, start with bank 1
  ram = new uint8_t[ramSize];
}

MBC1::~MBC1() {
  if (battery && ramNewData) {
    saveBatteryData();
  }
  free(rom);
  free(ram);
}

void MBC1::writeData(uint16_t address, uint8_t data) {
  if (address <= 0x1FFF) {
    // RAM Enable/Disable (0x0000-0x1FFF)
    bool newRamEnable = (data & 0xF) == 0xA;

    // If RAM is being disabled and battery-backed RAM has changes, save it
    if (battery && ramEnable && !newRamEnable && ramNewData) {
      saveBatteryData();
      ramNewData = false;
    }
    ramEnable = newRamEnable;
  } else if (address <= 0x3FFF) {
    // ROM Bank Number (0x2000-0x3FFF)
    // Values of 0 are treated as 1
    romBankNumber = (data & 0x1F) == 0 ? 1 : (data & 0x1F);
  } else if (address <= 0x5FFF) {
    // RAM Bank Number or Upper ROM Bank Bits (0x4000-0x5FFF)
    romRamBankNumber = data & 0x03;
  } else if (address <= 0x7FFF) {
    // Banking Mode Select (0x6000-0x7FFF)
    romRamMode = (data == 0x01);
  } else if (address >= 0xA000 && address <= 0xBFFF) {
    // External RAM Access (0xA000-0xBFFF)
    if (ramEnable && ramSize > 0) {
      uint32_t ramAddress;

      if (romRamMode) {
        // RAM Banking mode: use romRamBankNumber as bank selector
        ramAddress = ((address & 0x1FFF) | (romRamBankNumber << 13));
      } else {
        // ROM Banking mode: always use bank 0 for RAM
        ramAddress = (address & 0x1FFF);
      }

      // Apply size mask to handle mirroring
      ram[ramAddress & (ramSize - 1)] = data;
      ramNewData = true;
    }
  }
}

uint8_t MBC1::readData(uint16_t address) {
  if (address <= 0x7FFF) {
    // ROM Access (0x0000-0x7FFF)
    uint32_t romAddress = address & 0x3FFF;

    // Handle upper ROM bank (0x4000-0x7FFF)
    if (address & 0x4000) {
      if (romRamMode) {
        // RAM Banking mode: only use romBankNumber
        romAddress |= (romBankNumber << 14);
      } else {
        // ROM Banking mode: use combined bank number
        romAddress |= (((romRamBankNumber << 5) | romBankNumber) << 14);
      }

      // Apply size mask to handle mirroring
      romAddress &= (romSize - 1);
    }

    return rom[romAddress];
  } else if (address >= 0xA000 && address <= 0xBFFF) {
    // External RAM Access (0xA000-0xBFFF)
    if (ramEnable && ramSize > 0) {
      uint32_t ramAddress;

      if (romRamMode) {
        // RAM Banking mode: use romRamBankNumber as bank selector
        ramAddress = ((address & 0x1FFF) | (romRamBankNumber << 13));
      } else {
        // ROM Banking mode: always use bank 0 for RAM
        ramAddress = (address & 0x1FFF);
      }

      // Apply size mask to handle mirroring
      return ram[ramAddress & (ramSize - 1)];
    }
    return 0x00; // Return 0 when RAM is disabled or size is 0
  }

  return 0; // Default for unhandled memory ranges
}

// Set the battery location for saving/loading battery-backed RAM data
void MBC1::setBatteryLocation(string inBatteryPath) {
  battery = false;
  batteryPath = inBatteryPath;

  // Try to load battery-backed RAM data
  if (Cartridge::loadBatteryFile(ram, ramSize, batteryPath)) {
    battery = true;
  }
}

// Save battery-backed RAM data to file
void MBC1::saveBatteryData() {
  if (battery) {
    Cartridge::saveBatteryFile(ram, ramSize, batteryPath);
    ramNewData = false;
  }
}
