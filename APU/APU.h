#ifndef APU_H
#define APU_H

#include "channelFour.h"
#include "channelOne.h"
#include "channelThree.h"
#include "channelTwo.h"
#include <SDL2/SDL.h>
#include <cstdint>
#include <iostream>

typedef uint8_t BYTE;
typedef uint16_t WORD;
#define sampleSize 2048 // Size of the audio buffer

class APU {
private:
  BYTE NR52;                  // Audio Master Control
  BYTE NR51;                  // Sound Panning
  BYTE NR50;                  // Master Volume and VIN Panning

  int frameCounter;
  int audioCounter;
  int bufferFill = 0;
  float buffer[sampleSize] = {0}; // Buffer for audio samples

public:
  bool APUEnabled;
  ChannelOne channelOne;     // Using the ChannelOne class
  ChannelTwo channelTwo;     // Using the ChannelTwo class
  ChannelThree channelThree; // Using the ChannelThree class
  ChannelFour channelFour;   // Using the ChannelFour class

  APU(); // Constructor

  void apuStep(int cycles);
  void resetAPU();

  // NR52 Helper Functions
  bool getChannelStatus(BYTE channel);
  void setChannelStatus(BYTE channel, bool enable);
  void setAPUStatus(bool enable);
  void writeNR52(BYTE value);

  // NR51 Helper Functions
  void writeNR51(BYTE value);
  void writeNR51Part(BYTE channel, bool right, bool enable);
  void getChannelPanning(BYTE channel, bool &left, bool &right);

  // NR50 Helper Functions
  void writeNR50(BYTE value);
  void setVIN(bool enable, bool right);
  void setVolumeLevel(BYTE volume, bool right);
  void getVolumeLevel(BYTE &left, BYTE &right);
  void getVIN(bool &left, bool &right);

  // Channel One Functions
  void writeNR10(BYTE value);
  void writeNR11(BYTE value);
  void writeNR12(BYTE value);
  void writeNR13(BYTE value);
  void writeNR14(BYTE value);
  BYTE getNR10() const;
  BYTE getNR11() const;
  BYTE getNR12() const;
  BYTE getNR13() const;
  BYTE getNR14() const;

  // Channel Two Functions
  void writeNR21(BYTE value);
  void writeNR22(BYTE value);
  void writeNR23(BYTE value);
  void writeNR24(BYTE value);
  BYTE getNR21() const;
  BYTE getNR22() const;
  BYTE getNR23() const;
  BYTE getNR24() const;

  // Channel Three Functions
  void writeNR30(BYTE value);
  void writeNR31(BYTE value);
  void writeNR32(BYTE value);
  void writeNR33(BYTE value);
  void writeNR34(BYTE value);
  BYTE getNR30() const;
  BYTE getNR31() const;
  BYTE getNR32() const;
  BYTE getNR33() const;
  BYTE getNR34() const;

  // Channel Four Functions
  void writeNR41(BYTE value);
  void writeNR42(BYTE value);
  void writeNR43(BYTE value);
  void writeNR44(BYTE value);
  BYTE getNR41() const;
  BYTE getNR42() const;
  BYTE getNR43() const;
  BYTE getNR44() const;

  // APU Helper Functions
  void getAudioSample(int cycles);
  void updateChannelTimers(int cycles);

  // Destructor
  ~APU();
};

#endif // APU_H