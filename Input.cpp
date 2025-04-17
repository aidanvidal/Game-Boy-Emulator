#include "Input.h"

Input::Input() {
  joypad = 0xFF; // Initialize joypad state to all buttons released
}

Input::~Input() {
  // Destructor
}

void Input::updateJoypadState(BYTE data) { joypad = data; }

BYTE Input::readJoypadState() {
  const BYTE *keys = SDL_GetKeyboardState(NULL);
  BYTE state = 0;
  // Check if the d pad mode is selected, 0 means it is selected
  if ((joypad & 0x30) == 0x20) {
    // Up
    state |= (keys[SDL_SCANCODE_UP] ? 0 : 0x01) << 2;
    // Down
    state |= (keys[SDL_SCANCODE_DOWN] ? 0 : 0x01) << 3;
    // Left
    state |= (keys[SDL_SCANCODE_LEFT] ? 0 : 0x01) << 1;
    // Right
    state |= (keys[SDL_SCANCODE_RIGHT] ? 0 : 0x01);
  } else if ((joypad & 0x30) == 0x10) {
    // These are the mappings for the buttons I chose to use
    // Game Boy     Comupter
    // A            Z
    // B            X
    // Select       Enter
    // Start        Space

    // A
    state |= (keys[SDL_SCANCODE_Z] ? 0 : 0x01);
    // B
    state |= (keys[SDL_SCANCODE_X] ? 0 : 0x01) << 1;
    // Select
    state |= (keys[SDL_SCANCODE_RETURN] ? 0 : 0x01) << 2;
    // Start
    state |= (keys[SDL_SCANCODE_SPACE] ? 0 : 0x01) << 3;
  }
  joypad &= 0xF0;       // Clear the lower nibble
  joypad |= state;      // Set the new state
  return joypad | 0xC0; // Return the state with the upper nibble set to 1
}