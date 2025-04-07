#include "APU.h"

// Constructor
APU::APU() {
  // Initialize the APU registers
  NR52 = 0x00;
  NR51 = 0xF3;
  NR50 = 0x77;
  frameCounter = 0;
  audioCounter = 0;
  APUEnabled = false;

  // Set up SDL audio spec
  SDL_AudioSpec audioSpec;
  audioSpec.freq = 44100;
  audioSpec.format = AUDIO_F32SYS;
  audioSpec.channels = 2;         // Stereo
  audioSpec.samples = sampleSize; // Adjust as needed
  audioSpec.callback = NULL;
  audioSpec.userdata = this;

  SDL_AudioSpec obtainedSpec;
  SDL_OpenAudio(&audioSpec, &obtainedSpec);
  SDL_PauseAudio(0);
}

// APU Step
void APU::apuStep(int cycles) {
  if (!APUEnabled)
    return;

  // Update the frame sequencer
  static int step = 0;
  frameCounter += cycles;

  if (frameCounter < 8192) {
    return; // Not enough cycles for a step
  }

  frameCounter -= 8192;
  if (step % 2 == 0) {
    // Clock the length timer
    if (channelOne.getLengthEnable() && channelOne.isEnabled()) {
      channelOne.updateLengthTimer();
    }
    if (channelTwo.getLengthEnable() && channelTwo.isEnabled()) {
      channelTwo.updateLengthTimer();
    }
    if (channelThree.getLengthEnable() && channelThree.isEnabled()) {
      channelThree.updateLengthTimer();
    }
    if (channelFour.getLengthEnable() && channelFour.isEnabled()) {
      channelFour.updateLengthTimer();
    }
  }

  if (step == 3 || step == 7) {
    // Clock the sweep
    channelOne.sweepIteration();
  }

  if (step == 7) {
    // Clock the envelope
    channelOne.updateEnvelope();
    channelTwo.updateEnvelope();
    channelFour.updateEnvelope();
  }

  step = (step + 1) % 8;
}

// Reset APU
void APU::resetAPU() {
  // Reset all sound registers except NR52
  NR51 = 0xF3;          // Reset NR51 to default value
  NR50 = 0x77;          // Reset NR50 to default value
  channelOne.reset();   // Reset channel one
  channelTwo.reset();   // Reset channel two
  channelThree.reset(); // Reset channel three
  channelFour.reset();  // Reset channel four
}

// NR52 Helper Functions
bool APU::getChannelStatus(BYTE channel) { return NR52 & (1 << (channel - 1)); }

void APU::setChannelStatus(BYTE channel, bool enable) {
  switch (channel) {
  case 1:
    channelOne.setEnabled(enable);
    break;
  case 2:
    channelTwo.setEnabled(enable);
    break;
  case 3:
    channelThree.setEnabled(enable);
    break;
  case 4:
    channelFour.setEnabled(enable);
    break;
  default:
    return; // Invalid channel, do nothing
  }

  if (enable) {
    NR52 |= (1 << (channel - 1));
  } else {
    NR52 &= ~(1 << (channel - 1));
  }
}

void APU::setAPUStatus(bool enable) {
  if (enable) {
    NR52 |= 0x80;
    APUEnabled = true;
  } else {
    NR52 &= 0x7F;
    APUEnabled = false;
    resetAPU();
  }
}

void APU::writeNR52(BYTE value) {
  NR52 = value;
  setAPUStatus(value & 0x80);
}

// NR51 Helper Functions
void APU::writeNR51(BYTE value) { NR51 = value; }

void APU::writeNR51Part(BYTE channel, bool right, bool enable) {
  if (right) {
    if (enable) {
      NR51 |= (1 << (channel - 1));
    } else {
      NR51 &= ~(1 << (channel - 1));
    }
  } else {
    if (enable) {
      NR51 |= (1 << (channel + 3));
    } else {
      NR51 &= ~(1 << (channel + 3));
    }
  }
}

void APU::getChannelPanning(BYTE channel, bool &left, bool &right) {
  left = NR51 & (1 << (channel + 3));
  right = NR51 & (1 << (channel - 1));
}

