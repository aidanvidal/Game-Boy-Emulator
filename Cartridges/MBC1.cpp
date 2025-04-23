#include "MBC1.h"
#include "Cartridge.h"
#include <fstream>

MBC1::MBC1(BYTE *romData, int romSize, int ramSize,
           const std::string &batteryPath)
    : rom(romData), romSize(romSize), ramSize(ramSize),
      batteryPath(batteryPath) {
  // Constructor
  ram = new BYTE[ramSize]; // Allocate memory for RAM
  loadBattery();           // Load battery data from file if it exists
  romBank = 1;             // Set initial ROM bank to 1
  ramBank = 0;             // Set initial RAM bank to 0
  ramEnabled = false;      // Disable RAM by default
}

MBC1::~MBC1() {
  // Destructor
  saveBattery(); // Save battery data to file
  free(rom);     // Free the allocated memory for ROM data
  free(ram);     // Free the allocated memory for RAM data
}

void MBC1::writeData(WORD address, BYTE data) {
  // Handle Register writes

  // RAM Enable/Disable
  if (address >= 0x0000 && address <= 0x1FFF) {
    ramEnabled = (data & 0x0A) == 0x0A; // Enable RAM if data is 0x0A
  }

  // ROM Bank Number
  else if (address >= 0x2000 && address <= 0x3FFF) {
    // Mask to 5 bits and ensure bank 0 maps to bank 1
    BYTE bank = data & 0x1F; // Extract lower 5 bits
    if (bank == 0) {
      bank = 1; // Bank 0 behaves as bank 1
    }
    // Mask the bank number to the number of available banks
    int maxBanks = romSize / 0x4000; // Calculate the number of banks
    romBank = bank % maxBanks;       // Ensure bank number is valid
  }

  // RAM Bank Number or Upper bits of ROM Bank Number
  else if (address >= 0x4000 && address <= 0x5FFF) {
    // Mask to 2 bits for RAM bank number
    ramBank = data & 0x03; // Extract lower 2 bits
  }

  // Banking Mode Select
  else if (address >= 0x6000 && address <= 0x7FFF) {
    // Set banking mode (0 = simple, 1 = advanced)
    bankingMode = data & 0x01; // Extract the least significant bit
  }

  // RAM Data
  else if (address >= 0xA000 && address <= 0xBFFF) {
    if (ramEnabled && ramSize > 0) {
      // Check if RAM is enabled
      if (bankingMode) {
        // Advanced mode: use RAM bank number
        ram[((address & 0x1FFF) | (ramBank << 13)) & (ramSize - 1)] =
            data; // Write to RAM
      } else {
        // Simple mode: use fixed RAM bank 0
        ram[(address & 0x1FFF) & (ramSize - 1)] = data; // Write to RAM
      }
    }
  }
}

BYTE MBC1::readData(WORD address) const {
  // ROM Reads
  if (address >= 0x0000 && address <= 0x7FFF) {
    // Upper bank handling
    if (address & 0x4000) {
      // TODO: Check these (romSize - 1) and (ramSize - 1)
      // also check the romBank << 14
      if (bankingMode) {
        return rom[((address & 0x3FFF) | (romBank << 14)) & (romSize - 1)];
      } else {
        return rom[(address & 0x3FFF) & (romSize - 1)];
      }
    }
    return rom[(address & 0x3FFF)];
  }

  // RAM Reads
  else if (address >= 0xA000 && address <= 0xBFFF) {
    if (ramEnabled && ramSize > 0) {
      if (bankingMode) {
        return ram[((address & 0x1FFF) | (ramBank << 13)) & (ramSize - 1)];
      } else {
        return ram[(address & 0x1FFF) & (ramSize - 1)];
      }
    }
  }
  return 0xFF; // Default return value for invalid reads
}

void MBC1::loadBattery() {
  std::ifstream batteryFile(batteryPath, std::ios::in | std::ios::binary);
  if (!batteryFile.is_open()) {
    // Battery file does not exist, create a new one and fill RAM with zeros
    for (int i = 0; i < ramSize; i++) {
      ram[i] = 0;
    }
  } else {
    // Battery file exists
    // Check if the file size matches the RAM size
    batteryFile.seekg(0, std::ios::end);
    std::streamsize size = batteryFile.tellg();
    batteryFile.seekg(0, std::ios::beg);
    if (size != ramSize) {
      std::cerr << "Error: Battery file size does not match RAM size."
                << std::endl;
      exit(1);
    }
    // Read the battery data into RAM
    batteryFile.read(reinterpret_cast<char *>(ram), ramSize);
    if (batteryFile.gcount() != ramSize) {
      std::cerr << "Error: Failed to read battery data." << std::endl;
      exit(1);
    }
    batteryFile.close(); // Close the battery file
  }
}

void MBC1::saveBattery() {
  // Save battery data to file
  std::ofstream batteryFile(batteryPath, std::ios::out | std::ios::binary);
  if (!batteryFile.is_open()) {
    std::cerr << "Error: Could not open battery file for writing." << std::endl;
    return;
  }
  batteryFile.write(reinterpret_cast<char *>(ram), ramSize);
  if (!batteryFile) {
    std::cerr << "Error: Failed to write battery data." << std::endl;
  }
  batteryFile.close(); // Close the battery file
  std::cout << "Battery data saved to " << batteryPath << std::endl;
}