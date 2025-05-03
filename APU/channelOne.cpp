#include "channelOne.h"
#include <cmath>
#include <iostream>

ChannelOne::ChannelOne() {
  // Initialize the registers with default values
  NR10 = 0x80;
  NR11 = 0xBF;
  NR12 = 0xF3;
  NR13 = 0x00;
  NR14 = 0xBF;
  channelEnabled = false;
  state.dacEnabled = false;
}

// Register access functions
BYTE ChannelOne::getNR10() const { return NR10; }

BYTE ChannelOne::getNR11() const { return NR11; }

BYTE ChannelOne::getNR12() const { return NR12; }

BYTE ChannelOne::getNR13() const { return NR13; }

BYTE ChannelOne::getNR14() const { return NR14; }

void ChannelOne::writeNR10(BYTE value) { NR10 = value; }

void ChannelOne::writeNR11(BYTE value) { NR11 = value; }

void ChannelOne::writeNR12(BYTE value) { NR12 = value; }

void ChannelOne::writeNR13(BYTE value) { NR13 = value; }

void ChannelOne::writeNR14(BYTE value) {
  NR14 = value;
  if ((value & 0x80)) {
    trigger();
  }
}

// NR11 Helper Functions
void ChannelOne::setWaveDuty(BYTE duty) {
  NR11 &= 0x3F;
  NR11 |= (duty << 6);
}

void ChannelOne::setInitialLengthTimer(BYTE timer) {
  NR11 &= 0xC0;
  NR11 |= timer;
}

BYTE ChannelOne::getWaveDuty() const { return NR11 >> 6; }

BYTE ChannelOne::getInitialLengthTimer() const { return NR11 & 0x3F; }

// NR12 Helper Functions
void ChannelOne::setInitialVolume(BYTE volume) {
  NR12 &= 0x0F;
  NR12 |= (volume << 4);
}

void ChannelOne::setEnvelopeDirection(bool direction) {
  if (direction) {
    NR12 |= 0x08;
  } else {
    NR12 &= 0xF7;
  }
}

void ChannelOne::setEnvelopePeriod(BYTE pace) {
  NR12 &= 0xF8;
  NR12 |= pace;
}

BYTE ChannelOne::getInitialVolume() const { return NR12 >> 4; }

bool ChannelOne::getEnvelopeDirection() const { return NR12 & 0x08; }

BYTE ChannelOne::getEnvelopePeriod() const { return NR12 & 0x07; }

// Channel One Period
void ChannelOne::setPeriod(WORD period) {
  NR13 = period & 0xFF;
  NR14 &= 0xF8;
  NR14 |= (period >> 8) & 0x07;
}

WORD ChannelOne::getPeriod() const { return ((NR14 & 0x07) << 8) | NR13; }

// NR14 Helper Functions
void ChannelOne::setLengthEnable(bool enable) {
  if (enable) {
    NR14 |= 0x40;
  } else {
    NR14 &= 0xBF;
  }
}

void ChannelOne::setTrigger(bool trigger) {
  if (trigger) {
    NR14 |= 0x80;
    this->trigger();
  } else {
    NR14 &= 0x7F;
  }
}

bool ChannelOne::getLengthEnable() const { return NR14 & 0x40; }

bool ChannelOne::getTrigger() const { return NR14 & 0x80; }

// Channel operations
void ChannelOne::trigger() {
  if ((NR12 & 0xF8) != 0) {
    state.dacEnabled = true;
  } else {
    state.dacEnabled = false;
    setEnabled(false);
    return;
  }

  setEnabled(true);
  state.lengthTimer = getInitialLengthTimer();
  state.envelopePeriod = getEnvelopePeriod();
  state.envelopeTimer = state.envelopePeriod;
  state.envelopeDirection = getEnvelopeDirection();
  state.volume = getInitialVolume();
  state.timer = (2048 - getPeriod()) * 4;
  state.sequencePointer = 0;
  state.sweepTimer = getSweepPace();
}

void ChannelOne::reset() {
  NR10 = 0x80;
  NR11 = 0xBF;
  NR12 = 0xF3;
  NR13 = 0x00;
  NR14 = 0xBF;
  channelEnabled = false;
}

float ChannelOne::getSample() {
  if (!state.dacEnabled || !isEnabled()) {
    return 0.0f; // DAC disabled
  }

  static const int dutyTable[4][8] = {
      {0, 0, 0, 0, 1, 1, 1, 1}, // 12.5%
      {0, 0, 0, 1, 1, 1, 1, 1}, // 25%
      {0, 0, 1, 1, 1, 1, 0, 0}, // 50%
      {1, 1, 1, 1, 0, 0, 0, 0}  // 75%
  };
  int dutyType = getWaveDuty();
  int duty = dutyTable[dutyType][state.sequencePointer];
  return duty ? state.volume : 0;
}

void ChannelOne::updateSequenceTimer(int cycles) {
  if (!isEnabled()) {
    return;
  }

  // Update the sequence timer
  state.timer -= cycles;
  if (state.timer <= 0) {
    state.timer = (2048 - getPeriod()) * 4;
    state.sequencePointer = (state.sequencePointer + 1) % 8;
  }
}

void ChannelOne::updateLengthTimer() {
  if (state.lengthTimer < 64) {
    state.lengthTimer++;
    if (state.lengthTimer == 64) {
      setEnabled(false);
    }
  }
}

void ChannelOne::updateEnvelope() {
  if (state.envelopePeriod == 0) {
    return;
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
bool ChannelOne::isEnabled() const { return channelEnabled; }

void ChannelOne::setEnabled(bool enable) { channelEnabled = enable; }

bool ChannelOne::isDacEnabled() const { return state.dacEnabled; }

// Channel One Sweep Specific functions (NR10)

// A number 0-7 that sets the sweep pace
// 0 will instantly disable the sweep
void ChannelOne::setSweepPace(BYTE pace) {
  NR10 &= 0x0F;
  NR10 |= (pace << 4);
}

BYTE ChannelOne::getSweepPace() const { return (NR10 >> 4) & 0x07; }

// True for increasing, false for decreasing
void ChannelOne::setSweepDirection(bool direction) {
  if (direction) {
    NR10 &= 0xF7; // Clear the bit
  } else {
    NR10 |= 0x08; // Set the bit
  }
}

// Get the sweep direction, true for increasing, false for decreasing
bool ChannelOne::getSweepDirection() const { return (NR10 & 0x08) == 0; }

// A number 0-7 that sets the individual step
void ChannelOne::setSweepStep(BYTE step) {
  NR10 &= 0xF8;
  NR10 |= (step & 0x07);
}
BYTE ChannelOne::getSweepStep() const { return NR10 & 0x07; }

// Called every 4 APU steps
void ChannelOne::sweepIteration() {
  if (getSweepPace() == 0 || !isEnabled()) {
    return; // 0 means no sweep
  }
  state.sweepTimer--;
  if (state.sweepTimer <= 0) {
    state.sweepTimer = getSweepPace();
    if (getSweepDirection()) {
      int holder = getPeriod() + (getPeriod() / pow(2, getSweepStep()));
      if (holder > 2047) {
        setEnabled(false);
      } else {
        setPeriod(holder);
      }
    } else {
      int holder = getPeriod() - (getPeriod() / pow(2, getSweepStep()));
      if (holder >= 0) {
        setPeriod(holder);
      }
    }
  }
}
