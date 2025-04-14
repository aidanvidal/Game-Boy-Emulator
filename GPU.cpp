#include "GPU.h"
#include <cstdint>
#include <iostream>

GPU::GPU(Interrupts *interrupts, bool CGB)
    : interrupts(interrupts), cycleCount(0), CGB(CGB), vBlank(false) {
  // Initialize GPU registers
  LCDC = 0x91;
  LY = 0x00;
  LYC = 0x00;
  STAT = 0x00;
  SCY = 0x00;
  SCX = 0x00;
  WY = 0x00;
  WX = 0x00;
  BGP = 0xFC;  // Default background palette
  OBP0 = 0xFF; // Default object palette 0
  OBP1 = 0xFF; // Default object palette 1

  windowLine = 0;  // Initialize window line
  spriteCount = 0; // Initialize sprite count

  // CGB only registers
  VRAMBank = 0; // Default VRAM bank
  // Initialize CGB palettes to white (RGB555 = 0x7FFF)
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 4; j++) {
      bgPalettes[i][j] = 0x7FFF;
      objPalettes[i][j] = 0x7FFF;
    }
  }
  BCPS = 0x00; // Background Palette Specification
  OCPS = 0x00; // Object Palette Specification
  backgroundGlobal = SDL_CreateRGBSurface(0, 160, 144, 32, 0, 0, 0, 0);
}
GPU::~GPU() {
  // Destructor
}

void GPU::writeData(WORD address, BYTE value) {
  // Handle VRAM writes
  if (address >= 0x8000 && address < 0xA000) {
    // Get the offset in the current VRAM bank
    WORD offset = address & 0x1FFF;

    // Apply VRAM bank selection for CGB mode
    if (CGB && VRAMBank == 1) {
      offset += 0x2000; // Add 8KB offset for bank 1
    }

    VRAM[offset] = value;
    return;
  }
  // Handle OAM writes
  else if (address >= 0xFE00 && address < 0xFEA0) {
    OAM[address & 0xFF] = value;
    return;
  }

  // Handle GPU registers
  switch (address) {
  case 0xFF40: // LCDC
    LCDC = value;
    break;
  case 0xFF41:                             // STAT
    STAT = (STAT & 0x07) | (value & 0xF8); // Keep lower 3 bits
    break;
  case 0xFF42: // SCY
    SCY = value;
    break;
  case 0xFF43: // SCX
    SCX = value;
    break;
  case 0xFF44: // LY - Read only, write resets
    LY = 0;
    windowLine = 0;
    break;
  case 0xFF45: // LYC
    LYC = value;
    break;
  case 0xFF47: // BGP
    BGP = value;
    break;
  case 0xFF48: // OBP0
    OBP0 = value;
    break;
  case 0xFF49: // OBP1
    OBP1 = value;
    break;
  case 0xFF4A: // WY
    WY = value;
    break;
  case 0xFF4B: // WX
    WX = value;
    break;
  case 0xFF4F: // VRAM Bank (CGB only)
    VRAMBank = value & 0x01;
    break;
  case 0xFF68: // BCPS (Background Palette Index)
    BCPS = value;
    break;
  case 0xFF69: // BCPD (Background Palette Data)
    updateCGBPalette(bgPalettes, BCPS, value);
    break;
  case 0xFF6A: // OCPS (Object Palette Index)
    OCPS = value;
    break;
  case 0xFF6B: // OCPD (Object Palette Data)
    updateCGBPalette(objPalettes, OCPS, value);
    break;
  }
}

