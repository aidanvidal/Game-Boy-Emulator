#ifndef MBC1_H
#define MBC1_H

#include "Cartridge.h"

class MBC1 : public Cartridge {
private:
    BYTE *rom;   // Pointer to ROM data
    BYTE *ram;   // Pointer to RAM data
    int romSize; // Size of the ROM
    int ramSize; // Size of the RAM
    int romBank; // Current ROM bank number
    int ramBank; // Current RAM bank number
    bool ramEnabled; // Flag to indicate if RAM is enabled
    bool bankingMode; // Banking mode (0 = simple, 1 = advanced)
    std::string batteryPath; // Path to the battery file (if there is one)
public:
    // Constructor
    MBC1(BYTE *romData, int romSize, int ramSize, const std::string &batteryPath = "");
    ~MBC1();

    // Read and write functions
    BYTE readData(WORD address) const override;       // Read data from memory
    void writeData(WORD address, BYTE data) override; // Write data to memory

    // Additional functions
    void saveBattery(); // Save battery data to file
    void loadBattery(); // Load battery data from file
};

#endif