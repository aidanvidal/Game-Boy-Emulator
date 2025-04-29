// #include "stdafx.h"
#include "CPU.h"
// Flag Register constants
#define flagZset 0x80
#define flagNset 0x40
#define flagHset 0x20
#define flagCset 0x10
// Resets
#define flagZreset 0x7F
#define flagNreset 0xBF
#define flagHreset 0xDF
#define flagCreset 0xE0
// Interrupt stuff
#define vBlankByte 0x01
#define LCDCByte 0x02
#define timerOverflowByte 0x04
#define serialCompleteByte 0x08
#define buttonpressByte 0x10
#define IF 0xFF0F
#define IE 0xFFFF

// Constant array of Cycle Counts for each instruciton

const int CPU::cycleCount[] = {
    4,  12, 8, 8,  4,  4,  8,  4,  20, 8,  8, 8,  4,  4,  8,  4,  4,  12, 8,
    8,  4,  4, 8,  4,  12, 8,  8,  8,  4,  4, 8,  4,  8,  12, 8,  8,  4,  4,
    8,  4,  8, 8,  8,  8,  4,  4,  8,  4,  8, 12, 8,  8,  12, 12, 12, 4,  8,
    8,  8,  8, 4,  4,  8,  4,  4,  4,  4,  4, 4,  4,  8,  4,  4,  4,  4,  4,
    4,  4,  8, 4,  4,  4,  4,  4,  4,  4,  8, 4,  4,  4,  4,  4,  4,  4,  8,
    4,  4,  4, 4,  4,  4,  4,  8,  4,  4,  4, 4,  4,  4,  4,  8,  4,  8,  8,
    8,  8,  8, 8,  4,  8,  4,  4,  4,  4,  4, 4,  8,  4,  4,  4,  4,  4,  4,
    4,  8,  4, 4,  4,  4,  4,  4,  4,  8,  4, 4,  4,  4,  4,  4,  4,  8,  4,
    4,  4,  4, 4,  4,  4,  8,  4,  4,  4,  4, 4,  4,  4,  8,  4,  4,  4,  4,
    4,  4,  4, 8,  4,  4,  4,  4,  4,  4,  4, 8,  4,  4,  4,  4,  4,  4,  4,
    8,  4,  8, 12, 12, 16, 12, 16, 8,  16, 8, 16, 12, 4,  12, 24, 8,  16, 8,
    12, 12, 0, 12, 16, 8,  16, 8,  16, 12, 0, 12, 0,  8,  16, 12, 12, 8,  0,
    0,  16, 8, 16, 16, 4,  16, 0,  0,  0,  8, 16, 12, 12, 8,  4,  0,  16, 8,
    16, 12, 8, 16, 4,  0,  0,  8,  16,
};

const int CPU::cycleCount_CB[] = {
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 12, 8,
    8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,
    8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,
    8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
};

CPU::CPU(Memory &mainMem) : mainMem(&mainMem) {
  instructCount = 0;
  EIDIFlag = false;
  IME = false;
  IMEhold = false;
  halt = false;
}

CPU::~CPU() {}

void CPU::resetGBNoBios() {
  reg_A = 0x01;
  reg_F = 0xB0;
  reg_B = 0x00;
  reg_C = 0x13;
  reg_D = 0x00;
  reg_E = 0xD8;
  reg_H = 0x01;
  reg_L = 0x4D;
  reg_PC = 0x100;
  reg_SP = 0xFFFE;
  // Some registers need to be set up

  /*mainMem->writeByte(0xFF10, 0x80);
  mainMem->writeByte(0xFF11, 0xBF);
  mainMem->writeByte(0xFF12, 0xF3);
  mainMem->writeByte(0xFF14, 0xBF);
  mainMem->writeByte(0xFF16, 0x3F);
  mainMem->writeByte(0xFF19, 0xBF);
  mainMem->writeByte(0xFF1A, 0x7F);
  mainMem->writeByte(0xFF1B, 0xFF);
  mainMem->writeByte(0xFF1C, 0x9F);
  mainMem->writeByte(0xFF1E, 0xBF);
  mainMem->writeByte(0xFF20, 0xFF);
  mainMem->writeByte(0xFF23, 0xBF);
  mainMem->writeByte(0xFF24, 0x77);
  mainMem->writeByte(0xFF25, 0xF3);
  mainMem->writeByte(0xFF26, 0xF1);*/
  mainMem->writeByte(0xFF40, 0x91);
  mainMem->writeByte(0xFF47, 0xFC);
  mainMem->writeByte(0xFF48, 0xFF);
  mainMem->writeByte(0xFF49, 0xFF);
}

void CPU::resetGBBios() {
  reg_A = 0x00;
  reg_F = 0x00;
  reg_B = 0x00;
  reg_C = 0x00;
  reg_D = 0x00;
  reg_E = 0x00;
  reg_H = 0x00;
  reg_L = 0x00;
  reg_PC = 0x00;
  reg_SP = 0x00;
}

void CPU::resetCGBNoBios() {
  reg_A = 0x11;
  reg_F = 0x80;
  reg_B = 0x00;
  reg_C = 0x00;
  reg_D = 0xFF;
  reg_E = 0x56;
  reg_H = 0x00;
  reg_L = 0x0D;
  reg_PC = 0x100;
  reg_SP = 0xFFFE;

  mainMem->writeByte(0xFF40, 0x91);
  mainMem->writeByte(0xFF41, 0x81);
  mainMem->writeByte(0xFF40, 0x91);
}

