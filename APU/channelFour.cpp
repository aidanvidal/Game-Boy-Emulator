#include "channelFour.h"

ChannelFour::ChannelFour() {
  // Initialize the registers with default values
  NR41 = 0xFF;
  NR42 = 0x00;
  NR43 = 0x00;
  NR44 = 0x00;
  channelEnabled = false;
  state.dacEnabled = false;
  state.lfsr = 0;
}

// Register access functions
BYTE ChannelFour::getNR41() const { return NR41; }
BYTE ChannelFour::getNR42() const { return NR42; }
BYTE ChannelFour::getNR43() const { return NR43; }
BYTE ChannelFour::getNR44() const { return NR44; }
void ChannelFour::writeNR41(BYTE value) { NR41 = value; }
// Writes to this register require retriggering
void ChannelFour::writeNR42(BYTE value) { NR42 = value; }
void ChannelFour::writeNR43(BYTE value) { NR43 = value; }
void ChannelFour::writeNR44(BYTE value) {
  NR44 = value;
  if ((value & 0x80)) {
    trigger();
  }
}

// NR41 Helper Functions

// Set the initial length timer (0-63)
void ChannelFour::setInitialLengthTimer(BYTE timer) {
  NR41 &= 0xC0;
  NR41 |= timer;
}

BYTE ChannelFour::getInitialLengthTimer() const { return NR41 & 0x3F; }

// NR42 Helper Functions

// Set the initial volume (0-15)
void ChannelFour::setInitialVolume(BYTE volume) {
  NR42 &= 0x0F;
  NR42 |= (volume << 4);
}

// Get the initial volume (0-15)
BYTE ChannelFour::getInitialVolume() const { return NR42 >> 4; }

// Set the envelope period (0-7)
void ChannelFour::setEnvelopePeriod(BYTE pace) {
  NR42 &= 0xF8;
  NR42 |= pace;
}

// Get the envelope period (0-7)
BYTE ChannelFour::getEnvelopePeriod() const { return NR42 & 0x07; }

// Set the envelope direction (true for increase, false for decrease)
void ChannelFour::setEnvelopeDirection(bool direction) {
  if (direction) {
    NR42 |= 0x08;
  } else {
    NR42 &= 0xF7;
  }
}

// Get the envelope direction (true for increase, false for decrease)
bool ChannelFour::getEnvelopeDirection() const { return (NR42 & 0x08) != 0; }

// NR43 Helper Functions

// Set the clock shift (0-15)
void ChannelFour::setClockShift(BYTE shift) {
  NR43 &= 0x0F;
  NR43 |= (shift << 4);
}

// Get the clock shift (0-15)
BYTE ChannelFour::getClockShift() const { return (NR43 >> 4) & 0x0F; }

// Set LSFR width 0 for 15 bits, 1 for 7 bits
void ChannelFour::setLSFRWidth(bool width) {
  if (width) {
    NR43 |= 0x08;
  } else {
    NR43 &= 0xF7;
  }
}

// Get LSFR width 0 for 15 bits, 1 for 7 bits
bool ChannelFour::getLSFRWidth() const { return (NR43 & 0x08) != 0; }

// Set clock divider (0-7)
void ChannelFour::setClockDivider(BYTE divider) {
  NR43 &= 0xE0;
  NR43 |= (divider & 0x07);
}

// Get clock divider (0-7)
BYTE ChannelFour::getClockDivider() const { return NR43 & 0x07; }

// NR44 Helper Functions

// Set the length enable (true for enabled, false for disabled)
void ChannelFour::setLengthEnable(bool enable) {
  if (enable) {
    NR44 |= 0x40;
  } else {
    NR44 &= 0xBF;
  }
}

// Get the length enable (true for enabled, false for disabled)

bool ChannelFour::getLengthEnable() const { return (NR44 & 0x40) != 0; }

// Set the trigger (true for trigger, false for no trigger)
void ChannelFour::setTrigger(bool trigger) {
  if (trigger) {
    NR44 |= 0x80;
    this->trigger();
  } else {
    NR44 &= 0x7F;
  }
}