BYTE GPU::readData(WORD address) const {
  // Handle VRAM reads
  if (address >= 0x8000 && address < 0xA000) {
    // Get the offset in the current VRAM bank
    WORD offset = address & 0x1FFF;

    // Apply VRAM bank selection for CGB mode
    if (CGB && VRAMBank == 1) {
      offset += 0x2000; // Add 8KB offset for bank 1
    }

    return VRAM[offset];
  }
  // Handle OAM reads
  else if (address >= 0xFE00 && address < 0xFEA0) {
    return OAM[address & 0xFF];
  }

  // Handle GPU registers
  switch (address) {
  case 0xFF40: // LCDC
    return LCDC;
  case 0xFF41: // STAT
    return STAT;
  case 0xFF42: // SCY
    return SCY;
  case 0xFF43: // SCX
    return SCX;
  case 0xFF44: // LY
    return LY;
  case 0xFF45: // LYC
    return LYC;
  case 0xFF47: // BGP
    return BGP;
  case 0xFF48: // OBP0
    return OBP0;
  case 0xFF49: // OBP1
    return OBP1;
  case 0xFF4A: // WY
    return WY;
  case 0xFF4B: // WX
    return WX;
  case 0xFF4F: // VRAM Bank (CGB only)
    return VRAMBank;
  case 0xFF68: // BCPS
    return BCPS;
  case 0xFF69: // BCPD
    return bgPalettes[(BCPS & 0x3F) / 8][(BCPS & 0x3F) % 8 / 2];
  case 0xFF6A: // OCPS
    return OCPS;
  case 0xFF6B: // OCPD
    return objPalettes[(OCPS & 0x3F) / 8][(OCPS & 0x3F) % 8 / 2];
  default:
    return 0xFF;
  }
}

void GPU::updateGPU(int cycles) {
  // TODO: Check if the cycle logic is correct when testing
  cycleCount += cycles;
  while (cycleCount > 0) {
    switch (STAT & 0x03) {
    case 0: // HBlank
      if (cycleCount < 204 * 4)
        return;
      cycleCount -= 204 * 4;
      LY++;
      checkLYC();

      if (LY == 144) {               // Enter VBlank
        STAT = (STAT & 0xFC) | 0x01; // Mode 1
        // Check for VBlank Stat interrupt
        if (STAT & 0x01) {
          interrupts->setLCDStatFlag(true);
        }
        // Send VBlank interrupt
        interrupts->setVBlankFlag(true);
        vBlank = true;
      } else {
        STAT = (STAT & 0xFC) | 0x02; // Mode 2
        // Check for OAM interrupt
        if (STAT & 0x02) {
          interrupts->setLCDStatFlag(true);
        }
      }
      break;

    case 1: // VBlank
      if (cycleCount < 456 * 4)
        return;
      cycleCount -= 456 * 4;
      LY++;
      checkLYC();

      if (LY > 153) { // End of VBlank
        LY = 0;
        windowLine = 0; // Reset window line
        // Check for OAM interrupt
        if (STAT & 0x02) {
          interrupts->setLCDStatFlag(true);
        }
        STAT = (STAT & 0xFC) | 0x02; // Mode 2
        vBlank = false;
      }
      break;

    case 2: // OAM Search
      if (cycleCount < 80 * 4)
        return;
      cycleCount -= 80 * 4;
      STAT = (STAT & 0xFC) | 0x03; // Mode 3
      // Find sprites for current line
      findSprites();
      break;

    case 3: // Pixel Transfer
      if (cycleCount < 172 * 4)
        return;
      cycleCount -= 172 * 4;
      STAT = (STAT & 0xFC) | 0x00; // Mode 0

      renderScanline(); // Render LY to lineBuffer

      // Check for HBlank interrupt
      if (STAT & 0x08) {
        interrupts->setLCDStatFlag(true);
      }
      break;
    }
  }
}

void GPU::checkLYC() {
  if (LY == LYC) {
    STAT |= 0x04; // Set the Coincidence Flag
    if (STAT & 0x40) {
      interrupts->setLCDStatFlag(true); // Set the LYC=LY Interrupt
    }
  } else {
    STAT &= ~0x04; // Clear the Coincidence Flag
  }
}

void GPU::updateCGBPalette(uint16_t (&palettes)[8][4], BYTE &index_reg,
                           BYTE data) {
  bool auto_increment = index_reg & 0x80;
  uint8_t index = index_reg & 0x3F; // Lower 6 bits

  // Determine palette and color index
  uint8_t palette_idx = index / 8;
  uint8_t color_idx = (index % 8) / 2;
  bool is_high_byte = (index % 2) == 1;

  if (is_high_byte) {
    palettes[palette_idx][color_idx] =
        (palettes[palette_idx][color_idx] & 0x00FF) | (data << 8);
  } else {
    palettes[palette_idx][color_idx] =
        (palettes[palette_idx][color_idx] & 0xFF00) | data;
  }

  if (auto_increment) {
    index_reg = (index_reg & 0x80) | ((index + 1) & 0x3F);
  }
}

