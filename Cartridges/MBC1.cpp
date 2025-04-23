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
  bankingMode = false;     // Set banking mode to simple by default
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
    ramEnabled = (data & 0x0F) == 0x0A; // Enable RAM if data is 0x0A
  }

  // ROM Bank Number (5-bit register)
  else if (address >= 0x2000 && address <= 0x3FFF) {
    // Extract lower 5 bits
    romBank = data & 0x1F;
    
    // The bank 0 is treated as bank 1 in the 0x4000-0x7FFF region
    if ((romBank & 0x1F) == 0) {
      romBank = 1;
    }
  }

  // RAM Bank Number or Upper bits of ROM Bank Number (2-bit register)
  else if (address >= 0x4000 && address <= 0x5FFF) {
    // Store the 2-bit value (0-3)
    ramBank = data & 0x03;
  }

  // Banking Mode Select
  else if (address >= 0x6000 && address <= 0x7FFF) {
    // Set banking mode (0 = simple, 1 = advanced)
    bankingMode = data & 0x01;
  }

  // RAM Data
  else if (address >= 0xA000 && address <= 0xBFFF && ramEnabled && ramSize > 0) {
    // Only write if RAM is enabled and exists
    int ramAddress;
    
    if (bankingMode && ramSize > 0x2000) {
    // Advanced mode: use RAM bank number (only matters for 8KB RAM)
      ramAddress = (address & 0x1FFF) | (ramBank << 13);
    } else {
      // Simple mode: always use fixed RAM bank 0
      ramAddress = address & 0x1FFF;
    }
    
    // Make sure we don't go beyond the RAM bounds
    ramAddress &= (ramSize - 1);
    ram[ramAddress] = data;
  }
}

BYTE MBC1::readData(WORD address) const {
  // ROM Bank 0 (0000-3FFF)
  if (address <= 0x3FFF) {
    if (bankingMode && romSize > 0x80000) {  // Only for ROMs > 512KB
      // In advanced mode, upper bits from ramBank apply to this region too
      int effectiveBank = (ramBank << 5) & (romSize / 0x4000 - 1);
      return rom[(address & 0x3FFF) | (effectiveBank << 14)];
    } else {
      // Fixed to bank 0 in simple mode
      return rom[address & 0x3FFF];
    }
  }
  
  // ROM Bank 1-127 (4000-7FFF)
  else if (address >= 0x4000 && address <= 0x7FFF) {
    // Calculate effective bank number
    int effectiveBank = romBank;
    
    // For large ROMs (>512KB), apply upper bits from ramBank
    if (romSize > 0x80000) {  // 512KB
      effectiveBank |= (ramBank << 5);
    }
    
    // Ensure bank number is within ROM size bounds
    effectiveBank &= (romSize / 0x4000 - 1);
    
    // Map to ROM address
    return rom[(address & 0x3FFF) | (effectiveBank << 14)];
  }

  // RAM (A000-BFFF)
  else if (address >= 0xA000 && address <= 0xBFFF) {
    if (ramEnabled && ramSize > 0) {
      int ramAddress;
      
      if (bankingMode && ramSize > 0x2000) {  // Only relevant for 8KB RAM
        // Advanced mode: use RAM bank number
        ramAddress = (address & 0x1FFF) | (ramBank << 13);
      } else {
        // Simple mode: always use bank 0
        ramAddress = address & 0x1FFF;
      }
      
      // Make sure we don't go beyond the RAM bounds
      ramAddress &= (ramSize - 1);
      return ram[ramAddress];
    }
    return 0xFF;  // Default return for disabled RAM
  }
  
  return 0xFF;  // Default return value for invalid reads
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