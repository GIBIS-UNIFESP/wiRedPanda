# Tinkercad Setup Guide: input

## Overview
This guide helps you recreate the circuit for `input.ino` in Tinkercad.

## Target Arduino Board
**Arduino UNO R3/R4**
- Pin Usage: 6/18 pins
- Standard Arduino board with 20 I/O pins (14 digital + 6 analog)

## Required Components
- 1x Arduino Uno R3
- 1x Breadboard
- 2x Pushbutton
- 2x Generic Output
- 4x LED
- 4x Resistor

## Pin Usage
Total pins used: 6/18

### Input Connections
- **push_button1_x0** (Pushbutton) → Arduino pin A0
- **push_button2_x1** (Pushbutton) → Arduino pin A1

### Output Connections
- **push_button1_x0** (Generic Output) → Arduino pin A0
- **push_button2_x1** (Generic Output) → Arduino pin A1
- **led7_xor_0** (LED) → Arduino pin A2 (with 220Ω resistor)
- **led8_not_x0_0** (LED) → Arduino pin A3 (with 220Ω resistor)
- **led9_and_0** (LED) → Arduino pin A4 (with 220Ω resistor)
- **led10_or_0** (LED) → Arduino pin A5 (with 220Ω resistor)

## Setup Steps in Tinkercad

1. **Add Arduino UNO R3/R4**
   - Drag Arduino UNO R3/R4 to the workspace

2. **Add Breadboard**
   - Place a breadboard next to the Arduino

3. **Add Input Components**
   - Add Pushbutton for push_button1_x0
   - Connect to Arduino pin A0
   - Connect other terminal to GND (for buttons/switches)

   - Add Pushbutton for push_button2_x1
   - Connect to Arduino pin A1
   - Connect other terminal to GND (for buttons/switches)

4. **Add Output Components**
   - Add Generic Output for push_button1_x0
   - Connect to Arduino pin A0

   - Add Generic Output for push_button2_x1
   - Connect to Arduino pin A1

   - Add LED for led7_xor_0
   - Add 220Ω resistor in series
   - Connect to Arduino pin A2
   - Connect cathode to GND

   - Add LED for led8_not_x0_0
   - Add 220Ω resistor in series
   - Connect to Arduino pin A3
   - Connect cathode to GND

   - Add LED for led9_and_0
   - Add 220Ω resistor in series
   - Connect to Arduino pin A4
   - Connect cathode to GND

   - Add LED for led10_or_0
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
