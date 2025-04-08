#ifndef INTERUPTS_H
#define INTERUPTS_H
#include <cstdint>

typedef uint8_t BYTE;
typedef uint16_t WORD;

class Interrupts {
private:
  // IF & IE bit breakdown
  // 0: V-Blank
  // 1: LCD STAT
  // 2: Timer
  // 3: Serial
  // 4: Joypad
  BYTE IE;  // Interrupt Enable Register
  BYTE IF;  // Interrupt Flag Register
  bool IME; // Interrupt Master Enable
public:
  Interrupts();             // Constructor
  void resetInterrupts();   // Reset the interrupt registers
  void writeIE(BYTE value); // Write to the IE register
  void writeIF(BYTE value); // Write to the IF register
  BYTE readIE() const;      // Read from the IE register
  BYTE readIF() const;      // Read from the IF register
  void setIME(bool enable); // Set the IME flag
  bool getIME() const;      // Get the IME flag
  void handleInterrupts();  // Handle interrupts

  // Write to specific interrupt flags
  void setVBlankFlag(bool value);  // V-Blank
  void setLCDStatFlag(bool value); // LCD STAT
  void setTimerFlag(bool value);   // Timer
  void setSerialFlag(bool value);  // Serial
  void setJoypadFlag(bool value);  // Joypad
  // Read specific interrupt flags
  BYTE readVBlankFlag() const;  // V-Blank
  BYTE readLCDStatFlag() const; // LCD STAT
  BYTE readTimerFlag() const;   // Timer
  BYTE readSerialFlag() const;  // Serial
  BYTE readJoypadFlag() const;  // Joypad
};

#endif