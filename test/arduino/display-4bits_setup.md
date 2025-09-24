# Tinkercad Setup Guide: display-4bits

## Overview
This guide helps you recreate the circuit for `display-4bits.ino` in Tinkercad.

## Target Arduino Board
**Arduino UNO R3/R4**
- Pin Usage: 12/18 pins
- Standard Arduino board with 20 I/O pins (14 digital + 6 analog)

## Required Components
- 1x Arduino Uno R3
- 1x Breadboard
- 4x Slide Switch
- 4x Generic Output
- 8x 7-Segment Display
- 8x Resistor

## Pin Usage
Total pins used: 12/18

### Input Connections
- **input_switch1_d** (Slide Switch) → Arduino pin A0
- **input_switch2_c** (Slide Switch) → Arduino pin A1
- **input_switch3_b** (Slide Switch) → Arduino pin A2
- **input_switch4_a** (Slide Switch) → Arduino pin A3

### Output Connections
- **input_switch1_d** (Generic Output) → Arduino pin A0
- **input_switch2_c** (Generic Output) → Arduino pin A1
- **input_switch3_b** (Generic Output) → Arduino pin A2
- **input_switch4_a** (Generic Output) → Arduino pin A3
- **seven_segment_display63_g_middle** (7-Segment Display) → Arduino pin A4 (with 220Ω resistor)
- **seven_segment_display63_f_upper_left** (7-Segment Display) → Arduino pin A5 (with 220Ω resistor)
- **seven_segment_display63_e_lower_left** (7-Segment Display) → Arduino pin 2 (with 220Ω resistor)
- **seven_segment_display63_d_bottom** (7-Segment Display) → Arduino pin 3 (with 220Ω resistor)
- **seven_segment_display63_a_top** (7-Segment Display) → Arduino pin 4 (with 220Ω resistor)
- **seven_segment_display63_b_upper_right** (7-Segment Display) → Arduino pin 5 (with 220Ω resistor)
- **seven_segment_display63_dp_dot** (7-Segment Display) → Arduino pin 6 (with 220Ω resistor)
- **seven_segment_display63_c_lower_right** (7-Segment Display) → Arduino pin 7 (with 220Ω resistor)

## Setup Steps in Tinkercad

1. **Add Arduino UNO R3/R4**
   - Drag Arduino UNO R3/R4 to the workspace

2. **Add Breadboard**
   - Place a breadboard next to the Arduino

3. **Add Input Components**
   - Add Slide Switch for input_switch1_d
   - Connect to Arduino pin A0
   - Connect other terminal to GND (for buttons/switches)

   - Add Slide Switch for input_switch2_c
   - Connect to Arduino pin A1
   - Connect other terminal to GND (for buttons/switches)

   - Add Slide Switch for input_switch3_b
   - Connect to Arduino pin A2
   - Connect other terminal to GND (for buttons/switches)

   - Add Slide Switch for input_switch4_a
   - Connect to Arduino pin A3
   - Connect other terminal to GND (for buttons/switches)

4. **Add Output Components**
   - Add Generic Output for input_switch1_d
   - Connect to Arduino pin A0

   - Add Generic Output for input_switch2_c
   - Connect to Arduino pin A1

   - Add Generic Output for input_switch3_b
   - Connect to Arduino pin A2

   - Add Generic Output for input_switch4_a
   - Connect to Arduino pin A3

   - Add 7-Segment Display for seven_segment_display63_g_middle
   - Add 220Ω resistor in series
   - Connect to Arduino pin A4

   - Add 7-Segment Display for seven_segment_display63_f_upper_left
   - Add 220Ω resistor in series
   - Connect to Arduino pin A5

   - Add 7-Segment Display for seven_segment_display63_e_lower_left
   - Add 220Ω resistor in series
   - Connect to Arduino pin 2

   - Add 7-Segment Display for seven_segment_display63_d_bottom
   - Add 220Ω resistor in series
   - Connect to Arduino pin 3

   - Add 7-Segment Display for seven_segment_display63_a_top
   - Add 220Ω resistor in series
   - Connect to Arduino pin 4

   - Add 7-Segment Display for seven_segment_display63_b_upper_right
   - Add 220Ω resistor in series
   - Connect to Arduino pin 5

   - Add 7-Segment Display for seven_segment_display63_dp_dot
   - Add 220Ω resistor in series
   - Connect to Arduino pin 6

   - Add 7-Segment Display for seven_segment_display63_c_lower_right
   - Add 220Ω resistor in series
   - Connect to Arduino pin 7

5. **Upload Code**
   - Copy the contents of the .ino file
   - Paste into Tinkercad's code editor
   - Click "Start Simulation"

## Notes
- Ensure all connections are secure
- Check pin assignments match the code
- Use appropriate resistor values for LEDs
- Test inputs work correctly before running simulation
