#include "NOMBC.h"



NOMBC::NOMBC(uint8_t* romData, int romSize): romData(romData), romSize(romSize)
{
	if (romSize > 0x8000) {
		cout << "ROM too large for NOMBC. We'll mask out but it will likely fail";
	}
}


NOMBC::~NOMBC()
{
	free(romData);
}

void NOMBC::writeData(uint16_t address, uint8_t data)
{
	// NOMBC has no registers
}

uint8_t NOMBC::readData(uint16_t address)
{
	return romData[address & 0x7FFF];
}

void NOMBC::setBatteryLocation(string inBatteryPath)
{
}

void NOMBC::saveBatteryData()
{
}
