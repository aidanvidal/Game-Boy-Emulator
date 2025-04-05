#ifndef CHANNEL_ONE_HPP
#define CHANNEL_ONE_HPP

#include <cstdint>

typedef uint8_t BYTE;
typedef uint16_t WORD;

class ChannelOne {
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
    int sweepTimer;  // Timer for controlling the sweep
  };

  BYTE NR10; // Sweep Register
  BYTE NR11; // Sound Length/Wave Pattern Duty
  BYTE NR12; // Volume Envelope
  BYTE NR13; // Frequency Low
  BYTE NR14; // Frequency High
  ChannelState state;
  bool channelEnabled;

public:
  ChannelOne();

  // Register access functions
  BYTE getNR10() const;
  BYTE getNR11() const;
  BYTE getNR12() const;
  BYTE getNR13() const;
  BYTE getNR14() const;

  void writeNR10(BYTE value);
  void writeNR11(BYTE value);
  void writeNR12(BYTE value);
  void writeNR13(BYTE value);
  void writeNR14(BYTE value);

  // NR11 Helper Functions

  // Set the wave duty (0-3)
  void setWaveDuty(BYTE duty);

  // Set the initial length timer (0-63)
  void setInitialLengthTimer(BYTE timer);
  BYTE getWaveDuty() const;
  BYTE getInitialLengthTimer() const;

  // NR12 Helper Functions

  // Set the initial volume (0-15)
  void setInitialVolume(BYTE volume);

  // Envelope direction (true for increase, false for decrease)
  void setEnvelopeDirection(bool direction);

  // Envelope period (0-7)
  void setEnvelopePeriod(BYTE pace);
  BYTE getInitialVolume() const;
  bool getEnvelopeDirection() const;
  BYTE getEnvelopePeriod() const;

  // Channel One Period

  // Set the period (0-2047)
  void setPeriod(WORD period);
  WORD getPeriod() const;

  // NR14 Helper Functions
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

  // Sweep functions

  // A number 0-7 that sets the sweep pace, 0 will disable the sweep
  void setSweepPace(BYTE pace);
  BYTE getSweepPace() const;

  // Set the sweep direction (true for increase, false for decrease)
  void setSweepDirection(bool direction);
  bool getSweepDirection() const;

  // Set the sweep step (0-7)
  void setSweepStep(BYTE step);
  BYTE getSweepStep() const;
  void sweepIteration();
};

#endif // CHANNEL_ONE_HPP