uint32_t GPU::cgbToARGB(uint16_t rgb555) {
  uint8_t r = (rgb555 & 0x1F) << 3; // 5 bits → 8 bits
  uint8_t g = ((rgb555 >> 5) & 0x1F) << 3;
  uint8_t b = ((rgb555 >> 10) & 0x1F) << 3;
  return 0xFF000000 | (r << 16) | (g << 8) | b;
}

uint32_t GPU::getDMGColor(uint8_t color_idx, BYTE palette) {
  // Extract color from palette (BGP, OBP0, OBP1)
  uint8_t shade = (palette >> (color_idx * 2)) & 0x03;
  switch (shade) { // Map to ARGB8888
  case 0:
    return 0xFFFFFFFF; // White
  case 1:
    return 0xFFAAAAAA; // Light gray
  case 2:
    return 0xFF555555; // Dark gray
  case 3:
    return 0xFF000000; // Black
  }
  return 0xFF000000; // Fallback
}

void GPU::findSprites() {
  spriteCount = 0;                // Reset sprite count
  bool largeSprite = LCDC & 0x04; // Check if large sprites are enabled

  for (int i = 0; i < 40; i++) {
    BYTE spriteY = OAM[i * 4]; // Y coordinate
    if (spriteY == 0 || spriteY >= 160 || (spriteY <= 8 && !largeSprite)) {
      continue; // Skip if the sprite is not on the screen or Y ≤ 8 for 8×8
                // sprites
    }
    uint8_t spriteHeight = largeSprite ? 16 : 8; // Determine sprite height
    // Adjust Y coordinate for sprite height
    // Minus 16 since the sprite Y is offseted by 16
    int topY = spriteY - 16;
    int bottomY = topY + spriteHeight; // Calculate bottom Y coordinate
    // Check if the sprite is within the current line
    if (LY >= topY && LY < bottomY) {
      visiableSprites[spriteCount] = i; // Store the sprite index
      spriteCount++;
      if (spriteCount >= 10) {
        break; // Limit to 10 sprites per line
      }
    }
  }
}

void GPU::renderFrame(SDL_Renderer *ren) {
  // Gameboy screen: 160x144
  SDL_Surface *mainSurface = SDL_CreateRGBSurface(0, 160, 144, 32, 0, 0, 0, 0);

  // Copy background surface to the main surface
  SDL_BlitSurface(backgroundGlobal, NULL, mainSurface, NULL);

  // Render main surface
  SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, mainSurface);
  SDL_FreeSurface(mainSurface);
  SDL_RenderClear(ren);
  SDL_RenderCopy(ren, tex, NULL, NULL);
  SDL_RenderPresent(ren);

  SDL_DestroyTexture(tex);
}

void GPU::renderScanline() {
  // Check if the LCD is enabled
  if (!(LCDC & 0x80)) {
    memset(lineBuffer, 0xFF, sizeof(lineBuffer)); // Clear the line buffer
    return;
  }
  // Render the background line
  if (LCDC & 0x01) {
    renderBG();
  }
  // Render the window line
  if (LCDC & 0x20 && LCDC & 0x01) {
    renderWindow();
  }
  // Render sprites on the line
  if (LCDC & 0x02) {
    renderSprites();
  }

  // Declare BG pixels pointer/array
  uint32_t *globalBGPixels = (uint32_t *)backgroundGlobal->pixels;

  for (int i = 0; i < 160; i++) {
    int pixelData = lineBuffer[i];
    // Write the pixel data to the global background surface
    globalBGPixels[i + (LY * backgroundGlobal->w)] = pixelData;
    lineBuffer[i] = 0;
  }
}

