#ifndef MBC5_H
#define MBC5_H
#include "Cartridge.h"

class MBC5 :
	public Cartridge
{
public:
	MBC5(uint8_t* romData, unsigned int romSize, unsigned int ramSize);
	~MBC5();

	void writeData(uint16_t address, uint8_t data) override;
	uint8_t readData(uint16_t address) override;

	// Battery functions
	void setBatteryLocation(string batteryPath) override;
	void saveBatteryData() override;

private:
	uint8_t* rom;
	unsigned int romSize;
	unsigned int ramSize;
	bool battery = false;
	string batteryPath = "";
	bool ramNewData = false;

	bool ramEnable = false;
	uint16_t romBankNumber = 0;
	uint8_t ramBankNumber = 0;

	uint8_t* ram;
};

#endif // MBC5_H