#ifndef CARTRIDGE_H
#define CARTRIDGE_H
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

typedef unsigned char BYTE;
typedef unsigned short WORD;

class Cartridge {
public:
  Cartridge();
  virtual ~Cartridge();
};

#endif