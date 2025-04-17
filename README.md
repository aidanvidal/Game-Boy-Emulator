# Game Boy Emulator

Hello all, this here is a Game Boy emulator that I am currently working on for MacOS. I am using [Pan Docs](https://gbdev.io/pandocs/Memory_Map.html) for all the hardware information.
By the time this is finished it will hopefully include a fully fleshed out Game Boy Color and Original Game Boy compatibility with sound as well.


## Setup
- Clone the repo
```bash
git clone https://github.com/aidanvidal/Game-Boy-Emulator.git
cd Game-Boy-Emulator
```
- Run A Make Command (WIP)
```bash
make emulator
```
- Then get a ROM and play
``` bash
./emulator filename.rom
```

## TODO:
-   Cartridge things
-   Serial Data Transfer
-   Finish Interrurpts
-   A lot of instructions
-   Combine parts into Memory class
-   Create final cycle accurate Game Boy CPU loop
-   Test and debug

## Extra Info

This is all done in C++ and most major parts of the Game Boy design are sperated into classes, i.e APU, GPU, Memory, Timers, ect...
I think there will be around 8000 lines of code when finished, but around 1/3 of that will just be a whole lot of boring instruction handling code.
