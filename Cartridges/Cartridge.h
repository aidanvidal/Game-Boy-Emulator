#ifndef CARTRIDGE_H
#define CARTRIDGE_H
#include <stdint.h>
#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <fstream>

using namespace std;


class Cartridge
{
public:
	Cartridge();
	virtual ~Cartridge();

	virtual void writeData(uint16_t address, uint8_t data) = 0;
	virtual uint8_t readData(uint16_t address) = 0;

	virtual void setBatteryLocation(string batteryPath) = 0;
	virtual void saveBatteryData() = 0;

	static bool loadBatteryFile(uint8_t* extRAm, unsigned int ramSize, string inBatteryPath);
	static void saveBatteryFile(uint8_t* extRAM, unsigned int ramSize, string inBatteryPath);
};

#endif // CARTRIDGE_H