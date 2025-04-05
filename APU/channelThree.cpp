#include "channelThree.h"

ChannelThree::ChannelThree() {
  // Initialize the registers with default values
  NR30 = 0x7F;
  NR31 = 0xFF;
  NR32 = 0x9F;
  NR33 = 0xBF;
  NR34 = 0x00;
  channelEnabled = false;
  state.dacEnabled = false;
  state.sampleSelection = 1;
  state.sampleBuffer = 0.0f;
}

// Register access functions
BYTE ChannelThree::getNR30() const { return NR30; }
BYTE ChannelThree::getNR31() const { return NR31; }
BYTE ChannelThree::getNR32() const { return NR32; }
BYTE ChannelThree::getNR33() const { return NR33; }
BYTE ChannelThree::getNR34() const { return NR34; }
void ChannelThree::writeNR30(BYTE value) { NR30 = value; }
void ChannelThree::writeNR31(BYTE value) { NR31 = value; }
void ChannelThree::writeNR32(BYTE value) { NR32 = value; }
void ChannelThree::writeNR33(BYTE value) { NR33 = value; }
void ChannelThree::writeNR34(BYTE value) {
  NR34 = value;
  // Check if the trigger bit is set
  if ((value & 0x80) && state.dacEnabled) {
    // Trigger channel if the trigger bit is set
    trigger();
  }
}

// NR30 Helper Functions
// Set the DAC enable (true to enable, false to disable)
void ChannelThree::setDACEnable(bool enable) {
  if (enable) {
    NR30 |= 0x80;
  } else {
    NR30 &= 0x7F;
  }
}
// Get the DAC enable (true if enabled, false if disabled)
bool ChannelThree::getDACEnable() const { return (NR30 & 0x80) != 0; }

// NR31 Helper Functions
// Set the initial length timer (0-255)
void ChannelThree::setInitialLengthTimer(BYTE timer) { NR31 = timer; }
// Get the initial length timer (0-255)
BYTE ChannelThree::getInitialLengthTimer() const { return NR31; }

// NR32 Helper Functions

// Set the output level (0-3)
// 0: mute
// 1: 100% volume
// 2: 50% volume
// 3: 25% volume
void ChannelThree::setOutputLevel(BYTE level) {
  NR32 &= 0x9F;
  NR32 |= (level << 5);
}
// Get the output level (0-3)
BYTE ChannelThree::getOutputLevel() const { return (NR32 >> 5) & 0x03; }

// NR33 Helper Functions
// Set the period low (0-255)
void ChannelThree::setPeriodLow(BYTE period) { NR33 = period; }
// Get the period low (0-255)
BYTE ChannelThree::getPeriodLow() const { return NR33; }

// NR34 Helper Functions
// Set the period high (0-7)
void ChannelThree::setPeriodHigh(BYTE period) {
  NR34 &= 0xF8;
  NR34 |= (period & 0x07);
}
// Get the period high (0-7)
BYTE ChannelThree::getPeriodHigh() const { return NR34 & 0x07; }

// Set the period (0-2047)
void ChannelThree::setPeriod(WORD period) {
  NR33 = period & 0xFF;
  NR34 &= 0xF8;
  NR34 |= (period >> 8) & 0x07;
}
// Get the period (0-2047)
WORD ChannelThree::getPeriod() const { return ((NR34 & 0x07) << 8) | NR33; }

// Set the length enable (true to enable, false to disable)
void ChannelThree::setLengthEnable(bool enable) {
  if (enable) {
    NR34 |= 0x40;
  } else {
    NR34 &= 0xBF;
  }
}
// Get the length enable (true if enabled, false if disabled)
bool ChannelThree::getLengthEnable() const { return (NR34 & 0x40) != 0; }

// Set the trigger (true to trigger, false to disable)
void ChannelThree::setTrigger(bool trigger) {
  if (trigger) {
    NR34 |= 0x80;
    // Trigger channel
    this->trigger();
  } else {
    NR34 &= 0x7F;
  }
}
// Get the trigger (true if triggered, false if not)
bool ChannelThree::getTrigger() const { return (NR34 & 0x80) != 0; }

