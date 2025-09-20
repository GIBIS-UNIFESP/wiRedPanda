# Tinkercad Setup Guide: dflipflop-masterslave

## Overview
This guide helps you recreate the circuit for `dflipflop-masterslave.ino` in Tinkercad.

## Required Components
- 1x Arduino Uno R3
- 1x Breadboard
- 1x Pushbutton
- 1x Generic Output
- 2x LED
- 2x Resistor

## Pin Usage
Total pins used: 4/20 available

### Input Connections
- **push_button1_d** (Pushbutton) → Arduino pin A0

### Output Connections
- **push_button1_d** (Generic Output) → Arduino pin A0
- **led8_q_0** (LED) → Arduino pin A1 (with 220Ω resistor)
- **led9_q_0** (LED) → Arduino pin A2 (with 220Ω resistor)

### Timing Information
- Clock 1: 1000.0ms interval (1.00 Hz)

## Setup Steps in Tinkercad

1. **Add Arduino Uno R3**
   - Drag Arduino Uno R3 to the workspace

2. **Add Breadboard**
   - Place a breadboard next to the Arduino

3. **Add Input Components**
   - Add Pushbutton for push_button1_d
   - Connect to Arduino pin A0
   - Connect other terminal to GND (for buttons/switches)

4. **Add Output Components**
   - Add Generic Output for push_button1_d
   - Connect to Arduino pin A0

   - Add LED for led8_q_0
   - Add 220Ω resistor in series
   - Connect to Arduino pin A1
   - Connect cathode to GND

   - Add LED for led9_q_0
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
