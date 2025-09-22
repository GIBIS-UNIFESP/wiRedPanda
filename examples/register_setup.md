# Tinkercad Setup Guide: register

## Overview
This guide helps you recreate the circuit for `register.ino` in Tinkercad.

## Target Arduino Board
**Arduino UNO R3/R4**
- Pin Usage: 4/18 pins
- Standard Arduino board with 20 I/O pins (14 digital + 6 analog)

## Required Components
- 1x Arduino Uno R3
- 1x Breadboard
- 4x LED
- 4x Resistor

## Pin Usage
Total pins used: 4/18

### Output Connections
- **led11_0** (LED) → Arduino pin A0 (with 220Ω resistor)
- **led12_0** (LED) → Arduino pin A1 (with 220Ω resistor)
- **led13_0** (LED) → Arduino pin A2 (with 220Ω resistor)
- **led14_0** (LED) → Arduino pin A3 (with 220Ω resistor)

### Timing Information
- Clock 1: 125.0ms interval (8.00 Hz)
- Clock 2: 500.0ms interval (2.00 Hz)

## Setup Steps in Tinkercad

1. **Add Arduino UNO R3/R4**
   - Drag Arduino UNO R3/R4 to the workspace

2. **Add Breadboard**
   - Place a breadboard next to the Arduino

3. **Add Input Components**
4. **Add Output Components**
   - Add LED for led11_0
   - Add 220Ω resistor in series
   - Connect to Arduino pin A0
   - Connect cathode to GND

   - Add LED for led12_0
   - Add 220Ω resistor in series
   - Connect to Arduino pin A1
   - Connect cathode to GND

   - Add LED for led13_0
   - Add 220Ω resistor in series
   - Connect to Arduino pin A2
   - Connect cathode to GND

   - Add LED for led14_0
   - Add 220Ω resistor in series
   - Connect to Arduino pin A3
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