void CPU::executeOneInstruction() {
  if (reg_PC == 0x2377) {
    // printf("break ");
  }
  if (reg_PC == 0x2379) {
    // printf("breakA ");
  }
  // ISR
  // Activate IME next instruction

  if (EIDIFlag) {
    EIDIFlag = false;
  } else {
    IME = IMEhold;
  }
  // Vblank
  if ((mainMem->readByte(IF) & vBlankByte) != 0 &&
      (mainMem->readByte(IE) & vBlankByte) != 0) {
    halt = false;
    if (IME) {
      IMEhold = false;
      IME = false;
      mainMem->writeByte(IF, mainMem->readByte(IF) & ~vBlankByte);
      // cout << "Cycles since last vblank int: " << cycleCounter << "\n";
      cycleCounter = 0;
      bool debug = false;
      rst(0x40);
    }
  }
  // LCDC
  else if ((mainMem->readByte(IF) & LCDCByte) != 0 &&
           (mainMem->readByte(IE) & LCDCByte) != 0) {
    halt = false;
    if (IME) {
      IMEhold = false;
      IME = false;
      mainMem->writeByte(IF, mainMem->readByte(IF) & ~LCDCByte);
      rst(0x48);
    }
  }
  // Timer
  else if ((mainMem->readByte(IF) & timerOverflowByte) != 0 &&
           (mainMem->readByte(IE) & timerOverflowByte) != 0) {
    halt = false;
    if (IME) {
      IMEhold = false;
      IME = false;
      mainMem->writeByte(IF, mainMem->readByte(IF) & ~timerOverflowByte);
      rst(0x50);
    }
  }
  // Serial Transfer
  else if ((mainMem->readByte(IF) & serialCompleteByte) != 0 &&
           (mainMem->readByte(IE) & serialCompleteByte) != 0) {
    halt = false;
    if (IME) {
      IMEhold = false;
      IME = false;
      mainMem->writeByte(IF, mainMem->readByte(IF) & ~serialCompleteByte);
      rst(0x58);
    }
  }
  // Button press
  else if ((mainMem->readByte(IF) & buttonpressByte) != 0 &&
           (mainMem->readByte(IE) & buttonpressByte) != 0) {
    halt = false;
    if (IME) {
      IMEhold = false;
      IME = false;
      mainMem->writeByte(IF, mainMem->readByte(IF) & ~buttonpressByte);
      rst(0x60);
    }
  }

  // Execution
  if (!halt) {
    uint8_t opcode = mainMem->readByte(reg_PC);
    // cout << hex << "PC: 0x" << reg_PC << " SP: 0x" << reg_SP << " Opcode 0x"
    // << int(opcode) << "\n" << dec;
    decodeExecute(opcode);
    instructCount++;
  } else {
    lastCycleCount = 4; // Say 4 cycles passed due to halt
  }
  cycleCounter += getLastCycleCount();
}