// NR50 Helper Functions
void APU::writeNR50(BYTE value) { NR50 = value; }

void APU::setVIN(bool enable, bool right) {
  if (right) {
    if (enable) {
      NR50 |= 0x08;
    } else {
      NR50 &= 0xF7;
    }
  } else {
    if (enable) {
      NR50 |= 0x80;
    } else {
      NR50 &= 0x7F;
    }
  }
}

void APU::setVolumeLevel(BYTE volume, bool right) {
  if (right) {
    NR50 &= 0xF8;
    NR50 |= volume;
  } else {
    NR50 &= 0x8F;
    NR50 |= (volume << 4);
  }
}

void APU::getVolumeLevel(BYTE &left, BYTE &right) {
  left = NR50 >> 4;
  right = NR50 & 0x07;
}

void APU::getVIN(bool &left, bool &right) {
  left = NR50 & 0x80;
  right = NR50 & 0x08;
}

// APU Helper Functions
void APU::getAudioSample(int cycles) {
  if (!APUEnabled) {
    return;
  }

  audioCounter += cycles;

  if (audioCounter < 95) {
    return; // Not enough cycles for a sample
  }

  audioCounter -= 95;
  // Get the audio samples
  float sampleOne = channelOne.getSample();
  float sampleTwo = channelTwo.getSample();
  float sampleThree = channelThree.getSample();
  float sampleFour = channelFour.getSample();

  // Preform the mixing for the panning
  BYTE volumeLeft = 0;
  BYTE volumeRight = 0;
  getVolumeLevel(volumeLeft, volumeRight);
  volumeLeft = (volumeLeft * 128) / 7; // Scale to 0-128 for SDL
  volumeRight = (volumeRight * 128) / 7;
  float bufferLeft = 0.0f;
  float bufferRight = 0.0f;
  float tempLeft = 0.0f;
  float tempRight = 0.0f;
  bool leftChannel = false;
  bool rightChannel = false;

  // Channel 1 panning
  getChannelPanning(1, leftChannel, rightChannel);
  if (leftChannel) {
    bufferLeft = (sampleOne) / 15.0f; // Normalize to 0-1
    SDL_MixAudioFormat((Uint8 *)&tempLeft, (Uint8 *)&bufferLeft, AUDIO_F32SYS,
                       sizeof(float), volumeLeft);
  }
  if (rightChannel) {
    bufferRight = sampleOne / 15.0f;
    SDL_MixAudioFormat((Uint8 *)&tempRight, (Uint8 *)&bufferRight, AUDIO_F32SYS,
                       sizeof(float), volumeRight);
  }
  // Channel 2 panning
  getChannelPanning(2, leftChannel, rightChannel);
  if (leftChannel) {
    bufferLeft = sampleTwo / 15.0f;
    SDL_MixAudioFormat((Uint8 *)&tempLeft, (Uint8 *)&bufferLeft, AUDIO_F32SYS,
                       sizeof(float), volumeLeft);
  }
  if (rightChannel) {
    bufferRight = sampleTwo / 15.0f;
    SDL_MixAudioFormat((Uint8 *)&tempRight, (Uint8 *)&bufferRight, AUDIO_F32SYS,
                       sizeof(float), volumeRight);
  }
  // Channel 3 panning
  getChannelPanning(3, leftChannel, rightChannel);
  if (leftChannel) {
    bufferLeft = sampleThree / 15.0f;
    SDL_MixAudioFormat((Uint8 *)&tempLeft, (Uint8 *)&bufferLeft, AUDIO_F32SYS,
                       sizeof(float), volumeLeft);
  }
  if (rightChannel) {
    bufferRight = sampleThree / 15.0f;
    SDL_MixAudioFormat((Uint8 *)&tempRight, (Uint8 *)&bufferRight, AUDIO_F32SYS,
                       sizeof(float), volumeRight);
  }
  // Channel 4 panning
  getChannelPanning(4, leftChannel, rightChannel);
  if (leftChannel) {
    bufferLeft = sampleFour / 15.0f;
    SDL_MixAudioFormat((Uint8 *)&tempLeft, (Uint8 *)&bufferLeft, AUDIO_F32SYS,
                       sizeof(float), volumeLeft);
  }
  if (rightChannel) {
    bufferRight = sampleFour / 15.0f;
    SDL_MixAudioFormat((Uint8 *)&tempRight, (Uint8 *)&bufferRight, AUDIO_F32SYS,
                       sizeof(float), volumeRight);
  }

  // Add the samples to the buffer
  buffer[bufferFill] = tempLeft;
  buffer[bufferFill + 1] = tempRight;
  bufferFill += 2;

  // If the buffer is full, queue it for playback
  if (bufferFill >= sampleSize) {
    bufferFill = 0;
    while (SDL_GetQueuedAudioSize(1) > sampleSize * sizeof(float)) {
      SDL_Delay(1);
    }
    SDL_QueueAudio(1, buffer, sampleSize * sizeof(float));
  }
}

