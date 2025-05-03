#include "MBC3.h"

MBC3::MBC3(uint8_t *romData, unsigned int romSize, unsigned int ramSize,
           bool timerPresent)
    : rom(romData), romSize(romSize), ramSize(ramSize), hasRTC(timerPresent) {
  // Allocate extra memory for RTC data if needed
  ram = new uint8_t[hasRTC ? ramSize + 48 : ramSize];
}

MBC3::~MBC3() {
  if (battery && ramNewData) {
    saveBatteryData();
  }
  free(rom);
  free(ram);
}

void MBC3::writeData(uint16_t address, uint8_t data) {
  if (address <= 0x1FFF) {
    // RAM Enable/Disable (0x0000-0x1FFF)
    bool newRamEnable = (data & 0xF) == 0xA;
    
    // If RAM is being disabled and battery-backed RAM has changes, save it
    if (battery && ramEnable && !newRamEnable && ramNewData) {
      saveBatteryData();
      ramNewData = false;
    }
    ramEnable = newRamEnable;
  } 
  else if (address <= 0x3FFF) {
    // ROM Bank Number (0x2000-0x3FFF)
    romBankNumber = data & 0x7F;
    if (romBankNumber == 0) {
      romBankNumber = 1; // Values of 0 are treated as 1
    }
  } 
  else if (address <= 0x5FFF) {
    // RAM Bank Number or RTC Register Select (0x4000-0x5FFF)
    RAMRTCSelect = data % 0xD;
  } 
  else if (address <= 0x7FFF) {
    // Latch Clock Data (0x6000-0x7FFF)
    bool newLatch = (data & 0x1) == 0x1;
    if (!latch && newLatch && hasRTC) {
      latchTimer();
    }
    latch = newLatch;
  } 
  else if (address >= 0xA000 && address <= 0xBFFF) {
    // External RAM or RTC Register access (0xA000-0xBFFF)
    if (ramEnable) {
      if (RAMRTCSelect < 4 && ramSize > 0) {
        // RAM access
        uint32_t ramAddress = ((address & 0x1FFF) | (RAMRTCSelect << 13)) & (ramSize - 1);
        ram[ramAddress] = data;
        ramNewData = true;
      } 
      else if (hasRTC && RAMRTCSelect >= 0x8 && RAMRTCSelect <= 0xC) {
        // RTC register access
        updateTimer(); // Keep timer up-to-date before writing
        
        switch (RAMRTCSelect) {
          case 0x8: realSecs = data; break;    // Seconds
          case 0x9: realMins = data; break;    // Minutes
          case 0xA: realHours = data; break;   // Hours
          case 0xB: realDays = data; break;    // Days (lower)
          case 0xC: realDaysHi = data; break;  // Days (upper), halt, Day carry
        }
      }
    }
  }
}

uint8_t MBC3::readData(uint16_t address) {
  if (address <= 0x7FFF) {
    // ROM Access (0x0000-0x7FFF)
    uint32_t romAddress = address & 0x3FFF;
    
    if (address & 0x4000) {
      // Upper bank (0x4000-0x7FFF)
      romAddress |= romBankNumber << 14;
    }
    
    return rom[romAddress];
  }
  else if (address >= 0xA000 && address <= 0xBFFF) {
    // External RAM or RTC Register access (0xA000-0xBFFF)
    if (ramEnable) {
      if (RAMRTCSelect < 4 && ramSize > 0) {
        // RAM access
        uint32_t ramAddress = ((address & 0x1FFF) | (RAMRTCSelect << 13)) & (ramSize - 1);
        return ram[ramAddress];
      } 
      else if (hasRTC && RAMRTCSelect >= 0x8 && RAMRTCSelect <= 0xC) {
        // RTC register access - return latched values
        switch (RAMRTCSelect) {
          case 0x8: return latchSecs;    // Seconds
          case 0x9: return latchMins;    // Minutes
          case 0xA: return latchHours;   // Hours
          case 0xB: return latchDays;    // Days (lower)
          case 0xC: return latchDaysHi;  // Days (upper), halt, Day carry
        }
      }
    }
  }
  return 0xFF; // Default value for unmapped/disabled memory
}

