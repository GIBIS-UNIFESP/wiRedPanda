# Tinkercad Setup Guide: sequential

## Overview
This guide helps you recreate the circuit for `sequential.ino` in Tinkercad.

## Required Components
- 1x Arduino Uno R3
- 1x Breadboard
- 1x Pushbutton
- 1x Generic Output
- 5x LED
- 5x Resistor

## Pin Usage
Total pins used: 7/20 available

### Input Connections
- **push_button1_reset** (Pushbutton) → Arduino pin A0

### Output Connections
- **push_button1_reset** (Generic Output) → Arduino pin A0
- **led34_load_shift_0** (LED) → Arduino pin A1 (with 220Ω resistor)
- **led35_l3_0** (LED) → Arduino pin A2 (with 220Ω resistor)
- **led36_l2_0** (LED) → Arduino pin A3 (with 220Ω resistor)
- **led37_l0_0** (LED) → Arduino pin A4 (with 220Ω resistor)
- **led38_l1_0** (LED) → Arduino pin A5 (with 220Ω resistor)

### Timing Information
- Clock 1: 500.0ms interval (2.00 Hz)
- Clock 2: 62.5ms interval (16.00 Hz)

## Setup Steps in Tinkercad

1. **Add Arduino Uno R3**
   - Drag Arduino Uno R3 to the workspace

2. **Add Breadboard**
   - Place a breadboard next to the Arduino

3. **Add Input Components**
   - Add Pushbutton for push_button1_reset
   - Connect to Arduino pin A0
   - Connect other terminal to GND (for buttons/switches)

4. **Add Output Components**
   - Add Generic Output for push_button1_reset
   - Connect to Arduino pin A0

   - Add LED for led34_load_shift_0
   - Add 220Ω resistor in series
   - Connect to Arduino pin A1
   - Connect cathode to GND

   - Add LED for led35_l3_0
   - Add 220Ω resistor in series
   - Connect to Arduino pin A2
   - Connect cathode to GND

   - Add LED for led36_l2_0
   - Add 220Ω resistor in series
   - Connect to Arduino pin A3
   - Connect cathode to GND

   - Add LED for led37_l0_0
   - Add 220Ω resistor in series
   - Connect to Arduino pin A4
   - Connect cathode to GND

   - Add LED for led38_l1_0
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
