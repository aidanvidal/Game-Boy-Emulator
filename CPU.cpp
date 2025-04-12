#include "APU/APU.h"
#include "Timers.h"
#include "Interrupts.h"
#include "GPU.h"

/*--------------CPU Simulation-------------*/

int handleInstruction() {
  return 4; // Return the number of cycles for the instruction executed
}

void cpuUpdate(APU &apu, Timers &timers) {
  // CPU Counters
  const int CPUFREQ = 4194304; // CPU frequency in Hz
  const int MAXCYCLES = 69905;
  int cyclesThisUpdate = 0;

  while (cyclesThisUpdate < MAXCYCLES) {
    int cycles = handleInstruction();
    cyclesThisUpdate += cycles;
    /*-----------APU Controls--------------*/
    apu.updateChannelTimers(cycles);
    apu.apuStep(cycles);
    apu.getAudioSample(cycles);

    /*-----------Timer Controls------------*/
    // Update the timers
    timers.updateTimers(cycles);
  }
}

int main(){
    return 0;
}
