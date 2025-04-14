#ifndef CHANNEL_THREE_HPP
#define CHANNEL_THREE_HPP
#include <cstdint>
#include <iostream>

typedef uint8_t BYTE;
typedef uint16_t WORD;

class ChannelThree {
private:
  struct ChannelState {
    int lengthTimer;
    int volume;
    bool dacEnabled;
    int sampleSelection; // The index for which sample to be played 0-31
    int sampleTimer;     // Timer for sample selection
    float sampleBuffer;  // Buffer to store the last sample read
  };
  BYTE NR30; // DAC Control
  BYTE NR31; // Sound Length
  BYTE NR32; // Select Output Level
  BYTE NR33; // Period Low
  BYTE NR34; // Period High and Control
  ChannelState state;
  BYTE wavePatternRAM[16]; // 16 bytes of wave pattern RAM FF30-FF3F
  bool channelEnabled;

public:
  ChannelThree();

  // Register access functions
  BYTE getNR30() const;
  BYTE getNR31() const;
  BYTE getNR32() const;
  BYTE getNR33() const;
  BYTE getNR34() const;
  void writeNR30(BYTE value);
  void writeNR31(BYTE value);
  void writeNR32(BYTE value);
  void writeNR33(BYTE value);
  void writeNR34(BYTE value);

  // NR30 Helper Functions
  void setDACEnable(bool enable);
  bool getDACEnable() const;

  // NR31 Helper Functions
  void setInitialLengthTimer(BYTE timer);
  BYTE getInitialLengthTimer() const;

  // NR32 Helper Functions

  // Set the output level (0-3)
  // 0: mute
  // 1: 100% volume
  // 2: 50% volume
  // 3: 25% volume
  void setOutputLevel(BYTE level);
  BYTE getOutputLevel() const;

  // NR33 Helper Functions
  void setPeriodLow(BYTE period);
  BYTE getPeriodLow() const;

  // NR34 Helper Functions
  void setPeriodHigh(BYTE period);
  BYTE getPeriodHigh() const;
  // Set the period (0-2047)
  void setPeriod(WORD period);
  WORD getPeriod() const;
  void setLengthEnable(bool enable);
  bool getLengthEnable() const;
  void setTrigger(bool trigger);
  bool getTrigger() const;

  // Channel operations
  void trigger();
  void setEnabled(bool enable);
  bool isEnabled() const;
  void reset();
  float getSample();
  void updateLengthTimer();

  // Wave Pattern RAM operations
  void setNibbleWavePatternRAM(int index, BYTE value, bool upper);
  BYTE getNibbleWavePatternRAM(int index, bool upper) const;
  void setWavePatternRAM(WORD address, BYTE value);
  BYTE getWavePatternRAM(WORD address) const;
  void updateSampleTimer(int cycles);
};

#endif