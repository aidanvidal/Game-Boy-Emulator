#include "MBC1.h"

MBC1::MBC1(uint8_t *romData, unsigned int romSize, unsigned int ramSize)
    : romData(romData), romSize(romSize), ramSize(ramSize) {
  // Make sure rom bank is 1
  romBankNumber = 1;
  extRAM = new uint8_t[ramSize];
}

MBC1::~MBC1() {
  free(romData);
  free(extRAM);
  if (battery) {
    saveBatteryData();
  }
}

void MBC1::writeData(uint16_t address, uint8_t data) {
  if (address >= 0x0000 && address <= 0x1FFF) {
    // If it's being disabled and there's new data written to RAM, save the data
    // and reset the newData flag
    if (battery & ramEnable && (data & 0xF) != 0xA && ramNewData) {
      saveBatteryData();
      ramNewData = false;
    }
    ramEnable = (data & 0xF) == 0xA;
  } else if (address >= 0x2000 && address <= 0x3FFF) {
    if ((data & 0x1F) == 0) {
      data = 1;
    }
    romBankNumber = data & 0x1F;
  } else if (address >= 0x4000 && address <= 0x5FFF) {
    romRamBankNumber = data & 0x03;
  } else if (address >= 0x6000 && address <= 0x7FFF) {
    if (data == 0x00) {
      romRamMode = false;
    } else if (data == 0x01) {
      romRamMode = true;
    }
  } else if (address >= 0xA000 && address <= 0xBFFF) {
    // romRamMode: false = rom, true = ram
    if (ramEnable && ramSize > 0) {
      if (romRamMode == true) {
        // extRAM[romRamBankNumber][address - 0xA000] = data;
        extRAM[((address & 0x1FFF) | (romRamBankNumber << 13)) &
               (ramSize - 1)] = data;
        ramNewData = true;
      } else {
        // extRAM[0][address - 0xA000] = data;
        extRAM[(address & 0x1FFF) & (ramSize - 1)] = data;
        ramNewData = true;
      }
    }
  }
}

uint8_t MBC1::readData(uint16_t address) {
  if (address >= 0x0000 && address <= 0x7FFF) {
    uint32_t returnAddress = address & 0x3FFF; // Use a wide integer for this
    // Upper bank handling. Lower bank needs no help.
    if (address & 0x4000) {
      // romRamMode: false = rom, true = ram
      if (romRamMode) {
        returnAddress |= (romBankNumber << 14);
      } else {
        returnAddress |= (((romRamBankNumber << 5) | romBankNumber) << 14);
      }
      if (returnAddress > romSize) {
        // cout << "Warning: ROM Bank overflow.\n";	// This isn't always an
        // error, but a lot of the times it is, so I use this to help debug in
        // these cases.
      }
      returnAddress &=
          (romSize - 1); // Mask off for mirroring purposes. If it's not a power
                         // of 2 it'll probably fuck up?
    }
    return romData[returnAddress];
  } else if (address >= 0xA000 && address <= 0xBFFF) {
    // romRamMode: false = rom, true = ram
    if (ramEnable && ramSize > 0) {
      if (romRamMode == true) {
        // return extRAM[romRamBankNumber][address & 0x1FFF];
        return extRAM[((address & 0x1FFF) | (romRamBankNumber << 13)) &
                      (ramSize - 1)];
      } else {
        // return extRAM[0][address & 0x1FFF];
        return extRAM[(address & 0x1FFF) & (ramSize - 1)];
      }
    } else {
      return 0x00;
    }
  } else {
    // TODO: actual external ram
    return 0;
  }
}

void MBC1::setBatteryLocation(string inBatteryPath) {
  battery = false;
  batteryPath = inBatteryPath;
  if (Cartridge::loadBatteryFile(extRAM, ramSize, batteryPath)) {
    battery = true; // Disable battery if load wasn't sucessful;
  }
}

void MBC1::saveBatteryData() {
  if (battery) {
    Cartridge::saveBatteryFile(extRAM, ramSize, batteryPath);
  }
}