void APU::updateChannelTimers(int cycles) {
  channelOne.updateSequenceTimer(cycles);
  channelTwo.updateSequenceTimer(cycles);
  channelThree.updateSampleTimer(cycles);
  channelFour.updateLFSR(cycles);
}

/*--------Channel One Functions--------------*/
void APU::writeNR10(BYTE value) { channelOne.setSweepPace(value); }
void APU::writeNR11(BYTE value) { channelOne.setWaveDuty(value); }
void APU::writeNR12(BYTE value) { channelOne.setInitialVolume(value); }
void APU::writeNR13(BYTE value) { channelOne.setPeriod(value); }
void APU::writeNR14(BYTE value) { channelOne.setTrigger(value & 0x80); }
BYTE APU::getNR10() const { return channelOne.getNR10(); }
BYTE APU::getNR11() const { return channelOne.getNR11(); }
BYTE APU::getNR12() const { return channelOne.getNR12(); }
BYTE APU::getNR13() const { return channelOne.getNR13(); }
BYTE APU::getNR14() const { return channelOne.getNR14(); }

/*--------Channel Two Functions--------------*/
void APU::writeNR21(BYTE value) { channelTwo.setWaveDuty(value); }
void APU::writeNR22(BYTE value) { channelTwo.setInitialVolume(value); }
void APU::writeNR23(BYTE value) { channelTwo.setPeriod(value); }
void APU::writeNR24(BYTE value) { channelTwo.setTrigger(value & 0x80); }
BYTE APU::getNR21() const { return channelTwo.getNR21(); }
BYTE APU::getNR22() const { return channelTwo.getNR22(); }
BYTE APU::getNR23() const { return channelTwo.getNR23(); }
BYTE APU::getNR24() const { return channelTwo.getNR24(); }

/*--------Channel Three Functions--------------*/
void APU::writeNR30(BYTE value) { channelThree.setDACEnable(value & 0x80); }
void APU::writeNR31(BYTE value) { channelThree.setInitialLengthTimer(value); }
void APU::writeNR32(BYTE value) { channelThree.setOutputLevel(value); }
void APU::writeNR33(BYTE value) { channelThree.setPeriodLow(value); }
void APU::writeNR34(BYTE value) { channelThree.setPeriodHigh(value); }
BYTE APU::getNR30() const { return channelThree.getNR30(); }
BYTE APU::getNR31() const { return channelThree.getNR31(); }
BYTE APU::getNR32() const { return channelThree.getNR32(); }
BYTE APU::getNR33() const { return channelThree.getNR33(); }
BYTE APU::getNR34() const { return channelThree.getNR34(); }

/*--------Channel Four Functions--------------*/
void APU::writeNR41(BYTE value) { channelFour.setInitialLengthTimer(value); }
void APU::writeNR42(BYTE value) { channelFour.setInitialVolume(value); }
void APU::writeNR43(BYTE value) { channelFour.setClockShift(value); }
void APU::writeNR44(BYTE value) { channelFour.setTrigger(value & 0x80); }
BYTE APU::getNR41() const { return channelFour.getNR41(); }
BYTE APU::getNR42() const { return channelFour.getNR42(); }
BYTE APU::getNR43() const { return channelFour.getNR43(); }
BYTE APU::getNR44() const { return channelFour.getNR44(); }

// Destructor
APU::~APU() {
  SDL_CloseAudio();
  SDL_Quit();
}