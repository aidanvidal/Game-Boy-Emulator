#include "MBC5.h"

MBC5::MBC5(uint8_t *romData, unsigned int romSize, unsigned int ramSize)
    : rom(romData), romSize(romSize), ramSize(ramSize) {
  ram = new uint8_t[ramSize];
}

MBC5::~MBC5() {
  // Save battery data if needed before destruction
  if (battery && ramNewData) {
    saveBatteryData();
  }
  free(rom);
  free(ram);
}

void MBC5::writeData(uint16_t address, uint8_t data) {
  if (address <= 0x1FFF) {
    // RAM Enable/Disable (0x0000-0x1FFF)
    bool newRamEnable = (data & 0xF) == 0xA;

    // If RAM is being disabled and battery-backed RAM has changes, save it
    if (battery && ramEnable && !newRamEnable && ramNewData) {
      saveBatteryData();
      ramNewData = false;
    }
    ramEnable = newRamEnable;
  } else if (address <= 0x2FFF) {
    // ROM Bank Number Low Byte (0x2000-0x2FFF)
    romBankNumber = (romBankNumber & 0x100) | data;
  } else if (address <= 0x3FFF) {
    // ROM Bank Number High Bit (0x3000-0x3FFF)
    romBankNumber = (romBankNumber & 0xFF) | ((data & 0x1) << 8);
  } else if (address <= 0x5FFF) {
    // RAM Bank Number (0x4000-0x5FFF)
    ramBankNumber = data & 0xF;
  } else if (address >= 0xA000 && address <= 0xBFFF) {
    // External RAM Access (0xA000-0xBFFF)
    if (ramEnable && ramSize > 0) {
      uint32_t ramAddress = ((address & 0x1FFF) | (ramBankNumber << 13)) & (ramSize - 1);
      ram[ramAddress] = data;
      ramNewData = true;
    }
  }
}

uint8_t MBC5::readData(uint16_t address) {
  if (address <= 0x7FFF) {
    // ROM Access (0x0000-0x7FFF)
    uint32_t romAddress = address & 0x3FFF;

    if (address & 0x4000) {
      // Upper bank (0x4000-0x7FFF)
      romAddress |= romBankNumber << 14;
    }

    return rom[romAddress];
  } else if (address >= 0xA000 && address <= 0xBFFF) {
    // External RAM Access (0xA000-0xBFFF)
    if (ramEnable && ramSize > 0) {
      uint32_t ramAddress = ((address & 0x1FFF) | (ramBankNumber << 13)) & (ramSize - 1);
      return ram[ramAddress];
    }
  }

  return 0xFF;  // Default for unmapped/disabled memory
}

void MBC5::setBatteryLocation(string inBatteryPath) {
  battery = false;
  batteryPath = inBatteryPath;

  if (Cartridge::loadBatteryFile(ram, ramSize, batteryPath)) {
    battery = true;
    ramNewData = false;
  }
}

void MBC5::saveBatteryData() {
  if (battery) {
    Cartridge::saveBatteryFile(ram, ramSize, batteryPath);
    ramNewData = false;
  }
}
