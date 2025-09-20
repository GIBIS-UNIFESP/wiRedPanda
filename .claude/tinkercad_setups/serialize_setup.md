# Tinkercad Setup Guide: serialize

## Overview
This guide helps you recreate the circuit for `serialize.ino` in Tinkercad.

## Required Components
- 1x Arduino Uno R3
- 1x Breadboard
- 6x Slide Switch
- 6x Generic Output
- 2x LED
- 2x Resistor

## Pin Usage
Total pins used: 14/20 available

### Input Connections
- **input_switch1_load__shift** (Slide Switch) → Arduino pin A0
- **input_switch2_clock** (Slide Switch) → Arduino pin A1
- **input_switch3_d0** (Slide Switch) → Arduino pin A2
- **input_switch4_d1** (Slide Switch) → Arduino pin A3
- **input_switch5_d2** (Slide Switch) → Arduino pin A4
- **input_switch6_d3** (Slide Switch) → Arduino pin A5

### Output Connections
- **input_switch1_load__shift** (Generic Output) → Arduino pin A0
- **input_switch2_clock** (Generic Output) → Arduino pin A1
- **input_switch3_d0** (Generic Output) → Arduino pin A2
- **input_switch4_d1** (Generic Output) → Arduino pin A3
- **input_switch5_d2** (Generic Output) → Arduino pin A4
- **input_switch6_d3** (Generic Output) → Arduino pin A5
- **led37_0** (LED) → Arduino pin 2 (with 220Ω resistor)
- **led38_0** (LED) → Arduino pin 3 (with 220Ω resistor)

## Setup Steps in Tinkercad

1. **Add Arduino Uno R3**
   - Drag Arduino Uno R3 to the workspace

2. **Add Breadboard**
   - Place a breadboard next to the Arduino

3. **Add Input Components**
   - Add Slide Switch for input_switch1_load__shift
   - Connect to Arduino pin A0
   - Connect other terminal to GND (for buttons/switches)

   - Add Slide Switch for input_switch2_clock
   - Connect to Arduino pin A1
   - Connect other terminal to GND (for buttons/switches)

   - Add Slide Switch for input_switch3_d0
   - Connect to Arduino pin A2
   - Connect other terminal to GND (for buttons/switches)

   - Add Slide Switch for input_switch4_d1
   - Connect to Arduino pin A3
   - Connect other terminal to GND (for buttons/switches)

   - Add Slide Switch for input_switch5_d2
   - Connect to Arduino pin A4
   - Connect other terminal to GND (for buttons/switches)

   - Add Slide Switch for input_switch6_d3
   - Connect to Arduino pin A5
   - Connect other terminal to GND (for buttons/switches)

4. **Add Output Components**
   - Add Generic Output for input_switch1_load__shift
   - Connect to Arduino pin A0

   - Add Generic Output for input_switch2_clock
   - Connect to Arduino pin A1

   - Add Generic Output for input_switch3_d0
   - Connect to Arduino pin A2

   - Add Generic Output for input_switch4_d1
   - Connect to Arduino pin A3

   - Add Generic Output for input_switch5_d2
   - Connect to Arduino pin A4

   - Add Generic Output for input_switch6_d3
   - Connect to Arduino pin A5

   - Add LED for led37_0
   - Add 220Ω resistor in series
   - Connect to Arduino pin 2
   - Connect cathode to GND

   - Add LED for led38_0
   - Add 220Ω resistor in series
   - Connect to Arduino pin 3
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
