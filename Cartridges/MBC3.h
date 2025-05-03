#ifndef MBC3_H
#define MBC3_H
#include "Cartridge.h"
#include <time.h>

class MBC3 :
	public Cartridge
{
public:
	MBC3(uint8_t* romData, unsigned int romSize, unsigned int ramSize, bool timerPresent);
	~MBC3();
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

	//uint8_t extRAM[4][0xBFFF - 0xA000 + 1];	// Somewhat temporary
	uint8_t* ram;
	bool ramEnable = false;
	int romBankNumber = 1;
	uint8_t RAMRTCselect = 0;

	// RTC stuff
	bool RTC = false;
	// Real time (Updated at certain points).
	uint8_t realSecs = 0;
	uint8_t realMins = 0;
	uint8_t realHours = 0;
	uint8_t realDays = 0;
	uint8_t realDaysHi = 0;

	// Latched time (what's read by the game, latched in by a trigger)
	uint8_t latchSecs = 0;
	uint8_t latchMins = 0;
	uint8_t latchHours = 0;
	uint8_t latchDays = 0;
	uint8_t latchDaysHi = 0;

	// The unix/epoch time since the last timer update.
	time_t currentTime = 0;

	// Checks the last write to the latch register (latch happens from low to high)
	bool latch = false;

	// Update timer function (calculates difference in seconds, increments the relevant counters).
	void updateTimer();
	// Latches the time
	void latchTimer();

};

#endif // MBC3_H