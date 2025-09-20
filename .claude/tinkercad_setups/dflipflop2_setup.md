# Tinkercad Setup Guide: dflipflop2

## Overview
This guide helps you recreate the circuit for `dflipflop2.ino` in Tinkercad.

## Required Components
- 1x Arduino Uno R3
- 1x Breadboard
- 1x Slide Switch
- 1x Generic Output
- 2x LED
- 2x Resistor

## Pin Usage
Total pins used: 4/20 available

### Input Connections
- **input_switch1** (Slide Switch) → Arduino pin A0

### Output Connections
- **input_switch1** (Generic Output) → Arduino pin A0
- **led22_0** (LED) → Arduino pin A1 (with 220Ω resistor)
- **led24_0** (LED) → Arduino pin A2 (with 220Ω resistor)

### Timing Information
- Clock 1: 1000.0ms interval (1.00 Hz)

## Setup Steps in Tinkercad

1. **Add Arduino Uno R3**
   - Drag Arduino Uno R3 to the workspace

2. **Add Breadboard**
   - Place a breadboard next to the Arduino

3. **Add Input Components**
   - Add Slide Switch for input_switch1
   - Connect to Arduino pin A0
   - Connect other terminal to GND (for buttons/switches)

4. **Add Output Components**
   - Add Generic Output for input_switch1
   - Connect to Arduino pin A0

   - Add LED for led22_0
   - Add 220Ω resistor in series
   - Connect to Arduino pin A1
   - Connect cathode to GND

   - Add LED for led24_0
   - Add 220Ω resistor in series
   - Connect to Arduino pin A2
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
