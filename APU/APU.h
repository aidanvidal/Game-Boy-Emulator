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

  // APU Helper Functions
  void getAudioSample(int cycles);
  void updateChannelTimers(int cycles);
  void writeData(WORD address, BYTE value);
  BYTE getData(WORD address) const;

  // Destructor
  ~APU();
};

#endif // APU_H