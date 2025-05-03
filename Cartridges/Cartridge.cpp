#include "Cartridge.h"


Cartridge::Cartridge()
{
}


Cartridge::~Cartridge()
{
}


bool Cartridge::loadBatteryFile(uint8_t * extRAM, unsigned int ramSize, string inBatteryPath)
{
	bool success = false;	// Bool to return if file loaded fine
	// Attempt to load in a battery file
	ifstream batteryFileStream(inBatteryPath, ios::in | ios::binary | ios::ate);
	if (batteryFileStream.is_open()) {
		if (ramSize == (int)batteryFileStream.tellg()) {
			batteryFileStream.seekg(0, ios::beg);
			for (unsigned int i = 0; i < ramSize; i++) {
				char oneByte;
				batteryFileStream.read((&oneByte), 1);
				extRAM[i] = (uint8_t)oneByte;
			}
			success = true;
		}
		else {
			cout << "\nError: Opened save file doesn't match defined RAM size. Battery will not be saved";	// Probaly handle this better
			for (unsigned int i = 0; i < ramSize; i++) {
				extRAM[i] = 0;
			}
			success = false;
		}
		batteryFileStream.close();
	}
	else {
		// Load in 0 data if save couldn't be open, basically making a new file.
		// Doesn't really account for cases where it couldn't be opened but still exists.
		for (unsigned int i = 0; i < ramSize; i++) {
			extRAM[i] = 0;
		}
		success = true;	// This is fine.
	}
	return success;
}

void Cartridge::saveBatteryFile(uint8_t * extRAM, unsigned int ramSize, string inBatteryPath)
{
	// Create a file stream
	ofstream batteryFileStream(inBatteryPath, ios::out | ios::binary);
	if (batteryFileStream.is_open()) {
		for (unsigned int i = 0; i < ramSize; i++) {
			char oneByte = (char)extRAM[i];
			batteryFileStream.write(&oneByte, 1);
		}
		batteryFileStream.close();
		//cout << "Write (should be?) successful\n";
	}
	else {
		cout << "\nError: Could not open save file stream. File will not be saved\n";
	}
}
