# Tinkercad Setup Guide: jkflipflop

## Overview
This guide helps you recreate the circuit for `jkflipflop.ino` in Tinkercad.

## Target Arduino Board
**Arduino UNO R3/R4**
- Pin Usage: 6/18 pins
- Standard Arduino board with 20 I/O pins (14 digital + 6 analog)

## Required Components
- 1x Arduino Uno R3
- 1x Breadboard
- 4x Slide Switch
- 4x Generic Output
- 2x LED
- 2x Resistor

## Pin Usage
Total pins used: 6/18

### Input Connections
- **input_switch1__clear** (Slide Switch) → Arduino pin A0
- **input_switch2__preset** (Slide Switch) → Arduino pin A1
- **input_switch3_j** (Slide Switch) → Arduino pin A2
- **input_switch4_k** (Slide Switch) → Arduino pin A3

### Output Connections
- **input_switch1__clear** (Generic Output) → Arduino pin A0
- **input_switch2__preset** (Generic Output) → Arduino pin A1
- **input_switch3_j** (Generic Output) → Arduino pin A2
- **input_switch4_k** (Generic Output) → Arduino pin A3
- **led19_q_0** (LED) → Arduino pin A4 (with 220Ω resistor)
- **led20_q_0** (LED) → Arduino pin A5 (with 220Ω resistor)

### Timing Information
- Clock 1: 1000.0ms interval (1.00 Hz)

## Setup Steps in Tinkercad

1. **Add Arduino UNO R3/R4**
   - Drag Arduino UNO R3/R4 to the workspace

2. **Add Breadboard**
   - Place a breadboard next to the Arduino

3. **Add Input Components**
   - Add Slide Switch for input_switch1__clear
   - Connect to Arduino pin A0
   - Connect other terminal to GND (for buttons/switches)

   - Add Slide Switch for input_switch2__preset
   - Connect to Arduino pin A1
   - Connect other terminal to GND (for buttons/switches)

   - Add Slide Switch for input_switch3_j
   - Connect to Arduino pin A2
   - Connect other terminal to GND (for buttons/switches)

   - Add Slide Switch for input_switch4_k
   - Connect to Arduino pin A3
   - Connect other terminal to GND (for buttons/switches)

4. **Add Output Components**
   - Add Generic Output for input_switch1__clear
   - Connect to Arduino pin A0

   - Add Generic Output for input_switch2__preset
   - Connect to Arduino pin A1

   - Add Generic Output for input_switch3_j
   - Connect to Arduino pin A2

   - Add Generic Output for input_switch4_k
   - Connect to Arduino pin A3

   - Add LED for led19_q_0
   - Add 220Ω resistor in series
   - Connect to Arduino pin A4
   - Connect cathode to GND

   - Add LED for led20_q_0
   - Add 220Ω resistor in series
   - Connect to Arduino pin A5
   - Connect cathode to GND

5. **Upload Code**
   - Copy the contents of the .ino file
   - Paste into Tinkercad's code editor
   - Click "Start Simulation"

## Notes
- Ensure all connections are secure
- Check pin assignments match the code
- Use appropriate resistor values for LEDs
- Test inputs work correctly before running simulation
