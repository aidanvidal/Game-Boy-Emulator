#ifndef TIMERS_H
#define TIMERS_H
#include <cstdint>

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
public:
    Timers(); // Constructor

    void resetTimers(); // Reset the timer registers
    void writeDIV(); // Write to the DIV register
    void writeTIMA(BYTE value); // Write to the TIMA register
    void writeTMA(BYTE value); // Write to the TMA register
    void writeTAC(BYTE value); // Write to the TAC register

    BYTE readDIV(); // Read from the DIV register
    BYTE readTIMA(); // Read from the TIMA register
    BYTE readTMA(); // Read from the TMA register
    BYTE readTAC(); // Read from the TAC register

    void resetDIV(); // Reset the DIV register
    void resetTIMA(); // Reset the TIMA register
    void resetTMA(); // Reset the TMA register
    void resetTAC(); // Reset the TAC register

    void updateTimers(WORD cycles); // Increment the TIMA register
};

#endif