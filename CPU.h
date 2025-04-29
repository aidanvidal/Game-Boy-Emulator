#ifndef CPU_H
#define CPU_H
#include "Memory.h"
#include "stdint.h"

class CPU {
public:
  CPU(Memory &mainMem);
  ~CPU();
  void resetGBNoBios();
  void resetGBBios();
  void resetCGBNoBios();
  void executeOneInstruction();
  // void resetGBWithBios();
  void test();
  // Cycle count constants
  static const int cycleCount[];
  static const int cycleCount_CB[];
  // Function for retrieving last instruciton's cycle count
  int getLastCycleCount();

  // CGB stuff
  bool getDoubleSpeed();

private:
  // Thing
  uint64_t instructCount = 0;
  // Keeps track of last instructions cycle amount
  int lastCycleCount;
  // Memory pointer
  Memory *mainMem;
  // Interrupt flag thing
  bool EIDIFlag = false;
  bool IME = false;
  bool IMEhold = false;
  bool halt = false;
  // General Purpose Registers (besides F)
  uint8_t reg_A = 0;
  uint8_t reg_F = 0;
  uint8_t reg_B = 0;
  uint8_t reg_C = 0;
  uint8_t reg_D = 0;
  uint8_t reg_E = 0;
  uint8_t reg_H = 0;
  uint8_t reg_L = 0;
  // Combo register functions
  uint16_t getAF();
  uint16_t getBC();
  uint16_t getDE();
  uint16_t getHL();
  void setAF(uint16_t data);
  void setBC(uint16_t data);
  void setDE(uint16_t data);
  void setHL(uint16_t data);
  // Special registers
  uint16_t reg_PC;
  uint16_t reg_SP;
  // Private functions
  // Set Flag Register
  void setZ(bool set);
  void setN(bool set);
  void setH(bool set);
  void setC(bool set);
  bool getZ();
  bool getN();
  bool getH();
  bool getC();
  // Flag setter shortcuts
  void setHAdd(uint8_t left, uint8_t right);
  void setCAdd(uint8_t left, uint8_t right);
  void setHSub(uint8_t left, uint8_t right);
  void setCSub(uint8_t left, uint8_t right);
  // With carries
  void setHAdd(uint8_t left, uint8_t right, uint8_t carry);
  void setCAdd(uint8_t left, uint8_t right, uint8_t carry);
  void setHSub(uint8_t left, uint8_t right, uint8_t carry);
  void setCSub(uint8_t left, uint8_t right, uint8_t carry);
  // Decode and Execute
  void decodeExecute(uint8_t instruction);
  void CBPrefixed(uint8_t instruction);
  bool decodeBitResSet(uint8_t instruction);
  // Functions for particular instructions
  void ALU8bitAdd(uint8_t input);
  void ALU8bitAdc(uint8_t input);
  void ALU8bitSub(uint8_t input);
  void ALU8bitSbc(uint8_t input);
  void ALU8bitAnd(uint8_t input);
  void ALU8bitOr(uint8_t input);
  void ALU8bitXor(uint8_t input);
  void ALU8bitCp(uint8_t input);
  uint8_t ALU8bitInc(uint8_t input);
  uint8_t ALU8bitDec(uint8_t input);
  void ALU16bitAdd(uint16_t input);
  uint16_t ALU16bitInc(uint16_t input);
  uint16_t ALU16bitDec(uint16_t input);
  void jp(bool condition);
  void jr(bool condition);
  void call(bool condition);
  void rst(uint8_t n);
  void ret(bool condition);
  uint8_t swap(uint8_t input);
  uint8_t RLC(uint8_t input);
  uint8_t RL(uint8_t input);
  uint8_t RRC(uint8_t input);
  uint8_t RR(uint8_t input);
  uint8_t SLA(uint8_t input);
  uint8_t SRA(uint8_t input);
  uint8_t SRL(uint8_t input);
  void BIT(uint8_t input, int bit);
  uint8_t SET(uint8_t input, int bit);
  uint8_t RES(uint8_t input, int bit);

  int cycleCounter = 0;

  // CGB Stuff
  bool doubleSpeed = false;
};

#endif