#include "APU.h"

// Constructor
APU::APU() {
  // Initialize the APU registers
  NR52 = 0xF1;
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

void APU::writeData(WORD address, BYTE value) {
  switch (address) {
  // Channel One
  case 0xFF10:
    channelOne.writeNR10(value);
    break; // NR10
  case 0xFF11:
    channelOne.writeNR11(value);
    break; // NR11
  case 0xFF12:
    channelOne.writeNR12(value);
    break; // NR12
  case 0xFF13:
    channelOne.writeNR13(value);
    break; // NR13
  case 0xFF14:
    channelOne.writeNR14(value);
    break; // NR14

  // Channel Two
  case 0xFF16:
    channelTwo.writeNR21(value);
    break; // NR21
  case 0xFF17:
    channelTwo.writeNR22(value);
    break; // NR22
  case 0xFF18:
    channelTwo.writeNR23(value);
    break; // NR23
  case 0xFF19:
    channelTwo.writeNR24(value);
    break; // NR24

  // Channel Three
  case 0xFF1A:
    channelThree.writeNR30(value);
    break; // NR30
  case 0xFF1B:
    channelThree.writeNR31(value);
    break; // NR31
  case 0xFF1C:
    channelThree.writeNR32(value);
    break; // NR32
  case 0xFF1D:
    channelThree.writeNR33(value);
    break; // NR33
  case 0xFF1E:
    channelThree.writeNR34(value);
    break; // NR34

  // Channel Four
  case 0xFF20:
    channelFour.writeNR41(value);
    break; // NR41
  case 0xFF21:
    channelFour.writeNR42(value);
    break; // NR42
  case 0xFF22:
    channelFour.writeNR43(value);
    break; // NR43
  case 0xFF23:
    channelFour.writeNR44(value);
    break; // NR44

  // NR50, NR51, NR52
  case 0xFF24:
    writeNR50(value);
    break; // NR50
  case 0xFF25:
    writeNR51(value);
    break; // NR51
  case 0xFF26:
    writeNR52(value);
    break; // NR52

  default:
    break; // Invalid address
  }
}

BYTE APU::getData(WORD address) const {
  switch (address) {
  // Channel One
  case 0xFF10:
    return channelOne.getNR10(); // NR10
  case 0xFF11:
    return channelOne.getNR11(); // NR11
  case 0xFF12:
    return channelOne.getNR12(); // NR12
  case 0xFF13:
    return channelOne.getNR13(); // NR13
  case 0xFF14:
    return channelOne.getNR14(); // NR14

  // Channel Two
  case 0xFF16:
    return channelTwo.getNR21(); // NR21
  case 0xFF17:
    return channelTwo.getNR22(); // NR22
  case 0xFF18:
    return channelTwo.getNR23(); // NR23
  case 0xFF19:
    return channelTwo.getNR24(); // NR24

  // Channel Three
  case 0xFF1A:
    return channelThree.getNR30(); // NR30
  case 0xFF1B:
    return channelThree.getNR31(); // NR31
  case 0xFF1C:
    return channelThree.getNR32(); // NR32
  case 0xFF1D:
    return channelThree.getNR33(); // NR33
  case 0xFF1E:
    return channelThree.getNR34(); // NR34

  // Channel Four
  case 0xFF20:
    return channelFour.getNR41(); // NR41
  case 0xFF21:
    return channelFour.getNR42(); // NR42
  case 0xFF22:
    return channelFour.getNR43(); // NR43
  case 0xFF23:
    return channelFour.getNR44(); // NR44

  // NR50, NR51, NR52
  case 0xFF24:
    return NR50; // NR50
  case 0xFF25:
    return NR51; // NR51
  case 0xFF26:
    return NR52; // NR52

  default:
    return 0xFF; // Invalid address, return default value
  }
}

// Destructor
APU::~APU() {
  SDL_CloseAudio();
  SDL_Quit();
}