void MBC3::setBatteryLocation(string inBatteryPath) {
  battery = false;
  batteryPath = inBatteryPath;
  
  // Standard RAM-only battery
  if (!hasRTC && Cartridge::loadBatteryFile(ram, ramSize, batteryPath)) {
    battery = true;
  }
  // RTC battery handling
  else if (hasRTC && Cartridge::loadBatteryFile(ram, ramSize + 48, batteryPath)) {
    battery = true;
    
    // Extract RTC values from the end of ram array
    unsigned int offset = ramSize;
    
    // "Real" time values
    realSecs = ram[offset];
    realMins = ram[offset + 4];
    realHours = ram[offset + 8];
    realDays = ram[offset + 12];
    realDaysHi = ram[offset + 16];
    
    // Latched time values
    latchSecs = ram[offset + 20];
    latchMins = ram[offset + 24];
    latchHours = ram[offset + 28];
    latchDays = ram[offset + 32];
    latchDaysHi = ram[offset + 36];
    
    // Extract saved system time
    currentTime = 0;
    for (int i = 0; i < 8; i++) {
      currentTime |= (time_t)(ram[offset + 40 + i]) << (8 * i);
    }
    
    // Use current system time if saved time is invalid
    if (currentTime <= 0) {
      currentTime = time(nullptr);
    }
    
    // Update the timer now
    updateTimer();
  } 
  else if (hasRTC) {
    // Failed to load RTC data
    currentTime = time(nullptr);
  }
}

void MBC3::saveBatteryData() {
  if (!battery) return;
  
  if (!hasRTC) {
    // Standard RAM-only save
    Cartridge::saveBatteryFile(ram, ramSize, batteryPath);
  } 
  else {
    // Update RTC before saving
    updateTimer();
    
    // Store RTC values in the extended section of ram
    unsigned int offset = ramSize;
    
    // Real time
    ram[offset] = realSecs;
    ram[offset + 4] = realMins;
    ram[offset + 8] = realHours;
    ram[offset + 12] = realDays;
    ram[offset + 16] = realDaysHi;
    
    // Latched time
    ram[offset + 20] = latchSecs;
    ram[offset + 24] = latchMins;
    ram[offset + 28] = latchHours;
    ram[offset + 32] = latchDays;
    ram[offset + 36] = latchDaysHi;
    
    // System time
    for (int i = 0; i < 8; i++) {
      ram[offset + 40 + i] = (uint8_t)(currentTime >> (8 * i));
    }
    
    // Save the complete data
    Cartridge::saveBatteryFile(ram, ramSize + 48, batteryPath);
  }
  
  ramNewData = false;
}

void MBC3::updateTimer() {
  // Get the current system time
  time_t newTime = time(nullptr);
  
  // Calculate time difference in seconds
  unsigned int timeDiff = 0;
  
  // Skip update if: 
  // 1. Time went backward (system clock changed)
  // 2. Timer halt bit is set
  if (newTime > currentTime && (realDaysHi & 0x40) != 0x40) {
    timeDiff = (unsigned int)(newTime - currentTime);
    currentTime = newTime;
  } else {
    currentTime = newTime;
    return;
  }
  
  // No time has passed
  if (timeDiff == 0) return;

  // Update seconds
  unsigned int newSeconds = realSecs + timeDiff;
  realSecs = newSeconds % 60;
  
  // Update minutes
  unsigned int newMins = realMins + (newSeconds / 60);
  if (newMins == realMins) return;
  realMins = newMins % 60;
  
  // Update hours
  unsigned int newHours = realHours + (newMins / 60);
  if (newHours == realHours) return;
  realHours = newHours % 24;
  
  // Update days (including high bit)
  unsigned int fullDays = ((realDaysHi & 0x1) << 8) | realDays;
  unsigned int newDays = fullDays + (newHours / 24);
  if (newDays == fullDays) return;
  
  // Update days counter
  realDays = newDays & 0xFF;                // Low 8-bits
  realDaysHi = (realDaysHi & 0xFE) | ((newDays >> 8) & 0x1); // High bit
  
  // Set day counter overflow flag if > 511 days
  if (newDays > 511) {
    realDaysHi |= 0x80;
  }
}

void MBC3::latchTimer() {
  // Update and then copy the current time to latched time
  updateTimer();
  
  latchSecs = realSecs;
  latchMins = realMins;
  latchHours = realHours;
  latchDays = realDays;
  latchDaysHi = realDaysHi;
}
