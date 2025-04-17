#ifndef INPUT_H
#define INPUT_H
#include <SDL2/SDL.h>
#include <cstdint>

typedef uint8_t BYTE;
typedef uint16_t WORD;

class Input {
private:
  BYTE joypad; // Joypad state

public:
  Input();                           // Constructor
  ~Input();                          // Destructor
  void updateJoypadState(BYTE data); // Update joypad state
  BYTE readJoypadState();            // Read joypad state
};

#endif