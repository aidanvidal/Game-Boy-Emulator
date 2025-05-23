#ifndef GPU_H
#define GPU_H
#include "Interrupts.h"
#include "SDL2/SDL.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>

typedef uint8_t BYTE;
typedef uint16_t WORD;

// Forward declaration
class Memory;

class GPU {
private:
  // TODO:
  // add DMA register for OAM transfer in the future, probably in the memory
  // HDAM registers for VRAM DMA transfer, either in the GPU or in the memory
  // Need WRAM and WRAM bank registers for CGB mode

  Interrupts *interrupts; // Interrupts object to handle interrupts

  // Memory For GPU
  // Link for VRAM info:
  // https://gbdev.io/pandocs/Memory_Map.html#vram-memory-map
  BYTE VRAM[0x4000];        // Video RAM size is 8KB
  BYTE OAM[0xA0];           // Object Attribute Memory size is 160 bytes
  BYTE visiableSprites[10]; // Array to hold visible sprites
  BYTE spriteCount;         // Number of sprites on the current line

  int cycleCount;           // Cycle count for the GPUF
  uint32_t lineBuffer[160]; // Line buffer for the current line being rendered

  BYTE LCDC; // LCD Control
  BYTE LY;   // LCD Y Coordinate
  BYTE LYC;  // LY Compare
  BYTE STAT; // LCD Status

  BYTE SCY; // Scroll Y
  BYTE SCX; // Scroll X

  BYTE WY;        // Window Y Position
  int windowLine; // Current window line
  BYTE WX;        // Window X Position

  BYTE BGP;  // Background Palette Data non CGB
  BYTE OBP0; // Object Palette 0 Data non CGB, gray shades
  BYTE OBP1; // Object Palette 1 Data non CGB, gray shades

  // CGB only registers
  bool CGB;      // Flag to indicate if the GPU is in CGB mode
  BYTE BCPS;     // Background Palette Specification CGB
  BYTE OCPS;     // Object Palette Specification CGB
  BYTE VRAMBank; // VRAM Bank CGB

  uint16_t bgPalettes[8][4];     // 8 BG palettes, 4 colors each (RGB555)
  uint16_t objPalettes[8][4];    // 8 OBJ palettes, 4 colors each (RGB555)
  bool bgPriorties[160];         // Background priorities for each pixel
  uint8_t bgColorIndices[160];   // New buffer for BG color indices (0-3)
  SDL_Surface *backgroundGlobal; // Global background surface

  // HDMA length for VRAM transfer
  BYTE HDMALength; // Length of the transfer
  WORD HDMASource; // Source address for the transfer
  WORD HDMADest;   // Destination address for the transfer
  bool HDMAActive; // Flag to indicate if HDMA is active

  // Helper functions
  void checkLYC();       // Check the LY Compare register
  void findSprites();    // Find sprites for the current line
  void renderScanline(); // Render the current scanline
  void renderBG();       // Render the background for the current line
  void renderWindow();   // Render the window for the current line
  void renderSprites();  // Render the sprites for the current line
  void updateCGBPalette(uint16_t (&palettes)[8][4], BYTE &index_reg, BYTE data);
  uint32_t getDMGColor(uint8_t color_idx, BYTE palette);
  uint32_t cgbToARGB(uint16_t rgb555); // Convert RGB555 to ARGB8888
  void doHDMATransfer();

  int getModeDuration();
  void advanceMode();

public:
  GPU(Interrupts *interrupts, bool CGB = false,
      Memory *memory = nullptr); // Constructor
  ~GPU();
  void writeData(WORD address, BYTE value); // Write data to the GPU registers
  BYTE readData(WORD address) const;        // Read data from the GPU registers
  void updateGPU(int cycles);               // Update the GPU timers
  void renderFrame(SDL_Renderer *ren);      // Render the frame
  void setHDMA(BYTE len, WORD source, WORD dest,
               bool active);                        // Set the HDMA
  BYTE getHDMALength() const { return HDMALength; } // Get the HDMA length
  bool vBlank;    // Flag to indicate if the screen is blank
  Memory *memory; // Memory object to access memory
};

#endif