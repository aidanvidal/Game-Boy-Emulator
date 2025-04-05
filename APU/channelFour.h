#ifndef CHANNEL_FOUR_H
#define CHANNEL_FOUR_H

#include <cstdint>
#include <cmath>
#include <iostream>

typedef uint8_t BYTE;
typedef uint16_t WORD;

class ChannelFour {
private:
  // Channel State struct (moved from APU class)
  struct ChannelState {
    int lengthTimer;
    int envelopeTimer;      // Timer for controlling the envelope (volume)
    int envelopePeriod;     // Period for the envelope (how many ticks to update
                            // volume)
    bool envelopeDirection; // Direction of the envelope (true for increase,
                            // false for decrease)
    int lfsrTimer;          // Used for handling the LFSR
    bool lfsrWidth;  // Width of the LFSR (true for 7 bits, false for 15 bits)
    WORD lfsr;       // Current value of the LFSR
    int volume;      // Current volume level (0-15)
    bool dacEnabled; // DAC enabled or disabled (if volume is 0 and direction is
                     // decrease, DAC will be disabled)
  };

  BYTE NR41; // Sound Length
  BYTE NR42; // Volume Envelope
  BYTE NR43; // Polynomial Counter
  BYTE NR44; // Control Register
  ChannelState state;
  bool channelEnabled;

public:
  ChannelFour();

  // Register access functions
  BYTE getNR41() const;
  BYTE getNR42() const;
  BYTE getNR43() const;
  BYTE getNR44() const;

  void writeNR41(BYTE value);
  void writeNR42(BYTE value);
  void writeNR43(BYTE value);
  void writeNR44(BYTE value);

  // NR41 Helper Functions

  // Set the initial length timer (0-63)
  // Higher length means the shorter time before the channel is disabled
  void setInitialLengthTimer(BYTE timer);
  BYTE getInitialLengthTimer() const;

  // NR42 Helper Functions

  // Set the initial volume (0-15)
  void setInitialVolume(BYTE volume);
  BYTE getInitialVolume() const;

  // Set the envelope period (0-7)
  void setEnvelopePeriod(BYTE pace);
  BYTE getEnvelopePeriod() const;

  // Set the envelope direction (true for increase, false for decrease)
  void setEnvelopeDirection(bool direction);
  bool getEnvelopeDirection() const;

  // NR43 Helper Functions

  // Set the clock shift (0-15)
  void setClockShift(BYTE shift);
  BYTE getClockShift() const;

  // Set LSFR width (true for 7 bits, false for 15 bits)
  void setLSFRWidth(bool width);
  bool getLSFRWidth() const;

  // Set clock divider (0-7)
  void setClockDivider(BYTE divider);
  BYTE getClockDivider() const;

  // NR44 Helper Functions

  // Set the length enable (true for enabled, false for disabled)
  void setLengthEnable(bool enable);
  bool getLengthEnable() const;

  // Set the trigger (true for trigger, false for no trigger)
  void setTrigger(bool trigger);
  bool getTrigger() const;

  // Channel operations
  void trigger();
  void reset();
  float getSample();
  void updateLengthTimer();
  void updateEnvelope();
  void updateLFSR(int cycles);

  // Status functions
  bool isEnabled() const;
  void setEnabled(bool enable);
  bool isDacEnabled() const;
};
#endif