#include "APU.h"

/*--------------CPU Simulation-------------*/

int handleInstruction() {
  return 4; // Return the number of cycles for the instruction executed
}

void cpuUpdate(APU &apu) {
  const int MAXCYCLES = 69905;
  int cyclesThisUpdate = 0;
  int apuCycleCounter = 0;
  int sampleCycleCounter = 0;
  const int cyclesPerSample = 4194304 / 44100;

  while (cyclesThisUpdate < MAXCYCLES) {
    int cycles = handleInstruction();
    cyclesThisUpdate += cycles;
    apuCycleCounter += cycles;
    sampleCycleCounter += cycles;
    // Update the sequence timer for channels 1 and 2
    apu.channelOne.updateSequenceTimer(cycles);
    apu.channelTwo.updateSequenceTimer(cycles);
    // Update the sample timer for channel 3
    apu.channelThree.updateSampleTimer(cycles);
    // Update the LFSR for channel 4
    apu.channelFour.updateLFSR(cycles);
    // Every 512Hz update the APU frame sequencer
    if (apuCycleCounter >= 8192) {
      apu.apuStep();
      apuCycleCounter -= 8192;
    }

    // Every 44100Hz get the audio sample
    if (sampleCycleCounter >= cyclesPerSample) {
      apu.getAudioSample(cycles);
      sampleCycleCounter -= cyclesPerSample;
    }
  }
}

/*--------Channel Configs--------------*/

void configureChannelOne(APU &apu, int initialVolume, int waveDuty, int period,
                         int envelopePeriod, bool envelopeDirection,
                         int lengthTimer, bool lengthEnable, int sweepPace,
                         bool sweepDirection, int sweepStep) {
  apu.channelOne.setInitialVolume(initialVolume);
  apu.channelOne.setWaveDuty(waveDuty);
  apu.channelOne.setPeriod(period);
  apu.channelOne.setEnvelopePeriod(envelopePeriod);
  apu.channelOne.setEnvelopeDirection(envelopeDirection);
  apu.channelOne.setInitialLengthTimer(lengthTimer);
  apu.channelOne.setLengthEnable(lengthEnable);
  apu.channelOne.setSweepPace(sweepPace);
  apu.channelOne.setSweepDirection(sweepDirection);
  apu.channelOne.setSweepStep(sweepStep);
  apu.channelOne.trigger();
}

void configureChannelTwo(APU &apu, int initialVolume, int waveDuty, int period,
                         int envelopePeriod, bool envelopeDirection,
                         int lengthTimer, bool lengthEnable) {
  apu.channelTwo.setInitialVolume(initialVolume);
  apu.channelTwo.setWaveDuty(waveDuty);
  apu.channelTwo.setPeriod(period);
  apu.channelTwo.setEnvelopePeriod(envelopePeriod);
  apu.channelTwo.setEnvelopeDirection(envelopeDirection);
  apu.channelTwo.setInitialLengthTimer(lengthTimer);
  apu.channelTwo.setLengthEnable(lengthEnable);
  apu.channelTwo.trigger();
}

void configureChannelThree(APU &apu, bool dacEnable, int lengthTimer,
                           int outputLevel, int period, bool lengthEnable,
                           const BYTE wavePattern[16]) {
  apu.channelThree.setDACEnable(dacEnable);
  apu.channelThree.setInitialLengthTimer(lengthTimer);
  apu.channelThree.setOutputLevel(outputLevel);
  apu.channelThree.setPeriod(period);
  apu.channelThree.setLengthEnable(lengthEnable);
  for (int i = 0; i < 16; i++) {
    apu.channelThree.setWavePatternRAM(i, wavePattern[i]);
  }
  apu.channelThree.trigger();
}

void configureChannelFour(APU &apu, int initialVolume, int envelopePeriod,
                          bool envelopeDirection, int lengthTimer,
                          bool lengthEnable, int clockShift, bool lfsrWidth,
                          int clockDivider) {
  apu.channelFour.setInitialVolume(initialVolume);
  apu.channelFour.setEnvelopePeriod(envelopePeriod);
  apu.channelFour.setEnvelopeDirection(envelopeDirection);
  apu.channelFour.setInitialLengthTimer(lengthTimer);
  apu.channelFour.setLengthEnable(lengthEnable);
  apu.channelFour.setClockShift(clockShift);
  apu.channelFour.setLSFRWidth(lfsrWidth);
  apu.channelFour.setClockDivider(clockDivider);
  apu.channelFour.trigger();
}

/*-------------Main-----------------*/

int main() {

  APU apu;
  apu.setAPUStatus(true);

  // Configure channels
  configureChannelOne(apu, 0, 3, 1547, 0, false, 0, false, 7, false, 7);
  configureChannelTwo(apu, 0, 3, 1547, 0, false, 0, false);
  BYTE wavePattern[16] = {0};
  for (int i = 0; i < 16; i++) {
    wavePattern[i] = 0x08;
  }
  configureChannelThree(apu, true, 0, 3, 1547, false, wavePattern);
  configureChannelFour(apu, 0, 7, false, 0, true, 0, true, 7);

  // Main loop
  for (int i = 0; i < 200; i++) {
    cpuUpdate(apu);
  }

  return 0;
}