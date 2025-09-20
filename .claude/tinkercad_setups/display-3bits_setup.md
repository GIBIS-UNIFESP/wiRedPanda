# Tinkercad Setup Guide: display-3bits

## Overview
This guide helps you recreate the circuit for `display-3bits.ino` in Tinkercad.

## Required Components
- 1x Arduino Uno R3
- 1x Breadboard
- 3x Slide Switch
- 3x Generic Output
- 8x 7-Segment Display
- 8x Resistor

## Pin Usage
Total pins used: 14/20 available

### Input Connections
- **input_switch1_p2** (Slide Switch) → Arduino pin A0
- **input_switch2_p1** (Slide Switch) → Arduino pin A1
- **input_switch3_p3** (Slide Switch) → Arduino pin A2

### Output Connections
- **input_switch1_p2** (Generic Output) → Arduino pin A0
- **input_switch2_p1** (Generic Output) → Arduino pin A1
- **input_switch3_p3** (Generic Output) → Arduino pin A2
- **seven_segment_display32_g_middle** (7-Segment Display) → Arduino pin A3 (with 220Ω resistor)
- **seven_segment_display32_f_upper_left** (7-Segment Display) → Arduino pin A4 (with 220Ω resistor)
- **seven_segment_display32_e_lower_left** (7-Segment Display) → Arduino pin A5 (with 220Ω resistor)
- **seven_segment_display32_d_bottom** (7-Segment Display) → Arduino pin 2 (with 220Ω resistor)
- **seven_segment_display32_a_top** (7-Segment Display) → Arduino pin 3 (with 220Ω resistor)
- **seven_segment_display32_b_upper_right** (7-Segment Display) → Arduino pin 4 (with 220Ω resistor)
- **seven_segment_display32_dp_dot** (7-Segment Display) → Arduino pin 5 (with 220Ω resistor)
- **seven_segment_display32_c_lower_right** (7-Segment Display) → Arduino pin 6 (with 220Ω resistor)

## Setup Steps in Tinkercad

1. **Add Arduino Uno R3**
   - Drag Arduino Uno R3 to the workspace

2. **Add Breadboard**
   - Place a breadboard next to the Arduino

3. **Add Input Components**
   - Add Slide Switch for input_switch1_p2
   - Connect to Arduino pin A0
   - Connect other terminal to GND (for buttons/switches)

   - Add Slide Switch for input_switch2_p1
   - Connect to Arduino pin A1
   - Connect other terminal to GND (for buttons/switches)

   - Add Slide Switch for input_switch3_p3
   - Connect to Arduino pin A2
   - Connect other terminal to GND (for buttons/switches)

4. **Add Output Components**
   - Add Generic Output for input_switch1_p2
   - Connect to Arduino pin A0

   - Add Generic Output for input_switch2_p1
   - Connect to Arduino pin A1

   - Add Generic Output for input_switch3_p3
   - Connect to Arduino pin A2

   - Add 7-Segment Display for seven_segment_display32_g_middle
   - Add 220Ω resistor in series
   - Connect to Arduino pin A3

   - Add 7-Segment Display for seven_segment_display32_f_upper_left
   - Add 220Ω resistor in series
   - Connect to Arduino pin A4

   - Add 7-Segment Display for seven_segment_display32_e_lower_left
   - Add 220Ω resistor in series
   - Connect to Arduino pin A5

   - Add 7-Segment Display for seven_segment_display32_d_bottom
   - Add 220Ω resistor in series
   - Connect to Arduino pin 2

   - Add 7-Segment Display for seven_segment_display32_a_top
   - Add 220Ω resistor in series
   - Connect to Arduino pin 3

   - Add 7-Segment Display for seven_segment_display32_b_upper_right
   - Add 220Ω resistor in series
   - Connect to Arduino pin 4

   - Add 7-Segment Display for seven_segment_display32_dp_dot
   - Add 220Ω resistor in series
   - Connect to Arduino pin 5

   - Add 7-Segment Display for seven_segment_display32_c_lower_right
   - Add 220Ω resistor in series
   - Connect to Arduino pin 6

5. **Upload Code**
   - Copy the contents of the .ino file
   - Paste into Tinkercad's code editor
   - Click "Start Simulation"

## Notes
- Ensure all connections are secure
- Check pin assignments match the code
- Use appropriate resistor values for LEDs
- Test inputs work correctly before running simulation