void CPU::decodeExecute(uint8_t instruction) {
  // TODO: Keep track of instructions cycle count
  // Set PC to one instruction ahead
  uint16_t oldPC = reg_PC;
  reg_PC += 1;
  // Log cycle count
  lastCycleCount = cycleCount[instruction];
  // Execute the instruction
  switch (instruction) {
  // NOP
  case 0x00:
    break;
  // LD nn,n
  case 0x06:
    reg_B = mainMem->readByte(reg_PC);
    reg_PC++;
    break;
  case 0x0E:
    reg_C = mainMem->readByte(reg_PC);
    reg_PC++;
    break;
  case 0x16:
    reg_D = mainMem->readByte(reg_PC);
    reg_PC++;
    break;
  case 0x1E:
    reg_E = mainMem->readByte(reg_PC);
    reg_PC++;
    break;
  case 0x26:
    reg_H = mainMem->readByte(reg_PC);
    reg_PC++;
    break;
  case 0x2E:
    reg_L = mainMem->readByte(reg_PC);
    reg_PC++;
    break;
  // LD r1,r2
  // B
  case 0x40:
    reg_B = reg_B;
    break;
  case 0x41:
    reg_B = reg_C;
    break;
  case 0x42:
    reg_B = reg_D;
    break;
  case 0x43:
    reg_B = reg_E;
    break;
  case 0x44:
    reg_B = reg_H;
    break;
  case 0x45:
    reg_B = reg_L;
    break;
  case 0x46:
    reg_B = mainMem->readByte(getHL());
    break;
  // C
  case 0x48:
    reg_C = reg_B;
    break;
  case 0x49:
    reg_C = reg_C;
    break;
  case 0x4A:
    reg_C = reg_D;
    break;
  case 0x4B:
    reg_C = reg_E;
    break;
  case 0x4C:
    reg_C = reg_H;
    break;
  case 0x4D:
    reg_C = reg_L;
    break;
  case 0x4E:
    reg_C = mainMem->readByte(getHL());
    break;
  // D
  case 0x50:
    reg_D = reg_B;
    break;
  case 0x51:
    reg_D = reg_C;
    break;
  case 0x52:
    reg_D = reg_D;
    break;
  case 0x53:
    reg_D = reg_E;
    break;
  case 0x54:
    reg_D = reg_H;
    break;
  case 0x55:
    reg_D = reg_L;
    break;
  case 0x56:
    reg_D = mainMem->readByte(getHL());
    break;
  // E
  case 0x58:
    reg_E = reg_B;
    break;
  case 0x59:
    reg_E = reg_C;
    break;
  case 0x5A:
    reg_E = reg_D;
    break;
  case 0x5B:
    reg_E = reg_E;
    break;
  case 0x5C:
    reg_E = reg_H;
    break;
  case 0x5D:
    reg_E = reg_L;
    break;
  case 0x5E:
    reg_E = mainMem->readByte(getHL());
    break;
  // H
  case 0x60:
    reg_H = reg_B;
    break;
  case 0x61:
    reg_H = reg_C;
    break;
  case 0x62:
    reg_H = reg_D;
    break;
  case 0x63:
    reg_H = reg_E;
    break;
  case 0x64:
    reg_H = reg_H;
    break;
  case 0x65:
    reg_H = reg_L;
    break;
  case 0x66:
    reg_H = mainMem->readByte(getHL());
    break;
  // L
  case 0x68:
    reg_L = reg_B;
    break;
  case 0x69:
    reg_L = reg_C;
    break;
  case 0x6A:
    reg_L = reg_D;
    break;
  case 0x6B:
    reg_L = reg_E;
    break;
  case 0x6C:
    reg_L = reg_H;
    break;
  case 0x6D:
    reg_L = reg_L;
    break;
  case 0x6E:
    reg_L = mainMem->readByte(getHL());
    break;
  // (HL)
  case 0x70:
    mainMem->writeByte(getHL(), reg_B);
    break;
  case 0x71:
    mainMem->writeByte(getHL(), reg_C);
    break;
  case 0x72:
    mainMem->writeByte(getHL(), reg_D);
    break;
  case 0x73:
    mainMem->writeByte(getHL(), reg_E);
    break;
  case 0x74:
    mainMem->writeByte(getHL(), reg_H);
    break;
  case 0x75:
    mainMem->writeByte(getHL(), reg_L);
    break;
  case 0x36:
    mainMem->writeByte(getHL(), mainMem->readByte(reg_PC));
    reg_PC++;
    break;
  // LD A,n
  case 0x7F:
    reg_A = reg_A;
    break;
  case 0x78:
    reg_A = reg_B;
    break;
  case 0x79:
    reg_A = reg_C;
    break;
  case 0x7A:
    reg_A = reg_D;
    break;
  case 0x7B:
    reg_A = reg_E;
    break;
  case 0x7C:
    reg_A = reg_H;
    break;
  case 0x7D:
    reg_A = reg_L;
    break;
  case 0x7E:
    reg_A = mainMem->readByte(getHL());
    break;
  case 0x0A:
    reg_A = mainMem->readByte(getBC());
    break;
  case 0x1A:
    reg_A = mainMem->readByte(getDE());
    break;
  case 0xFA:
    reg_A = mainMem->readByte(mainMem->readWord(reg_PC));
    reg_PC += 2;
    break;
  case 0x3E:
    reg_A = mainMem->readByte(reg_PC);
    reg_PC++;
    break;
  // LD n,A
  case 0x47:
    reg_B = reg_A;
    break;
  case 0x4F:
    reg_C = reg_A;
    break;
  case 0x57:
    reg_D = reg_A;
    break;
  case 0x5F:
    reg_E = reg_A;
    break;
  case 0x67:
    reg_H = reg_A;
    break;
  case 0x6F:
    reg_L = reg_A;
    break;
  case 0x02:
    mainMem->writeByte(getBC(), reg_A);
    break;
  case 0x12:
    mainMem->writeByte(getDE(), reg_A);
    break;
  case 0x77:
    mainMem->writeByte(getHL(), reg_A);
    break;
  case 0xEA:
    mainMem->writeByte(mainMem->readWord(reg_PC), reg_A);
    reg_PC += 2;
    break;
  // LD A,(C)
  case 0xF2:
    reg_A = mainMem->readByte(0xFF00 + reg_C);
    break;
  // LD (C), A
  case 0xE2:
    mainMem->writeByte(0xFF00 + reg_C, reg_A);
    break;
  // LDD A,(HL)
  case 0x3A:
    reg_A = mainMem->readByte(getHL());
    setHL(getHL() - 1);
    break;
  // LDD (HL),A
  case 0x32:
    mainMem->writeByte(getHL(), reg_A);
    setHL(getHL() - 1);
    break;
  // LDI A,(HL)
  case 0x2A:
    reg_A = mainMem->readByte(getHL());
    setHL(getHL() + 1);
    break;
  // LDI (HL),A
  case 0x22:
    mainMem->writeByte(getHL(), reg_A);
    setHL(getHL() + 1);
    break;
  // LDH (n),A
  case 0xE0:
    mainMem->writeByte(0xFF00 + mainMem->readByte(reg_PC), reg_A);
    reg_PC++;
    break;
  // LDH A,(n)
  case 0xF0:
    reg_A = mainMem->readByte(0xFF00 + mainMem->readByte(reg_PC));
    reg_PC++;
    break;
  // LD n,nn
  case 0x01:
    setBC(mainMem->readWord(reg_PC));
    reg_PC += 2;
    break;
  case 0x11:
    setDE(mainMem->readWord(reg_PC));
    reg_PC += 2;
    break;
  case 0x21:
    setHL(mainMem->readWord(reg_PC));
    reg_PC += 2;
    break;
  case 0x31:
    reg_SP = mainMem->readWord(reg_PC);
    reg_PC += 2;
    break;
  // LD SP,HL
  case 0xF9:
    reg_SP = getHL();
    break;
  // LDHL SP,n
  case 0xF8: {
    int8_t n = mainMem->readByte(reg_PC);

    setZ(false);
    setN(false);
    setHAdd((uint8_t)reg_SP, n);
    setCAdd((uint8_t)reg_SP, n);

    setHL(reg_SP + n);
    reg_PC++;
    break;
  }
  // LD (nn),sp
  case 0x08:
    mainMem->writeWord(mainMem->readWord(reg_PC), reg_SP);
    reg_PC += 2;
    break;
  // PUSH nn
  case 0xF5:
    reg_SP -= 2;
    mainMem->writeWord(reg_SP, getAF());
    break;
  case 0xC5:
    reg_SP -= 2;
    mainMem->writeWord(reg_SP, getBC());
    break;
  case 0xD5:
    reg_SP -= 2;
    mainMem->writeWord(reg_SP, getDE());
    break;
  case 0xE5:
    reg_SP -= 2;
    mainMem->writeWord(reg_SP, getHL());
    break;
  // POP nn
  case 0xF1:
    setAF(mainMem->readWord(reg_SP));
    reg_SP += 2;
    break;
  case 0xC1:
    setBC(mainMem->readWord(reg_SP));
    reg_SP += 2;
    break;
  case 0xD1:
    setDE(mainMem->readWord(reg_SP));
    reg_SP += 2;
    break;
  case 0xE1:
    setHL(mainMem->readWord(reg_SP));
    reg_SP += 2;
    break;
  // ADD A,n
  case 0x87:
    ALU8bitAdd(reg_A);
    break;
  case 0x80:
    ALU8bitAdd(reg_B);
    break;
  case 0x81:
    ALU8bitAdd(reg_C);
    break;
  case 0x82:
    ALU8bitAdd(reg_D);
    break;
  case 0x83:
    ALU8bitAdd(reg_E);
    break;
  case 0x84:
    ALU8bitAdd(reg_H);
    break;
  case 0x85:
    ALU8bitAdd(reg_L);
    break;
  case 0x86:
    ALU8bitAdd(mainMem->readByte(getHL()));
    break;
  case 0xC6:
    ALU8bitAdd(mainMem->readByte(reg_PC));
    reg_PC++;
    break;
  // ADC A,n
  case 0x8F:
    ALU8bitAdc(reg_A);
    break;
  case 0x88:
    ALU8bitAdc(reg_B);
    break;
  case 0x89:
    ALU8bitAdc(reg_C);
    break;
  case 0x8A:
    ALU8bitAdc(reg_D);
    break;
  case 0x8B:
    ALU8bitAdc(reg_E);
    break;
  case 0x8C:
    ALU8bitAdc(reg_H);
    break;
  case 0x8D:
    ALU8bitAdc(reg_L);
    break;
  case 0x8E:
    ALU8bitAdc(mainMem->readByte(getHL()));
    break;
  case 0xCE:
    ALU8bitAdc(mainMem->readByte(reg_PC));
    reg_PC++;
    break;
  // SUB n
  case 0x97:
    ALU8bitSub(reg_A);
    break;
  case 0x90:
    ALU8bitSub(reg_B);
    break;
  case 0x91:
    ALU8bitSub(reg_C);
    break;
  case 0x92:
    ALU8bitSub(reg_D);
    break;
  case 0x93:
    ALU8bitSub(reg_E);
    break;
  case 0x94:
    ALU8bitSub(reg_H);
    break;
  case 0x95:
    ALU8bitSub(reg_L);
    break;
  case 0x96:
    ALU8bitSub(mainMem->readByte(getHL()));
    break;
  case 0xD6:
    ALU8bitSub(mainMem->readByte(reg_PC));
    reg_PC++;
    break;
  // SBC A,n
  case 0x9F:
    ALU8bitSbc(reg_A);
    break;
  case 0x98:
    ALU8bitSbc(reg_B);
    break;
  case 0x99:
    ALU8bitSbc(reg_C);
    break;
  case 0x9A:
    ALU8bitSbc(reg_D);
    break;
  case 0x9B:
    ALU8bitSbc(reg_E);
    break;
  case 0x9C:
    ALU8bitSbc(reg_H);
    break;
  case 0x9D:
    ALU8bitSbc(reg_L);
    break;
  case 0x9E:
    ALU8bitSbc(mainMem->readByte(getHL()));
    break;
  case 0xDE:
    ALU8bitSbc(mainMem->readByte(reg_PC));
    reg_PC++;
    break;
  // AND n
  case 0xA7:
    ALU8bitAnd(reg_A);
    break;
  case 0xA0:
    ALU8bitAnd(reg_B);
    break;
  case 0xA1:
    ALU8bitAnd(reg_C);
    break;
  case 0xA2:
    ALU8bitAnd(reg_D);
    break;
  case 0xA3:
    ALU8bitAnd(reg_E);
    break;
  case 0xA4:
    ALU8bitAnd(reg_H);
    break;
  case 0xA5:
    ALU8bitAnd(reg_L);
    break;
  case 0xA6:
    ALU8bitAnd(mainMem->readByte(getHL()));
    break;
  case 0xE6:
    ALU8bitAnd(mainMem->readByte(reg_PC));
    reg_PC++;
    break;
  // OR n
  case 0xB7:
    ALU8bitOr(reg_A);
    break;
  case 0xB0:
    ALU8bitOr(reg_B);
    break;
  case 0xB1:
    ALU8bitOr(reg_C);
    break;
  case 0xB2:
    ALU8bitOr(reg_D);
    break;
  case 0xB3:
    ALU8bitOr(reg_E);
    break;
  case 0xB4:
    ALU8bitOr(reg_H);
    break;
  case 0xB5:
    ALU8bitOr(reg_L);
    break;
  case 0xB6:
    ALU8bitOr(mainMem->readByte(getHL()));
    break;
  case 0xF6:
    ALU8bitOr(mainMem->readByte(reg_PC));
    reg_PC++;
    break;
  // XOR n
  case 0xAF:
    ALU8bitXor(reg_A);
    break;
  case 0xA8:
    ALU8bitXor(reg_B);
    break;
  case 0xA9:
    ALU8bitXor(reg_C);
    break;
  case 0xAA:
    ALU8bitXor(reg_D);
    break;
  case 0xAB:
    ALU8bitXor(reg_E);
    break;
  case 0xAC:
    ALU8bitXor(reg_H);
    break;
  case 0xAD:
    ALU8bitXor(reg_L);
    break;
  case 0xAE:
    ALU8bitXor(mainMem->readByte(getHL()));
    break;
  case 0xEE:
    ALU8bitXor(mainMem->readByte(reg_PC));
    reg_PC++;
    break;
  // CP n
  case 0xBF:
    ALU8bitCp(reg_A);
    break;
  case 0xB8:
    ALU8bitCp(reg_B);
    break;
  case 0xB9:
    ALU8bitCp(reg_C);
    break;
  case 0xBA:
    ALU8bitCp(reg_D);
    break;
  case 0xBB:
    ALU8bitCp(reg_E);
    break;
  case 0xBC:
    ALU8bitCp(reg_H);
    break;
  case 0xBD:
    ALU8bitCp(reg_L);
    break;
  case 0xBE:
    ALU8bitCp(mainMem->readByte(getHL()));
    break;
  case 0xFE:
    ALU8bitCp(mainMem->readByte(reg_PC));
    reg_PC++;
    break;
  // INC n
  case 0x3C:
    reg_A = ALU8bitInc(reg_A);
    break;
  case 0x04:
    reg_B = ALU8bitInc(reg_B);
    break;
  case 0x0C:
    reg_C = ALU8bitInc(reg_C);
    break;
  case 0x14:
    reg_D = ALU8bitInc(reg_D);
    break;
  case 0x1C:
    reg_E = ALU8bitInc(reg_E);
    break;
  case 0x24:
    reg_H = ALU8bitInc(reg_H);
    break;
  case 0x2C:
    reg_L = ALU8bitInc(reg_L);
    break;
  case 0x34:
    mainMem->writeByte(getHL(), ALU8bitInc(mainMem->readByte(getHL())));
    break;
  // DEC n
  case 0x3D:
    reg_A = ALU8bitDec(reg_A);
    break;
  case 0x05:
    reg_B = ALU8bitDec(reg_B);
    break;
  case 0x0D:
    reg_C = ALU8bitDec(reg_C);
    break;
  case 0x15:
    reg_D = ALU8bitDec(reg_D);
    break;
  case 0x1D:
    reg_E = ALU8bitDec(reg_E);
    break;
  case 0x25:
    reg_H = ALU8bitDec(reg_H);
    break;
  case 0x2D:
    reg_L = ALU8bitDec(reg_L);
    break;
  case 0x35:
    mainMem->writeByte(getHL(), ALU8bitDec(mainMem->readByte(getHL())));
    break;
  // ADD HL,n
  case 0x09:
    ALU16bitAdd(getBC());
    break;
  case 0x19:
    ALU16bitAdd(getDE());
    break;
  case 0x29:
    ALU16bitAdd(getHL());
    break;
  case 0x39:
    ALU16bitAdd(reg_SP);
    break;
  // ADD SP,n
  case 0xE8: {
    int8_t n = mainMem->readByte(reg_PC);

    setZ(false);
    setN(false);
    setHAdd((uint8_t)reg_SP, n);
    setCAdd((uint8_t)reg_SP, n);

    reg_SP += n;

    reg_PC++;
    break;
  }
  // INC nn
  case 0x03:
    setBC(getBC() + 1);
    break;
  case 0x13:
    setDE(getDE() + 1);
    break;
  case 0x23:
    setHL(getHL() + 1);
    break;
  case 0x33:
    reg_SP++;
    break;
  // DEC nn
  case 0x0B:
    setBC(getBC() - 1);
    break;
  case 0x1B:
    setDE(getDE() - 1);
    break;
  case 0x2B:
    setHL(getHL() - 1);
    break;
  case 0x3B:
    reg_SP--;
    break;
  // DAA
  case 0x27: {
    if (getN()) {
      if (getH()) {
        reg_A += 0xFA;
      }
      if (getC()) {
        reg_A += 0xA0;
      }
    } else {
      uint16_t tempReg_A = reg_A;
      if ((reg_A & 0xf) > 0x9 || getH()) {
        tempReg_A += 0x6;
      }
      if ((tempReg_A & 0x1f0) > 0x90 || getC()) {
        tempReg_A += 0x60;
        setC(true);
      } else {
        setC(false);
      }
      reg_A = (uint8_t)tempReg_A;
    }
    setH(false);
    setZ(reg_A == 0);
    break;
  }
  // CPL
  case 0x2F:
    reg_A = ~reg_A;
    setN(true);
    setH(true);
    break;
  // CCF
  case 0x3F:
    setC(!getC());
    setN(false);
    setH(false);
    break;
  // SCF
  case 0x37:
    setN(false);
    setH(false);
    setC(true);
    break;
  // HALT
  case 0x76:
    // cout << "Halt\n";
    halt = true;
    if (!IME) {
      // reg_PC++;
    }
    break;
  // STOP
  case 0x10: {
    // TODO: Implement this
    // CGB Double Speed Mode set
    uint8_t key1 = mainMem->readByte(0xFF4D);
    if ((key1 & 0x1) == 0x1) {
      doubleSpeed = !doubleSpeed;
    }
    // Clear low bit, report double speed mode on bit 7
    mainMem->writeByte(0xFF4D, (key1 & 0x7E) | ((doubleSpeed ? 1 : 0) << 7));
    reg_PC++;
    break;
  }
  // DI
  case 0xF3:
    IMEhold = false;
    EIDIFlag = true;
    break;
  // EI
  case 0xFB:
    IMEhold = true;
    EIDIFlag = true;
    break;
  // RLCA
  case 0x07: {
    bool carry = (reg_A & 0x80) != 0;
    setN(false);
    setH(false);
    setC(carry);
    reg_A = (reg_A << 1) | (uint8_t)carry;
    setZ(false);
    break;
  }
  // RLA
  case 0x17: {
    bool carryFlagBit = getC();
    bool carry = (reg_A & 0x80) != 0;

    setZ(false);
    setN(false);
    setH(false);
    setC(carry);
    reg_A = (reg_A << 1) | (uint8_t)carryFlagBit;
    break;
  }
  // RRCA
  case 0x0F: {
    bool carry = (reg_A & 0x1) != 0;

    setZ(false);
    setN(false);
    setH(false);
    setC(carry);
    reg_A = (reg_A >> 1) | (carry << 7);
    break;
  }
  // RRA
  case 0x1F: {
    bool carry = (reg_A & 0x1) != 0;
    bool carryFlagBit = getC();

    setZ(false);
    setN(false);
    setH(false);
    setC(carry);
    reg_A = (reg_A >> 1) | (carryFlagBit << 7);
    break;
  }
  // JP nn
  case 0xC3:
    jp(true);
    break;
  // JP cc,nn
  case 0xC2:
    jp(!getZ());
    break;
  case 0xCA:
    jp(getZ());
    break;
  case 0xD2:
    jp(!getC());
    break;
  case 0xDA:
    jp(getC());
    break;
  // JP (HL);
  case 0xE9:
    reg_PC = getHL();
    break;
  // JR n
  case 0x18:
    jr(true);
    break;
  // JR cc,nn
  case 0x20:
    jr(!getZ());
    break;
  case 0x28:
    jr(getZ());
    break;
  case 0x30:
    jr(!getC());
    break;
  case 0x38:
    jr(getC());
    break;
  // Call nn
  case 0xCD:
    call(true);
    break;
  // Call cc,nn
  case 0xC4:
    call(!getZ());
    break;
  case 0xCC:
    call(getZ());
    break;
  case 0xD4:
    call(!getC());
    break;
  case 0xDC:
    call(getC());
    break;
  // RST n
  case 0xC7:
    rst(0x00);
    break;
  case 0xCF:
    rst(0x08);
    break;
  case 0xD7:
    rst(0x10);
    break;
  case 0xDF:
    rst(0x18);
    break;
  case 0xE7:
    rst(0x20);
    break;
  case 0xEF:
    rst(0x28);
    break;
  case 0xF7:
    rst(0x30);
    break;
  case 0xFF:
    rst(0x38);
    break;
  // RET
  case 0xC9:
    ret(true);
    // Weird special case where non-conditional ret is always 16, not 20
    lastCycleCount = 16;
    break;
  // RET CC
  case 0xC0:
    ret(!getZ());
    break;
  case 0xC8:
    ret(getZ());
    break;
  case 0xD0:
    ret(!getC());
    break;
  case 0xD8:
    ret(getC());
    break;
  // RETI
  case 0xD9:
    ret(true);
    IME = true;
    IMEhold = true;
    // Weird special case where non-conditional ret is always 16, not 20
    lastCycleCount = 16;
    break;
  // CB Pre-fixed instructions
  case 0xCB:
    CBPrefixed(mainMem->readByte(reg_PC));
    break;

  default:
    printf("Illegal or Unimplmented opcode 0x%X at 0x%X", instruction,
           reg_PC - 1);
    while (true)
      ;
    break; // Why did this happen?
  }
  if (reg_PC < 0xC000) {
    // printf("break ");
  }
}

