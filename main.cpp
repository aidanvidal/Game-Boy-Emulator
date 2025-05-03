// The main file for the GameBoy emulator
// To run the emulator, use the command:
// ./GameBoy <romfile> <screenmultiplier>(optional)
#include "CPU.h"
#include "Memory.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace std;

int screenMultiplier = 4;

int main(int argc, char *argv[]) {
  // Handle arguments
  string romFilePath = "";
  // Launch arguments
  bool launchError = false;
  if (argc > 1) {
    romFilePath = argv[1];
  } else {
    launchError = true;
  }
  if (argc > 2) {
    try {
      screenMultiplier = stoi(argv[2]);
    } catch (invalid_argument e) {
      launchError = true;
    }
  }
  if (launchError) {
    cout << "Usage: " << argv[0] << " romfile screenmultiplier\n";
    exit(-1);
  }

  // Creat obbjects
  Memory mainMem(romFilePath);
  CPU CPU(mainMem);
  // SDL Stuff
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window = 0;
  window = SDL_CreateWindow("GameBoy", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, 160 * screenMultiplier,
                            144 * screenMultiplier, SDL_WINDOW_SHOWN);
  SDL_Renderer *ren = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  SDL_Event events;

  // Check if bootstrap file is present. If it is, load it in, if not, skip the
  // bootstrap.

  if (!mainMem.CBG) {
    CPU.resetGBNoBios();
  } else {
    CPU.resetCGBNoBios();
  }

  std::cout << "Color Mode: " << (mainMem.CBG) << std::endl;
  // Main loop
  bool running = true;
  while (running) {
    // SDL Events check
    SDL_PumpEvents();
    while (SDL_PollEvent(&events)) {
      if (events.type == SDL_QUIT) {
        running = false;
      }
    }
    int cycleCount = 0;
    // Simulate CPU cycles
    while (!mainMem.gpu->vBlank) {
      CPU.executeOneInstruction();
      int lastCycleCount = CPU.getLastCycleCount();
      cycleCount += lastCycleCount;
      if (CPU.getDoubleSpeed()) {
        mainMem.updateCycles(lastCycleCount / 2);
      } else {
        mainMem.updateCycles(lastCycleCount);
      }
      mainMem.updateTimers(lastCycleCount);
    }
    mainMem.gpu->vBlank = false;
    mainMem.renderGPU(ren);
  }

  return 0;
}