// Get the trigger (true for trigger, false for no trigger)
bool ChannelFour::getTrigger() const { return (NR44 & 0x80) != 0; }

// Channel operations

void ChannelFour::trigger() {
  if ((NR42 & 0xF8) != 0) {
    state.dacEnabled = true;
  } else {
    state.dacEnabled = false;
    setEnabled(false);
    return;
  }
  // Set the initial volume
  state.volume = getInitialVolume();
  // Set the envelope period
  state.envelopePeriod = getEnvelopePeriod();
  state.envelopeDirection = getEnvelopeDirection();
  state.envelopeTimer = state.envelopePeriod;
  // Set the initial length timer
  state.lengthTimer = getInitialLengthTimer();
  // Set the LFSR width
  state.lfsrWidth = getLSFRWidth();
  if (getClockDivider() == 0) {
    // Handle 0 as 0.5
    state.lfsrTimer = 16 * 0.5 * pow(2, getClockShift());
  } else {
    state.lfsrTimer = 16 * getClockDivider() * pow(2, getClockShift());
  }
  // Set the LFSR
  state.lfsr = 0;
  setEnabled(true);
}

// Reset the channel
void ChannelFour::reset() {
  NR41 = 0xFF;
  NR42 = 0x00;
  NR43 = 0x00;
  NR44 = 0x00;
  channelEnabled = false;
  state.dacEnabled = false;
  state.lfsr = 0;
}

// Get the sample from Channel Four
float ChannelFour::getSample() {
  if (!state.dacEnabled) {
    return 7.0f; // If DAC is disabled, return 0
  }
  if (!isEnabled()) {
    return 15.0f; // Disabled channel outputs "analog 1"
  }

  // Use LFSR for waveform generation
  return (state.lfsr & 1) ? state.volume : 0; // Return volume if LFSR is high
}

// Update the length timer
void ChannelFour::updateLengthTimer() {
  if (state.lengthTimer < 64) {
    state.lengthTimer++;
    // If length timer reaches zero, turn off the channel
    if (state.lengthTimer == 64) {
      setEnabled(false); // Disable channel
      std::cout << "Channel Four Disabled due to length timer." << std::endl;
    }
  }
}

// Update the envelope
void ChannelFour::updateEnvelope() {
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

// Update the LFSR
void ChannelFour::updateLFSR(int cycles) {
  state.lfsrTimer -= cycles;
  if (state.lfsrTimer <= 0) {
    state.lfsrTimer = (16 * getClockDivider() * pow(2, getClockShift()));
    // Update LFSR
    if (state.lfsrWidth) {
      // 7-bit LFSR
      // Perform an XNOR between bit 0 and bit 1,
      // Store results in bit 15 and bit 7
      // Then shift the LFSR to the right by 1
      bool xnorResult =
          !((state.lfsr & 1) ^ ((state.lfsr >> 1) & 1)); // XNOR bit 0 and bit 1
      state.lfsr = state.lfsr | (xnorResult << 15) |
                   (xnorResult << 7); // Store in bit 15 and bit 7
      state.lfsr = (state.lfsr >> 1); // Shift right by 1
    } else {
      // 15-bit LFSR
      // Perform an XNOR between bit 0 and bit 1,
      // Store results in bit 15
      // Then shift the LFSR to the right by 1
      bool xnorResult =
          !((state.lfsr & 1) ^ ((state.lfsr >> 1) & 1)); // XNOR bit 0 and bit 1
      state.lfsr = state.lfsr | (xnorResult << 15);      // Store in bit 15
      state.lfsr = (state.lfsr >> 1);                    // Shift right by 1
    }
  }
}

// Status functions
bool ChannelFour::isEnabled() const { return channelEnabled; }
void ChannelFour::setEnabled(bool enable) { channelEnabled = enable; }
bool ChannelFour::isDacEnabled() const { return state.dacEnabled; }