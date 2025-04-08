#ifndef TIMERS_H
#define TIMERS_H
#include <cstdint>
#include "Interrupts.h"

typedef uint8_t BYTE;
typedef uint16_t WORD;

class Timers {
private:
    BYTE DIV; // Divider Register
    BYTE TIMA; // Timer Counter
    BYTE TMA; // Timer Modulo
    BYTE TAC; // Timer Control

    WORD timaCounter; // 16-bit timer counter
    bool TIMA_Enabled; // Flag to indicate if the timer is enabled
    WORD timaCycles; // Number of cycles for the TIMA register

    WORD divCounter; // 16-bit divider counter

    // TODO: Add interrupt member variable
    Interrupts interrupts; // Interrupts object to handle interrupts
public:
    Timers(Interrupts &interrupts); // Constructor

    void resetTimers(); // Reset the timer registers
    void writeData(WORD address, BYTE value); // Write data to the timer registers
    BYTE readData(WORD address) const; // Read data from the timer registers
    void updateTimers(WORD cycles); // Increment the TIMA register
};

#endif