# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -O2 -g -fno-omit-frame-pointer
INCLUDES = -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib
LIBS = -lSDL2

# Object files directory
BUILD_DIR = build

# APU Component
APU_TARGET = APU_emulator
APU_SRCS = APU/main.cpp APU/APU.cpp APU/channelTwo.cpp APU/channelOne.cpp APU/channelFour.cpp APU/channelThree.cpp
APU_OBJS = $(APU_SRCS:%.cpp=$(BUILD_DIR)/%.o)

# Graphics Component (placeholder for future use)
GRAPHICS_TARGET = graphics
GRAPHICS_SRCS = GPU.cpp Interrupts.cpp testGPU.cpp
GRAPHICS_OBJS = $(GRAPHICS_SRCS:%.cpp=$(BUILD_DIR)/%.o)

# GameBoy Component (placeholder for future use)
GAMEBOY_TARGET = gameboy
GAMEBOY_SRCS = main.cpp CPU.cpp Memory.cpp Interrupts.cpp Input.cpp GPU.cpp Timers.cpp WRAM.cpp APU/APU.cpp APU/channelTwo.cpp APU/channelOne.cpp APU/channelFour.cpp APU/channelThree.cpp Cartridges/Cartridge.cpp Cartridges/NoMBC.cpp Cartridges/MBC1.cpp Cartridges/MBC3.cpp
GAMEBOY_OBJS = $(GAMEBOY_SRCS:%.cpp=$(BUILD_DIR)/%.o)

# Default target
all: $(APU_TARGET)

# APU Target
$(APU_TARGET): $(APU_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

# Graphics Target (placeholder)
$(GRAPHICS_TARGET): $(GRAPHICS_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

# GameBoy Target (placeholder)
$(GAMEBOY_TARGET): $(GAMEBOY_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

# Compile source files
$(BUILD_DIR)/%.o: %.cpp
	mkdir -p $(dir $@) # Create the necessary subdirectory
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean up
clean:
	rm -f $(APU_TARGET) $(GRAPHICS_TARGET) $(GAMEBOY_TARGET)
	rm -rf $(BUILD_DIR)

# Declare phony targets
.PHONY: all clean $(APU_TARGET) $(GRAPHICS_TARGET) $(GAMEBOY_TARGET)