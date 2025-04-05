#include "channelTwo.h"

ChannelTwo::ChannelTwo() {
  // Initialize the registers with default values from APU constructor
  NR21 = 0x3F;
  NR22 = 0x00;
  NR23 = 0x00;
  NR24 = 0xBF;
  channelEnabled = false;
  state.dacEnabled = false;
}

// Register access functions
BYTE ChannelTwo::getNR21() const { return NR21; }

BYTE ChannelTwo::getNR22() const { return NR22; }

BYTE ChannelTwo::getNR23() const { return NR23; }

BYTE ChannelTwo::getNR24() const { return NR24; }

void ChannelTwo::writeNR21(BYTE value) { NR21 = value; }

// Writes to this register require retriggering
void ChannelTwo::writeNR22(BYTE value) { NR22 = value; }

void ChannelTwo::writeNR23(BYTE value) { NR23 = value; }

void ChannelTwo::writeNR24(BYTE value) {
  NR24 = value;
  // Check if the trigger bit is set
  if ((value & 0x80) && state.dacEnabled) {
    // Trigger channel if the trigger bit is set
    trigger();
  }
}

// NR21 Helper Functions

// Set the wave duty (0-3)
void ChannelTwo::setWaveDuty(BYTE duty) {
  NR21 &= 0x3F;
  NR21 |= (duty << 6);
}

// Set the initial length timer (0-63)
void ChannelTwo::setInitialLengthTimer(BYTE timer) {
  NR21 &= 0xC0;
  NR21 |= timer;
}

// Get the wave duty (0-3)
BYTE ChannelTwo::getWaveDuty() const { return NR21 >> 6; }

// Get the initial length timer (0-63)
BYTE ChannelTwo::getInitialLengthTimer() const { return NR21 & 0x3F; }

// NR22 Helper Functions
// Writes to all these require retriggering

// Set the initial volume (0-15)
void ChannelTwo::setInitialVolume(BYTE volume) {
  NR22 &= 0x0F;
  NR22 |= (volume << 4);
}

// Set the envelope direction (true for increase, false for decrease)
void ChannelTwo::setEnvelopeDirection(bool direction) {
  if (direction) {
    NR22 |= 0x08;
  } else {
    NR22 &= 0xF7;
  }
}

// Set the envelope period (0-7)
void ChannelTwo::setEnvelopePeriod(BYTE pace) {
  NR22 &= 0xF8;
  NR22 |= pace;
}

// Get the initial volume (0-15)
BYTE ChannelTwo::getInitialVolume() const { return NR22 >> 4; }

// Get the envelope direction (true for increase, false for decrease)
bool ChannelTwo::getEnvelopeDirection() const { return NR22 & 0x08; }

// Get the envelope period (0-7)
BYTE ChannelTwo::getEnvelopePeriod() const { return NR22 & 0x07; }

// Channel Two Period

// Set the period (0-2047)
void ChannelTwo::setPeriod(WORD period) {
  NR23 = period & 0xFF;
  NR24 &= 0xF8;
  NR24 |= (period >> 8) & 0x07;
}

// Get the period (0-2047)
WORD ChannelTwo::getPeriod() const { return ((NR24 & 0x07) << 8) | NR23; }

// NR24 Helper Functions

// Set the length enable (true to enable, false to disable)
void ChannelTwo::setLengthEnable(bool enable) {
  if (enable) {
    NR24 |= 0x40;
  } else {
    NR24 &= 0xBF;
  }
}

// Set the trigger bit (true to trigger, false to disable)
void ChannelTwo::setTrigger(bool trigger) {
  if (trigger) {
    NR24 |= 0x80;
    // Retrigger channel
    this->trigger();
  } else {
    NR24 &= 0x7F;
  }
}

// Get the length enable (true if enabled, false if disabled)
bool ChannelTwo::getLengthEnable() const { return NR24 & 0x40; }

// Get the trigger bit (true if triggered, false if not)
bool ChannelTwo::getTrigger() const { return NR24 & 0x80; }

// Channel operations

// Trigger the channel
void ChannelTwo::trigger() {
  // Check if DAC is enabled
  if ((NR22 & 0xF8) != 0) {
    state.dacEnabled = true;
  } else {
    state.dacEnabled = false;
    setEnabled(false); // Disable channel
    return;            // If DAC is disabled, do not trigger
  }

  // Enable channel
  setEnabled(true);
  state.lengthTimer = getInitialLengthTimer();
  state.envelopePeriod = getEnvelopePeriod();
  state.envelopeTimer = state.envelopePeriod;
  state.envelopeDirection = getEnvelopeDirection();
  state.volume = getInitialVolume();
  state.timer = (2048 - getPeriod()) * 4;
  state.sequencePointer = 0;
}

// Reset the channel
void ChannelTwo::reset() {
  NR21 = 0x3F;
  NR22 = 0x00;
  NR23 = 0x00;
  NR24 = 0xBF;
  channelEnabled = false;
}

// Get the waveform sample in analog format
// Returns a value between -1.0 and 1.0
// 0.0 represents the middle of the waveform
float ChannelTwo::getSample() {
  if (!state.dacEnabled) {
    return 7.0f; // If DAC is disabled, output 0
  }

  if (!isEnabled()) {
    return 15.0f; // Disabled channel outputs "analog 1"
  }

  // Use duty table for waveform lookup
  static const int dutyTable[4][8] = {
      {0, 0, 0, 0, 1, 1, 1, 1}, // 12.5%
      {0, 0, 0, 1, 1, 1, 1, 1}, // 25%
      {0, 0, 1, 1, 1, 1, 0, 0}, // 50%
      {1, 1, 1, 1, 0, 0, 0, 0}  // 75%
  };
  int dutyType = getWaveDuty();
  int duty = dutyTable[dutyType][state.sequencePointer];
  return duty ? state.volume : 0; // Return volume if duty is high
}

// Update the sequence timer
// This function is called every CPU cycle
void ChannelTwo::updateSequenceTimer(int cycles) {
  if (!isEnabled()) {
    return; // If channel is disabled, do nothing
  }

  // Update the sequence timer
  state.timer -= cycles;
  if (state.timer <= 0) {
    state.timer = (2048 - getPeriod()) * 4; // Reset timer
    state.sequencePointer = (state.sequencePointer + 1) % 8;
  }
}

// Update the length timer
// This function is called 256Hz (every 2 APU steps)
void ChannelTwo::updateLengthTimer() {
  if (state.lengthTimer < 64) {
    state.lengthTimer++;
    // If length timer reaches zero, turn off the channel
    if (state.lengthTimer == 64) {
      setEnabled(false); // Disable channel
      std::cout << "Channel Two Disabled due to length timer." << std::endl;
    }
  }
}

// Update the envelope timer
// This function is called 64Hz (step 7 of APU step)
// It updates the volume based on the envelope settings
void ChannelTwo::updateEnvelope() {
  if (state.envelopePeriod == 0) {
    return; // Envelope disabled
  }

  state.envelopeTimer--;
  if (state.envelopeTimer <= 0) {
    state.envelopeTimer = state.envelopePeriod;
    if (state.envelopeDirection) {
      if (state.volume < 15) {
        state.volume++;
      }
    } else {
      if (state.volume > 0) {
        state.volume--;
      }
    }
  }
}

// Status functions
bool ChannelTwo::isEnabled() const { return channelEnabled; }

void ChannelTwo::setEnabled(bool enable) { channelEnabled = enable; }

bool ChannelTwo::isDacEnabled() const { return state.dacEnabled; }