void GPU::renderBG() {
  int x = SCX;              // X scroll
  int y = (SCY + LY) % 256; // Y scroll (wrap around)
  for (int i = 0; i < 160; i++, x++) {
    if (x >= 256) {
      x = 0; // Wrap around X scroll
    }
    // Calculate tile index
    // Each tile is 8x8 pixels, so we divide the coordinates by 8
    int tileIndex = ((y / 8) * 32) + (x / 8);
    WORD mapLocation =
        ((LCDC & 0x08) ? 0x1C00 : 0x1800) + tileIndex; // Map location
    BYTE tileLocation = VRAM[mapLocation];             // Tile map content
    BYTE mapAtrribute = VRAM[0x2000 | mapLocation];    // Map attribute content
    // 0x8000 method
    if (LCDC & 0x10) {
      tileLocation <<= 4; // Shift since each tile is 16 bytes
    }
    // 0x8800 method
    else {
      // Proper signed conversion
      int8_t signedTile = static_cast<int8_t>(tileLocation);
      tileLocation = static_cast<BYTE>(signedTile);
      tileLocation = 0x800 + (tileLocation << 4);
    }

    // Get pixels
    // Modulo 8 to get the pixel coordinates within the tile
    int pixelX = (x % 8); // Pixel X coordinate
    int pixelY = (y % 8); // Pixel Y coordinate

    // Game Boy Color map attributes
    if (CGB) {
      if (mapAtrribute & 0x20) {
        // Flip X
        pixelX = 7 - pixelX;
      }
      if (mapAtrribute & 0x40) {
        // Flip Y
        pixelY = 7 - pixelY;
      }
      if (mapAtrribute & 0x08) {
        // Fetch from VRAM bank 1
        tileLocation |= 0x2000;
      }
    }

    // Get the pixel data
    // The pixel data is stored in two bytes, each byte contains 8 pixels
    // The first byte contains the lower bits and the second byte contains the
    // Fetch the lower byte of the pixel data for the current row
    BYTE lowerByte = VRAM[tileLocation + (pixelY * 2)];
    // Fetch the upper byte of the pixel data for the current row
    BYTE upperByte = VRAM[tileLocation + (pixelY * 2) + 1];
    // Extract the pixel data for the current column (pixelX)
    int pixelData = ((lowerByte >> (7 - pixelX)) & 0x01) |
                    (((upperByte >> (7 - pixelX)) & 0x01) << 1);
    // In Game Boy color background tiles have a priority bit
    if (CGB) {
      bgPriorties[i] = (mapAtrribute & 0x80); // True if the tile has priority
      lineBuffer[i] = cgbToARGB(bgPalettes[mapAtrribute & 0x07][pixelData]);
    } else {
      lineBuffer[i] = getDMGColor(pixelData, BGP);
    }
  }
}

void GPU::renderWindow() {
  int x = WX - 7; // Window X position (subtract 7 for the window offset)
  int y = WY;     // Window Y position

  // Check if the window on the current line
  if (LY < y || x >= 160) {
    return; // Window is not visible
  }

  y = windowLine;
  windowLine++; // Increment window line

  for (int i = 0; i < 160; i++, x++) {
    if (x >= 256) {
      x = 0; // Wrap around X scroll
    }
    int tileIndex = ((y / 8) * 32) + (x / 8); // Calculate tile index
    WORD mapLocation =
        ((LCDC & 0x40) ? 0x1C00 : 0x1800) + tileIndex; // Map location
    BYTE tileLocation = VRAM[mapLocation];             // Tile map content
    BYTE mapAtrribute = VRAM[0x2000 | mapLocation];    // Map attribute content
    // 0x8000 method
    if (LCDC & 0x10) {
      tileLocation <<= 4; // Shift since each tile is 16 bytes
    }
    // 0x8800 method
    else {
      // Proper signed conversion
      int8_t signedTile = static_cast<int8_t>(tileLocation);
      tileLocation = static_cast<BYTE>(signedTile);
      tileLocation = 0x800 + (tileLocation << 4);
    }

    // Get pixels
    int pixelX = (x % 8); // Pixel X coordinate
    int pixelY = (y % 8); // Pixel Y coordinate

    // Game Boy Color map attributes
    if (CGB) {
      if (mapAtrribute & 0x20) {
        // Flip X
        pixelX = 7 - pixelX;
      }
      if (mapAtrribute & 0x40) {
        // Flip Y
        pixelY = 7 - pixelY;
      }
      if (mapAtrribute & 0x08) {
        // Fetch from VRAM bank 1
        tileLocation |= 0x2000;
      }
    }

    // Get the pixel data
    BYTE lowerByte = VRAM[tileLocation + (pixelY * 2)];
    BYTE upperByte = VRAM[tileLocation + (pixelY * 2) + 1];
    int pixelData = ((lowerByte >> (7 - pixelX)) & 0x01) |
                    (((upperByte >> (7 - pixelX)) & 0x01) << 1);
    if (CGB) {
      bgPriorties[i] = (mapAtrribute & 0x80); // True if the tile has priority
      lineBuffer[i] = cgbToARGB(
          bgPalettes[mapAtrribute & 0x07][pixelData]); // Apply the palette
    } else {
      lineBuffer[i] = getDMGColor(pixelData, BGP);
    }
  }
}

