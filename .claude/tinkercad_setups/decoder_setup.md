# Tinkercad Setup Guide: decoder

## Overview
This guide helps you recreate the circuit for `decoder.ino` in Tinkercad.

## Required Components
- 1x Arduino Uno R3
- 1x Breadboard
- 3x Slide Switch
- 3x Generic Output
- 8x LED
- 8x Resistor

## Pin Usage
Total pins used: 14/20 available

### Input Connections
- **input_switch1** (Slide Switch) → Arduino pin A0
- **input_switch2** (Slide Switch) → Arduino pin A1
- **input_switch3** (Slide Switch) → Arduino pin A2

### Output Connections
- **input_switch1** (Generic Output) → Arduino pin A0
- **input_switch2** (Generic Output) → Arduino pin A1
- **input_switch3** (Generic Output) → Arduino pin A2
- **led42_0** (LED) → Arduino pin A3 (with 220Ω resistor)
- **led43_0** (LED) → Arduino pin A4 (with 220Ω resistor)
- **led44_0** (LED) → Arduino pin A5 (with 220Ω resistor)
- **led45_0** (LED) → Arduino pin 2 (with 220Ω resistor)
- **led46_0** (LED) → Arduino pin 3 (with 220Ω resistor)
- **led47_0** (LED) → Arduino pin 4 (with 220Ω resistor)
- **led48_0** (LED) → Arduino pin 5 (with 220Ω resistor)
- **led55_0** (LED) → Arduino pin 6 (with 220Ω resistor)

## Setup Steps in Tinkercad

1. **Add Arduino Uno R3**
   - Drag Arduino Uno R3 to the workspace

2. **Add Breadboard**
   - Place a breadboard next to the Arduino

3. **Add Input Components**
   - Add Slide Switch for input_switch1
   - Connect to Arduino pin A0
   - Connect other terminal to GND (for buttons/switches)

   - Add Slide Switch for input_switch2
   - Connect to Arduino pin A1
   - Connect other terminal to GND (for buttons/switches)

   - Add Slide Switch for input_switch3
   - Connect to Arduino pin A2
   - Connect other terminal to GND (for buttons/switches)

4. **Add Output Components**
   - Add Generic Output for input_switch1
   - Connect to Arduino pin A0

   - Add Generic Output for input_switch2
   - Connect to Arduino pin A1

   - Add Generic Output for input_switch3
   - Connect to Arduino pin A2

   - Add LED for led42_0
   - Add 220Ω resistor in series
   - Connect to Arduino pin A3
   - Connect cathode to GND

   - Add LED for led43_0
   - Add 220Ω resistor in series
   - Connect to Arduino pin A4
   - Connect cathode to GND

   - Add LED for led44_0
   - Add 220Ω resistor in series
   - Connect to Arduino pin A5
   - Connect cathode to GND

   - Add LED for led45_0
   - Add 220Ω resistor in series
   - Connect to Arduino pin 2
   - Connect cathode to GND

   - Add LED for led46_0
   - Add 220Ω resistor in series
   - Connect to Arduino pin 3
   - Connect cathode to GND

   - Add LED for led47_0
   - Add 220Ω resistor in series
   - Connect to Arduino pin 4
   - Connect cathode to GND

   - Add LED for led48_0
   - Add 220Ω resistor in series
   - Connect to Arduino pin 5
   - Connect cathode to GND

   - Add LED for led55_0
   - Add 220Ω resistor in series
   - Connect to Arduino pin 6
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