void CPU::CBPrefixed(uint8_t instruction) {
  reg_PC++;
  // Keep track of cycle count
  lastCycleCount = cycleCount_CB[instruction];
  switch (instruction) {
  // RLC n
  case 0x00:
    reg_B = RLC(reg_B);
    break;
  case 0x01:
    reg_C = RLC(reg_C);
    break;
  case 0x02:
    reg_D = RLC(reg_D);
    break;
  case 0x03:
    reg_E = RLC(reg_E);
    break;
  case 0x04:
    reg_H = RLC(reg_H);
    break;
  case 0x05:
    reg_L = RLC(reg_L);
    break;
  case 0x06:
    mainMem->writeByte(getHL(), RLC(mainMem->readByte(getHL())));
    break;
  case 0x07:
    reg_A = RLC(reg_A);
    break;
  // RRC
  case 0x08:
    reg_B = RRC(reg_B);
    break;
  case 0x09:
    reg_C = RRC(reg_C);
    break;
  case 0x0A:
    reg_D = RRC(reg_D);
    break;
  case 0x0B:
    reg_E = RRC(reg_E);
    break;
  case 0x0C:
    reg_H = RRC(reg_H);
    break;
  case 0x0D:
    reg_L = RRC(reg_L);
    break;
  case 0x0E:
    mainMem->writeByte(getHL(), RRC(mainMem->readByte(getHL())));
    break;
  case 0x0F:
    reg_A = RRC(reg_A);
    break;
  // RL n
  case 0x10:
    reg_B = RL(reg_B);
    break;
  case 0x11:
    reg_C = RL(reg_C);
    break;
  case 0x12:
    reg_D = RL(reg_D);
    break;
  case 0x13:
    reg_E = RL(reg_E);
    break;
  case 0x14:
    reg_H = RL(reg_H);
    break;
  case 0x15:
    reg_L = RL(reg_L);
    break;
  case 0x16:
    mainMem->writeByte(getHL(), RL(mainMem->readByte(getHL())));
    break;
  case 0x17:
    reg_A = RL(reg_A);
    break;
  // RR n
  case 0x18:
    reg_B = RR(reg_B);
    break;
  case 0x19:
    reg_C = RR(reg_C);
    break;
  case 0x1A:
    reg_D = RR(reg_D);
    break;
  case 0x1B:
    reg_E = RR(reg_E);
    break;
  case 0x1C:
    reg_H = RR(reg_H);
    break;
  case 0x1D:
    reg_L = RR(reg_L);
    break;
  case 0x1E:
    mainMem->writeByte(getHL(), RR(mainMem->readByte(getHL())));
    break;
  case 0x1F:
    reg_A = RR(reg_A);
    break;
  // SLA n
  case 0x20:
    reg_B = SLA(reg_B);
    break;
  case 0x21:
    reg_C = SLA(reg_C);
    break;
  case 0x22:
    reg_D = SLA(reg_D);
    break;
  case 0x23:
    reg_E = SLA(reg_E);
    break;
  case 0x24:
    reg_H = SLA(reg_H);
    break;
  case 0x25:
    reg_L = SLA(reg_L);
    break;
  case 0x26:
    mainMem->writeByte(getHL(), SLA(mainMem->readByte(getHL())));
    break;
  case 0x27:
    reg_A = SLA(reg_A);
    break;
  // SRA n
  case 0x28:
    reg_B = SRA(reg_B);
    break;
  case 0x29:
    reg_C = SRA(reg_C);
    break;
  case 0x2A:
    reg_D = SRA(reg_D);
    break;
  case 0x2B:
    reg_E = SRA(reg_E);
    break;
  case 0x2C:
    reg_H = SRA(reg_H);
    break;
  case 0x2D:
    reg_L = SRA(reg_L);
    break;
  case 0x2E:
    mainMem->writeByte(getHL(), SRA(mainMem->readByte(getHL())));
    break;
  case 0x2F:
    reg_A = SRA(reg_A);
    break;
  // SWAP n
  case 0x30:
    reg_B = swap(reg_B);
    break;
  case 0x31:
    reg_C = swap(reg_C);
    break;
  case 0x32:
    reg_D = swap(reg_D);
    break;
  case 0x33:
    reg_E = swap(reg_E);
    break;
  case 0x34:
    reg_H = swap(reg_H);
    break;
  case 0x35:
    reg_L = swap(reg_L);
    break;
  case 0x36:
    mainMem->writeByte(getHL(), swap(mainMem->readByte(getHL())));
    break;
  case 0x37:
    reg_A = swap(reg_A);
    break;
  // SRL n
  case 0x38:
    reg_B = SRL(reg_B);
    break;
  case 0x39:
    reg_C = SRL(reg_C);
    break;
  case 0x3A:
    reg_D = SRL(reg_D);
    break;
  case 0x3B:
    reg_E = SRL(reg_E);
    break;
  case 0x3C:
    reg_H = SRL(reg_H);
    break;
  case 0x3D:
    reg_L = SRL(reg_L);
    break;
  case 0x3E:
    mainMem->writeByte(getHL(), SRL(mainMem->readByte(getHL())));
    break;
  case 0x3F:
    reg_A = SRL(reg_A);
    break;

  default:
    // Use a special method of handling the BIT,RES, and SET
    if (!decodeBitResSet(instruction)) {
      // If that failed for some odd reason
      printf("Illegal or Unimplmented opcode CB 0x%X at 0x%X", instruction,
             reg_PC - 2);
      while (true)
        ;
    }
    break;
  }
}

