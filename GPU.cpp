#include "GPU.h"

GPU::GPU(Interrupts *interrupts)
    : interrupts(interrupts), cycleCount(0), vBlank(false) {
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

  // CGB only registers
  VRAMBank = 0; // Default VRAM bank
}
GPU::~GPU() {
  // Destructor
}

void GPU::writeData(WORD address, BYTE value) {
  // TODO: Handle CGB mode

  // Handle VRAM and OAM writes
  if (address >= 0x8000 && address < 0xA000) {
    VRAM[(address & 0x1FFF) | (VRAMBank << 13)] = value; // Write to VRAM
    return;
  } else if (address >= 0xFE00 && address < 0xFEA0) {
    OAM[address & 0xFF] = value; // Write to OAM
    return;
  }
  // Handle CGB VRAM bank switching
  else if (address == 0xFF4F) {
    VRAMBank = value & 0x01; // Set the VRAM bank
    return;
  }

  // Handle GPU registers
  switch (address) {
  case 0xFF40: // LCDC
    LCDC = value;
    break;
  case 0xFF41: // STAT
    STAT = STAT & 0x7 |
           (value & 0xF8); // Update STAT, keep the lower 3 bits unchanged
    break;
  case 0xFF42: // SCY
    SCY = value;
    break;
  case 0xFF43: // SCX
    SCX = value;
    break;
  case 0xFF44: // LY
    LY = 0;    // Reset LY to 0 on write, it is read-only
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
  default:
    break; // Ignore other writes
  }
}

BYTE GPU::readData(WORD address) const {
  // TODO: Handle CGB mode

  // Handle VRAM and OAM reads
  if (address >= 0x8000 && address < 0xA000) {
    return VRAM[(address & 0x1FFF) | (VRAMBank << 13)]; // Read from VRAM
  } else if (address >= 0xFE00 && address < 0xFEA0) {
    return OAM[address & 0xFF]; // Read from OAM
  }
  // CGB VRAM bank
  else if (address == 0xFF4F) {
    return VRAMBank; // Read the VRAM bank
  }

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
  default:
    return 0; // Ignore other reads
  }
}

void GPU::updateGPU(WORD cycles) {
  cycleCount += cycles;
  while (cycleCount > 0) {
    switch (STAT & 0x03) {
    case 0: // HBlank
      if (cycleCount < 204)
        return;
      cycleCount -= 204;
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
      if (cycleCount < 456)
        return;
      cycleCount -= 456;
      LY++;
      checkLYC();

      if (LY > 153) { // End of VBlank
        LY = 0;
        // Check for OAM interrupt
        if (STAT & 0x02) {
          interrupts->setLCDStatFlag(true);
        }
        STAT = (STAT & 0xFC) | 0x02; // Mode 2
        vBlank = false;
      }
      break;

    case 2: // OAM Search
      if (cycleCount < 80)
        return;
      cycleCount -= 80;
      STAT = (STAT & 0xFC) | 0x03; // Mode 3
      // Find sprites for current line
      findSprites();
      break;

    case 3: // Pixel Transfer
      if (cycleCount < 172)
        return;
      cycleCount -= 172;
      STAT = (STAT & 0xFC) | 0x00; // Mode 0

      // TODO: renderScanline(); // Render LY to lineBuffer

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
    int topY = spriteY - 16;           // Adjust Y coordinate for sprite height
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