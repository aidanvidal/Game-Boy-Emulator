#ifndef NOMBC_H
#define NOMBC_H
#include "Cartridge.h"
class NOMBC : public Cartridge {
public:
  NOMBC(uint8_t *romData, int romSize);
  ~NOMBC();
  void writeData(uint16_t address, uint8_t data) override;
  uint8_t readData(uint16_t address) override;

  // Battery functions
  void setBatteryLocation(string batteryPath) override;
  void saveBatteryData() override;

private:
  uint8_t *romData;
  unsigned int romSize;
};

#endif // NOMBC_H