int CPU::getLastCycleCount() { return lastCycleCount; }

bool CPU::decodeBitResSet(uint8_t instruction) {
  if (!(instruction >= 0x40 && instruction <= 0xFF)) {
    return false; // Something weird happened
  }
  // BIT operations parse
  if (instruction >= 0x40 && instruction <= 0x7F) {
    int bitNum = (((instruction >> 4) - 0x4) * 2) + ((instruction & 0xF) >> 3);
    switch (instruction & 0x07) {
    case 0x00:
      BIT(reg_B, bitNum);
      break;
    case 0x01:
      BIT(reg_C, bitNum);
      break;
    case 0x02:
      BIT(reg_D, bitNum);
      break;
    case 0x03:
      BIT(reg_E, bitNum);
      break;
    case 0x04:
      BIT(reg_H, bitNum);
      break;
    case 0x05:
      BIT(reg_L, bitNum);
      break;
    case 0x06:
      BIT(mainMem->readByte(getHL()), bitNum);
      break;
    case 0x07:
      BIT(reg_A, bitNum);
      break;
    default:
      return false;
      break;
    }
  }
  // RES operations
  if (instruction >= 0x80 && instruction <= 0xBF) {
    int bitNum = (((instruction >> 4) - 0x8) * 2) + ((instruction & 0xF) >> 3);
    switch (instruction & 0x07) {
    case 0x00:
      reg_B = RES(reg_B, bitNum);
      break;
    case 0x01:
      reg_C = RES(reg_C, bitNum);
      break;
    case 0x02:
      reg_D = RES(reg_D, bitNum);
      break;
    case 0x03:
      reg_E = RES(reg_E, bitNum);
      break;
    case 0x04:
      reg_H = RES(reg_H, bitNum);
      break;
    case 0x05:
      reg_L = RES(reg_L, bitNum);
      break;
    case 0x06:
      mainMem->writeByte(getHL(), RES(mainMem->readByte(getHL()), bitNum));
      break;
    case 0x07:
      reg_A = RES(reg_A, bitNum);
      break;
    default:
      return false;
      break;
    }
  }
  // SET operations
  if (instruction >= 0xC0 && instruction <= 0xFF) {
    int bitNum = (((instruction >> 4) - 0xC) * 2) + ((instruction & 0xF) >> 3);
    switch (instruction & 0x07) {
    case 0x00:
      reg_B = SET(reg_B, bitNum);
      break;
    case 0x01:
      reg_C = SET(reg_C, bitNum);
      break;
    case 0x02:
      reg_D = SET(reg_D, bitNum);
      break;
    case 0x03:
      reg_E = SET(reg_E, bitNum);
      break;
    case 0x04:
      reg_H = SET(reg_H, bitNum);
      break;
    case 0x05:
      reg_L = SET(reg_L, bitNum);
      break;
    case 0x06:
      mainMem->writeByte(getHL(), SET(mainMem->readByte(getHL()), bitNum));
      break;
    case 0x07:
      reg_A = SET(reg_A, bitNum);
      break;
    default:
      return false;
      break;
    }
  }
  return true;
}