void GPU::renderSprites() {
  BYTE spriteHeight =
      (LCDC & 0x04) ? 16 : 8; // Check if large sprites are enabled

  // Sort visible sprites based on priority
  std::sort(visiableSprites, visiableSprites + spriteCount,
            [this](int a, int b) {
              BYTE xA = OAM[a * 4 + 1]; // X coordinate of sprite A
              BYTE xB = OAM[b * 4 + 1]; // X coordinate of sprite B
              if (CGB) {
                // In CGB mode, priority is determined by OAM order
                return a < b;
              } else {
                // In non-CGB mode, smaller X has higher priority; if equal,
                // earlier OAM index wins
                return (xA < xB) || (xA == xB && a < b);
              }
            });

  // Render sprites
  for (int i = 0; i < spriteCount; i++) {
    BYTE spriteIndex = visiableSprites[i];
    BYTE spriteY = OAM[spriteIndex * 4];        // Y coordinate
    BYTE spriteX = OAM[spriteIndex * 4 + 1];    // X coordinate
    BYTE tileIndex = OAM[spriteIndex * 4 + 2];  // Tile index
    BYTE attributes = OAM[spriteIndex * 4 + 3]; // Attributes

    BYTE pixelY = (attributes & 0x40) ? (spriteHeight - 1) - (LY - spriteY)
                                      : LY - spriteY; // Flip Y if needed

    // Re-adjust the tile index for tall sprites
    if (spriteHeight == 16) {
      // Modulo 16 to get where the pixel is in the tile
      if (pixelY % 16 < 8) {
        tileIndex &= 0xFE;
      } else {
        tileIndex |= 0x1;
      }
    }

    // Calculate the tile pointer
    // Each tile is 16 bytes, so we multiply the tile index by 16
    uint16_t tilePointer = tileIndex << 4;

    // VRAM bank
    if (CGB && (attributes & 0x8)) {
      tilePointer |= 0x2000;
    }

    BYTE lowerByte = VRAM[tilePointer + (2 * (pixelY % 8))];
    BYTE upperByte = VRAM[tilePointer + (2 * (pixelY % 8)) + 1];

    // Render the sprite pixels
    for (int x = 0; x < 8; x++) {
      int pixelX = (attributes & 0x20) ? (7 - x) : x; // Flip X if needed
      int screenX = spriteX - 8 + pixelX;

      if (screenX < 0 || screenX >= 160) {
        continue; // Skip pixels outside the screen
      }

      int pixelData = ((lowerByte >> (7 - x)) & 0x01) |
                      (((upperByte >> (7 - x)) & 0x01) << 1);

      if (pixelData == 0) {
        continue; // Skip transparent pixels
      }

      // Handle priority and overwrite conditions
      if (CGB) {
        // In CGB mode, skip if background has priority in the sprite attributes
        // or if the background has priority in the map attributes
        if (bgPriorties[screenX] || (attributes & 0x80)) {
          continue;
        }
      } else {
        // In non-CGB mode, skip if background has priority in sprite atributes
        if (attributes & 0x80) {
          continue;
        }
      }

      // Apply the palette
      if (CGB) {
        lineBuffer[screenX] =
            cgbToARGB(objPalettes[attributes & 0x07][pixelData]);
      } else {
        lineBuffer[screenX] =
            getDMGColor(pixelData, (attributes & 0x10) ? OBP1 : OBP0);
      }
    }
  }
}