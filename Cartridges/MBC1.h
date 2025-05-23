#ifndef MBC1_H
#define MBC1_H

#include <fstream>
#include <iostream>
#include <stdint.h>
#include <stdio.h>

#include "Cartridge.h"

using namespace std;

class MBC1 : public Cartridge {
public:
  // Constructor
  MBC1(uint8_t *romData, unsigned int romSize, unsigned int ramSize);
  ~MBC1();
  // Public functions
  void writeData(uint16_t address, uint8_t data) override;
  uint8_t readData(uint16_t address) override;

  // Battery functions
  void setBatteryLocation(string batteryPath) override;
  void saveBatteryData() override;

private:

  // Private variables and classes
  unsigned int romSize;
  unsigned int ramSize;
  string batteryPath = "";
  bool battery = false;
  bool ramNewData = false;
  uint8_t *ram;
  ifstream romFileStream;
  // MBC1 registers
  bool ramEnable;
  int romBankNumber;
  int romRamBankNumber = 0;
  bool romRamMode;
  uint8_t *rom;
};

#endif // MBC1_H