// Combo register stuff
// Get
uint16_t CPU::getAF() {
  uint16_t returnval;
  returnval = reg_F;
  returnval |= reg_A << 8;
  return returnval;
}
uint16_t CPU::getBC() {
  uint16_t returnval;
  returnval = reg_C;
  returnval |= reg_B << 8;
  return returnval;
}
uint16_t CPU::getDE() {
  uint16_t returnval;
  returnval = reg_E;
  returnval |= reg_D << 8;
  return returnval;
}
uint16_t CPU::getHL() {
  uint16_t returnval;
  returnval = reg_L;
  returnval |= reg_H << 8;
  return returnval;
}
// Set
void CPU::setAF(uint16_t data) {
  reg_F = data & 0xF0; // Lower 4 can't be set
  reg_A = (data >> 8) & 0xFF;
}
void CPU::setBC(uint16_t data) {
  reg_C = data & 0xFF;
  reg_B = (data >> 8) & 0xFF;
}
void CPU::setDE(uint16_t data) {
  reg_E = data & 0xFF;
  reg_D = (data >> 8) & 0xFF;
}
void CPU::setHL(uint16_t data) {
  reg_L = data & 0xFF;
  reg_H = (data >> 8) & 0xFF;
}
// Flag register manipulating
void CPU::setZ(bool set) {
  if (set) {
    reg_F |= flagZset;
  } else {
    reg_F &= flagZreset;
  }
}
void CPU::setN(bool set) {
  if (set) {
    reg_F |= flagNset;
  } else {
    reg_F &= flagNreset;
  }
}
void CPU::setH(bool set) {
  if (set) {
    reg_F |= flagHset;
  } else {
    reg_F &= flagHreset;
  }
}
void CPU::setC(bool set) {
  if (set) {
    reg_F |= flagCset;
  } else {
    reg_F &= flagCreset;
  }
}
bool CPU::getZ() { return (reg_F & flagZset) != 0; }
bool CPU::getN() { return (reg_F & flagNset) != 0; }
bool CPU::getH() { return (reg_F & flagHset) != 0; }
bool CPU::getC() { return (reg_F & flagCset) != 0; }