// Channel operations
void ChannelThree::trigger() {
  // Check if DAC is enabled
  if ((NR30 & 0x80) != 0) {
    state.dacEnabled = true;
  } else {
    state.dacEnabled = false;
    setEnabled(false); // Disable channel
    return;            // If DAC is disabled, do not trigger
  }
  // Enable channel
  setEnabled(true);
  state.lengthTimer = getInitialLengthTimer();
  switch (getOutputLevel()) {
  case 0:
    state.volume = 0;
    break;
  case 1:
    state.volume = 15;
    break;
  case 2:
    state.volume = 7;
    break;
  case 3:
    state.volume = 3;
    break;
  default:
    state.volume = 0; // Default to mute
    break;
  }
  state.sampleSelection = 1;                    // Reset sample selection
  state.sampleTimer = (2048 - getPeriod()) * 4; // Reset sample timer
}

// Set the enabled state of the channel
void ChannelThree::setEnabled(bool enable) {
  channelEnabled = enable;
  if (enable) {
    NR34 |= 0x80; // Set the trigger bit
  } else {
    NR34 &= 0x7F; // Clear the trigger bit
  }
}
// Get the enabled state of the channel
bool ChannelThree::isEnabled() const { return channelEnabled; }

// Reset the channel
void ChannelThree::reset() {
  NR30 = 0x7F;
  NR31 = 0xFF;
  NR32 = 0x9F;
  NR33 = 0xBF;
  NR34 = 0x00;
  channelEnabled = false;
  state.dacEnabled = false;
  state.sampleSelection = 1;
  state.sampleBuffer = 0.0f;
}

// Get the sample from Channel Three
float ChannelThree::getSample() {
  if (!state.dacEnabled) {
    return 0.0f; // If DAC is disabled, return 0
  }
  if (!isEnabled()) {
    return 1.0f; // Disabled channel outputs "analog 1"
  }

  // Convert the shifted digital sample to an analog value (-1.0 to 1.0)
  int analogSample = state.sampleBuffer;
  if (state.volume == 0) {
    return 0.0f; // If volume is 0, return 0
  } else if (state.volume == 7) {
    analogSample >>= 1; // If volume is 7, shift right by 1
  } else if (state.volume == 3) {
    analogSample >>= 2; // If volume is 3, shift right by 2
  }

  // Convert to a float value between -1.0 and 1.0
  float sample = (analogSample / 15.0f) * 2.0f - 1.0f;
  return sample; // Return the sample
}

// Update length timer
void ChannelThree::updateLengthTimer() {
  if (state.lengthTimer < 256) {
    state.lengthTimer++;
    // If length timer reaches zero, turn off the channel
    if (state.lengthTimer == 256) {
      setEnabled(false); // Disable channel
      std::cout << "Channel Three Disabled due to length timer." << std::endl;
    }
  }
}

// Wave Pattern RAM access
// Set the wave pattern RAM at the specified index and nibble (upper or lower)
void ChannelThree::setNibbleWavePatternRAM(int index, BYTE value, bool upper) {
  if (upper) {
    wavePatternRAM[index] = (wavePatternRAM[index] & 0x0F) | (value << 4);
  } else {
    wavePatternRAM[index] = (wavePatternRAM[index] & 0xF0) | (value & 0x0F);
  }
}
// Get the wave pattern RAM at the specified index and nibble (upper or lower)
BYTE ChannelThree::getNibbleWavePatternRAM(int index, bool upper) const {
  if (upper) {
    return (wavePatternRAM[index] >> 4) & 0x0F;
  } else {
    return wavePatternRAM[index] & 0x0F;
  }
}
// Set the wave pattern RAM at the specified index
void ChannelThree::setWavePatternRAM(int index, BYTE value) {
  wavePatternRAM[index] = value;
}
// Get the wave pattern RAM at the specified index
BYTE ChannelThree::getWavePatternRAM(int index) const {
  return wavePatternRAM[index];
}
// Update the sample timer
void ChannelThree::updateSampleTimer(int cycles) {
  if (!isEnabled()) {
    return; // If channel is disabled, do nothing
  }

  // Update the sample timer
  state.sampleTimer -= cycles;
  if (state.sampleTimer <= 0) {
    state.sampleTimer = (2048 - getPeriod()) * 4; // Reset sample timer
    state.sampleSelection =
        (state.sampleSelection + 1) % 32; // Update sample selection
    // Read the new sample and store it in the buffer
    int sample = getNibbleWavePatternRAM(state.sampleSelection / 2,
                                         (state.sampleSelection % 2 == 0));
    state.sampleBuffer = sample;
  }
}