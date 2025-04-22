#ifndef CARTRIDGE_H
#define CARTRIDGE_H
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

typedef unsigned char BYTE;
typedef unsigned short WORD;

// This is a base class for the cartridge
// It will be inherited by different MBC classes
// such as NoMBC, MBC1, MBC2, etc.
// Each MBC class will implement its own read and write functions
// to handle the specific memory mapping and banking logic
// for that MBC type
class Cartridge {
public:
  Cartridge();
  virtual ~Cartridge();
  virtual BYTE readData(WORD address) const = 0;       // Pure virtual function
  virtual void writeData(WORD address, BYTE data) = 0; // Pure virtual function
};

#endif