// Flag setting shortcuts
void CPU::setHAdd(uint8_t left, uint8_t right) {
  uint8_t result = (left & 0xF) + (right & 0xF);
  setH(result > 0xF);
}

void CPU::setCAdd(uint8_t left, uint8_t right) {
  uint16_t result = left + right;
  setC(result > 0xff);
}
// With carry
void CPU::setHAdd(uint8_t left, uint8_t right, uint8_t carry) {
  uint8_t result = (left & 0xF) + (right & 0xF) + carry;
  setH(result > 0xF);
}

void CPU::setCAdd(uint8_t left, uint8_t right, uint8_t carry) {
  uint16_t result = left + right + carry;
  setC(result > 0xff);
}

// TODO: Make sure I'm doing this right
void CPU::setHSub(uint8_t left, uint8_t right) {
  setH((left & 0xf) < (right & 0xf));
}

void CPU::setCSub(uint8_t left, uint8_t right) { setC(left < right); }
// With carry
void CPU::setHSub(uint8_t left, uint8_t right, uint8_t carry) {
  setH((left & 0xf) < ((right & 0xf) + carry));
}

void CPU::setCSub(uint8_t left, uint8_t right, uint8_t carry) {
  setC(left < (right + carry));
}

// Instruction functions
void CPU::ALU8bitAdd(uint8_t input) {
  setN(false);
  setHAdd(reg_A, input);
  setCAdd(reg_A, input);

  reg_A += input;

  setZ(reg_A == 0);
}
void CPU::ALU8bitAdc(uint8_t input) {
  uint8_t carry = getC();
  setN(false);
  setHAdd(reg_A, input, carry);
  setCAdd(reg_A, input, carry);

  reg_A += input + carry;

  setZ(reg_A == 0);
}

