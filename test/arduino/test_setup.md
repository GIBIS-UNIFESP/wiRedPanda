# Tinkercad Setup Guide: test

## Overview
This guide helps you recreate the circuit for `test.ino` in Tinkercad.

## Target Arduino Board
**Arduino UNO R3/R4**
- Pin Usage: 3/18 pins
- Standard Arduino board with 20 I/O pins (14 digital + 6 analog)

## Required Components
- 1x Arduino Uno R3
- 1x Breadboard
- 3x LED
- 3x Resistor

## Pin Usage
Total pins used: 3/18

### Output Connections
- **led5_0** (LED) → Arduino pin A0 (with 220Ω resistor)
- **led6_0** (LED) → Arduino pin A1 (with 220Ω resistor)
- **led7_0** (LED) → Arduino pin A2 (with 220Ω resistor)

### Timing Information
- Clock 1: 250.0ms interval (4.00 Hz)

## Setup Steps in Tinkercad

1. **Add Arduino UNO R3/R4**
   - Drag Arduino UNO R3/R4 to the workspace

2. **Add Breadboard**
   - Place a breadboard next to the Arduino

3. **Add Input Components**
4. **Add Output Components**
   - Add LED for led5_0
   - Add 220Ω resistor in series
   - Connect to Arduino pin A0
   - Connect cathode to GND

   - Add LED for led6_0
   - Add 220Ω resistor in series
   - Connect to Arduino pin A1
   - Connect cathode to GND

   - Add LED for led7_0
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
