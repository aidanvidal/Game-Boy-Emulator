#ifndef CHANNEL_TWO_HPP
#define CHANNEL_TWO_HPP

#include <cstdint>
#include <iostream>

typedef uint8_t BYTE;
typedef uint16_t WORD;

class ChannelTwo {
private:
  // Channel State struct (moved from APU class)
  struct ChannelState {
    int lengthTimer;
    int envelopeTimer;      // Timer for controlling the envelope (volume)
    int envelopePeriod;     // Period for the envelope (how many ticks to update
                            // volume)
    bool envelopeDirection; // Direction of the envelope (true for increase,
                            // false for decrease)
    int timer;              // Used for incrementing sequencePointer
    int volume;             // Current volume level (0-15)
    int sequencePointer;    // Pointer to the current position in the waveform
                            // sequence
    bool dacEnabled; // DAC enabled or disabled (if volume is 0 and direction is
                     // decrease, DAC will be disabled)
  };

  BYTE NR21; // Sound Length/Wave Pattern Duty
  BYTE NR22; // Volume Envelope
  BYTE NR23; // Frequency Low
  BYTE NR24; // Frequency High
  ChannelState state;
  bool channelEnabled;

public:
  ChannelTwo();

  // Register access functions
  BYTE getNR21() const;
  BYTE getNR22() const;
  BYTE getNR23() const;
  BYTE getNR24() const;

  void writeNR21(BYTE value);
  void writeNR22(BYTE value);
  void writeNR23(BYTE value);
  void writeNR24(BYTE value);

  // NR21 Helper Functions

  // Set the wave duty (0-3)
  void setWaveDuty(BYTE duty);

  // Set the initial length timer (0-63)
  void setInitialLengthTimer(BYTE timer);
  BYTE getWaveDuty() const;
  BYTE getInitialLengthTimer() const;

  // NR22 Helper Functions

  // Set the initial volume (0-15)
  void setInitialVolume(BYTE volume);

  // Envelope direction (true for increase, false for decrease)
  void setEnvelopeDirection(bool direction);

  // Envelope period (0-7)
  void setEnvelopePeriod(BYTE pace);
  BYTE getInitialVolume() const;
  bool getEnvelopeDirection() const;
  BYTE getEnvelopePeriod() const;

  // Channel Two Period

  // Set the period (0-2047)
  void setPeriod(WORD period);
  WORD getPeriod() const;

  // NR24 Helper Functions
  void setLengthEnable(bool enable);
  void setTrigger(bool trigger);
  bool getLengthEnable() const;
  bool getTrigger() const;

  // Channel operations
  void trigger();
  void reset();
  float getSample();
  void updateSequenceTimer(int cycles);
  void updateLengthTimer();
  void updateEnvelope();

  // Status functions
  bool isEnabled() const;
  void setEnabled(bool enable);
  bool isDacEnabled() const;
};

#endif // CHANNEL_TWO_HPP