void CPU::ALU8bitSub(uint8_t input) {
  setZ(reg_A == input);
  setN(true);
  setHSub(reg_A, input);
  setCSub(reg_A, input);

  reg_A -= input;
}
void CPU::ALU8bitSbc(uint8_t input) {
  uint8_t carry = getC();
  setN(true);
  setHSub(reg_A, input, carry);
  setCSub(reg_A, input, carry);

  reg_A -= (input + carry);

  setZ(reg_A == 0);
}

void CPU::ALU8bitAnd(uint8_t input) {
  uint8_t result = reg_A & input;
  setZ(result == 0);
  setN(false);
  setH(true);
  setC(false);
  reg_A = result;
}

void CPU::ALU8bitOr(uint8_t input) {
  uint8_t result = reg_A | input;
  setZ(result == 0);
  setN(false);
  setH(false);
  setC(false);
  reg_A = result;
}

void CPU::ALU8bitXor(uint8_t input) {
  uint8_t result = reg_A ^ input;
  setZ(result == 0);
  setN(false);
  setH(false);
  setC(false);
  reg_A = result;
}

void CPU::ALU8bitCp(uint8_t input) {
  setZ(reg_A == input);
  setN(true);
  setHSub(reg_A, input);
  setCSub(reg_A, input);
}

uint8_t CPU::ALU8bitInc(uint8_t input) {
  uint8_t result = input + 1;
  setZ(result == 0);
  setN(false);
  setHAdd(input, 1);
  return result;
}

uint8_t CPU::ALU8bitDec(uint8_t input) {
  uint8_t result = input - 1;
  setZ(result == 0);
  setN(true);
  setHSub(input, 1);
  return result;
}

void CPU::ALU16bitAdd(uint16_t input) {
  uint32_t bitresult = getHL() + input;
  setC(bitresult > 0xffff);
  setN(false);
  setH((((getHL() & 0x0FFF) + (input & 0x0FFF)) & 0xF000) != 0);

  setHL(getHL() + input);
}
// Might not be necessary
uint16_t CPU::ALU16bitInc(uint16_t input) { return uint16_t(); }

uint16_t CPU::ALU16bitDec(uint16_t input) { return uint16_t(); }

void CPU::jp(bool condition) {
  if (condition) {
    lastCycleCount = 16; // 16 if true
    reg_PC = mainMem->readWord(reg_PC);
  } else {
    reg_PC += 2;
  }
}

void CPU::jr(bool condition) {
  if (condition) {
    lastCycleCount = 12; // 12 cycles if it happens
    reg_PC = reg_PC + (int8_t)mainMem->readByte(reg_PC);
    reg_PC++; // I don't get why this is necessary
  } else {
    reg_PC++;
  }
}
void CPU::call(bool condition) {
  if (condition) {
    reg_SP -= 2;
    mainMem->writeWord(reg_SP, reg_PC + 2);
    jp(true);
    lastCycleCount = 24; // 24 cycles if true
  } else {
    reg_PC += 2;
  }
}
void CPU::rst(uint8_t n) {
  reg_SP -= 2;
  mainMem->writeWord(reg_SP, reg_PC);
  reg_PC = n;
}

void CPU::ret(bool condition) {
  if (condition) {
    lastCycleCount = 20;
    reg_PC = mainMem->readWord(reg_SP);
    reg_SP += 2;
  }
}
// CB Prefix functions
uint8_t CPU::swap(uint8_t input) {
  uint8_t result = (input & 0xf0) >> 4;
  result |= input << 4;
  setZ(result == 0);
  setN(false);
  setH(false);
  setC(false);
  return result;
}

uint8_t CPU::RLC(uint8_t input) {
  uint8_t result = input;
  bool carry = (result & 0x80) != 0;

  result = (result << 1) | (uint8_t)carry;

  setZ(result == 0);
  setN(false);
  setH(false);
  setC(carry);

  return result;
}

uint8_t CPU::RL(uint8_t input) {
  uint8_t result = input;
  bool carryFlagBit = getC();
  bool carry = (result & 0x80) != 0;

  result = (result << 1) | (uint8_t)carryFlagBit;

  setZ(result == 0);
  setN(false);
  setH(false);
  setC(carry);

  return result;
}

uint8_t CPU::RRC(uint8_t input) {
  uint8_t result = input;
  bool carry = (result & 0x1) != 0;

  result = (result >> 1) | (carry << 7);

  setZ(result == 0);
  setN(false);
  setH(false);
  setC(carry);

  return result;
}

uint8_t CPU::RR(uint8_t input) {
  uint8_t result = input;
  bool carry = (result & 0x1) != 0;
  bool carryFlagBit = getC();

  result = (result >> 1) | (carryFlagBit << 7);

  setZ(result == 0);
  setN(false);
  setH(false);
  setC(carry);

  return result;
}

uint8_t CPU::SLA(uint8_t input) {
  uint8_t result = input;
  bool carry = (result & 0x80) != 0;

  result = result << 1;

  setZ(result == 0);
  setN(false);
  setH(false);
  setC(carry);

  return result;
}

uint8_t CPU::SRA(uint8_t input) {
  int8_t result = input; // It's an arithmitc shift, so have it be signed
  bool carry = (result & 0x01) != 0;

  result = result >> 1;

  setZ(result == 0);
  setN(false);
  setH(false);
  setC(carry);

  return (uint8_t)result;
}

uint8_t CPU::SRL(uint8_t input) {
  uint8_t result = input; // This one's logical
  bool carry = (result & 0x01) != 0;

  result = result >> 1;

  setZ(result == 0);
  setN(false);
  setH(false);
  setC(carry);

  return (uint8_t)result;
}

void CPU::BIT(uint8_t input, int bit) {
  bool result = (input & (1 << bit)) == 0;
  setZ(result);
  setN(false);
  setH(true);
}

uint8_t CPU::SET(uint8_t input, int bit) {
  uint8_t setBit = 1 << bit;
  uint8_t result = input | setBit;
  return result;
}

uint8_t CPU::RES(uint8_t input, int bit) {
  uint8_t resetBit = ~(1 << bit);
  uint8_t result = input & resetBit;
  return result;
}

bool CPU::getDoubleSpeed() { return doubleSpeed; }

void CPU::test() {
  mainMem->writeByteNoProtect(0x100, 0xCB);
  mainMem->writeByteNoProtect(0x101, 0x56);
  executeOneInstruction();
}