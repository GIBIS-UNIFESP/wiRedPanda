# Tinkercad Setup Guide: display-4bits-counter

## Overview
This guide helps you recreate the circuit for `display-4bits-counter.ino` in Tinkercad.

## Target Arduino Board
**Arduino UNO R3/R4**
- Pin Usage: 9/18 pins
- Standard Arduino board with 20 I/O pins (14 digital + 6 analog)

## Required Components
- 1x Arduino Uno R3
- 1x Breadboard
- 1x Pushbutton
- 1x Generic Output
- 8x 7-Segment Display
- 8x Resistor

## Pin Usage
Total pins used: 9/18

### Input Connections
- **push_button1** (Pushbutton) → Arduino pin A0

### Output Connections
- **push_button1** (Generic Output) → Arduino pin A0
- **seven_segment_display24_g_middle** (7-Segment Display) → Arduino pin A1 (with 220Ω resistor)
- **seven_segment_display24_f_upper_left** (7-Segment Display) → Arduino pin A2 (with 220Ω resistor)
- **seven_segment_display24_e_lower_left** (7-Segment Display) → Arduino pin A3 (with 220Ω resistor)
- **seven_segment_display24_d_bottom** (7-Segment Display) → Arduino pin A4 (with 220Ω resistor)
- **seven_segment_display24_a_top** (7-Segment Display) → Arduino pin A5 (with 220Ω resistor)
- **seven_segment_display24_b_upper_right** (7-Segment Display) → Arduino pin 2 (with 220Ω resistor)
- **seven_segment_display24_dp_dot** (7-Segment Display) → Arduino pin 3 (with 220Ω resistor)
- **seven_segment_display24_c_lower_right** (7-Segment Display) → Arduino pin 4 (with 220Ω resistor)

### Timing Information
- Clock 1: 250.0ms interval (4.00 Hz)

## Setup Steps in Tinkercad

1. **Add Arduino UNO R3/R4**
   - Drag Arduino UNO R3/R4 to the workspace

2. **Add Breadboard**
   - Place a breadboard next to the Arduino

3. **Add Input Components**
   - Add Pushbutton for push_button1
   - Connect to Arduino pin A0
   - Connect other terminal to GND (for buttons/switches)

4. **Add Output Components**
   - Add Generic Output for push_button1
   - Connect to Arduino pin A0

   - Add 7-Segment Display for seven_segment_display24_g_middle
   - Add 220Ω resistor in series
   - Connect to Arduino pin A1

   - Add 7-Segment Display for seven_segment_display24_f_upper_left
   - Add 220Ω resistor in series
   - Connect to Arduino pin A2

   - Add 7-Segment Display for seven_segment_display24_e_lower_left
   - Add 220Ω resistor in series
   - Connect to Arduino pin A3

   - Add 7-Segment Display for seven_segment_display24_d_bottom
   - Add 220Ω resistor in series
   - Connect to Arduino pin A4

   - Add 7-Segment Display for seven_segment_display24_a_top
   - Add 220Ω resistor in series
   - Connect to Arduino pin A5

   - Add 7-Segment Display for seven_segment_display24_b_upper_right
   - Add 220Ω resistor in series
   - Connect to Arduino pin 2

   - Add 7-Segment Display for seven_segment_display24_dp_dot
   - Add 220Ω resistor in series
   - Connect to Arduino pin 3

   - Add 7-Segment Display for seven_segment_display24_c_lower_right
   - Add 220Ω resistor in series
   - Connect to Arduino pin 4

5. **Upload Code**
   - Copy the contents of the .ino file
   - Paste into Tinkercad's code editor
   - Click "Start Simulation"

## Notes
- Ensure all connections are secure
- Check pin assignments match the code
- Use appropriate resistor values for LEDs
- Test inputs work correctly before running simulation
