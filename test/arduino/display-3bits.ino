// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //
//
// Target Board: Arduino UNO R3/R4
// Pin Usage: 11/18 pins
// Standard Arduino board with 20 I/O pins (14 digital + 6 analog)
//


/* ========= Inputs ========== */
const int input_switch1_p3 = A0;
const int input_switch2_p1 = A1;
const int input_switch3_p2 = A2;

/* ========= Outputs ========== */
const int seven_segment_display32_g_middle = A3;
const int seven_segment_display32_f_upper_left = A4;
const int seven_segment_display32_e_lower_left = A5;
const int seven_segment_display32_d_bottom = 2;
const int seven_segment_display32_a_top = 3;
const int seven_segment_display32_b_upper_right = 4;
const int seven_segment_display32_dp_dot = 5;
const int seven_segment_display32_c_lower_right = 6;

/* ====== Aux. Variables ====== */
bool input_switch1_p3_val = false;
bool input_switch2_p1_val = false;
bool input_switch3_p2_val = false;
bool aux_input_switch_0 = false;
bool aux_input_switch_1 = false;
bool aux_input_switch_2 = false;
bool aux_or_4 = false;
bool aux_or_5 = false;
bool aux_or_6 = false;
bool aux_not_7 = false;
bool aux_not_8 = false;
bool aux_not_9 = false;
bool aux_and_10 = false;
bool aux_and_11 = false;
bool aux_and_12 = false;
bool aux_and_13 = false;
bool aux_xnor_14 = false;
bool aux_and_15 = false;
bool aux_and_16 = false;
bool aux_and_17 = false;
bool aux_and_18 = false;
bool aux_and_19 = false;
bool aux_xnor_20 = false;
bool aux_and_22 = false;
bool aux_node_23 = false;
bool aux_or_24 = false;
bool aux_or_25 = false;
bool aux_nand_26 = false;
bool aux_or_27 = false;
bool aux_or_28 = false;
bool aux_or_29 = false;
bool aux_or_30 = false;

void setup() {
    pinMode(input_switch1_p3, INPUT);
    pinMode(input_switch2_p1, INPUT);
    pinMode(input_switch3_p2, INPUT);
    pinMode(seven_segment_display32_g_middle, OUTPUT);
    pinMode(seven_segment_display32_f_upper_left, OUTPUT);
    pinMode(seven_segment_display32_e_lower_left, OUTPUT);
    pinMode(seven_segment_display32_d_bottom, OUTPUT);
    pinMode(seven_segment_display32_a_top, OUTPUT);
    pinMode(seven_segment_display32_b_upper_right, OUTPUT);
    pinMode(seven_segment_display32_dp_dot, OUTPUT);
    pinMode(seven_segment_display32_c_lower_right, OUTPUT);
}

void loop() {
    // Read input data
    input_switch1_p3_val = digitalRead(input_switch1_p3);
    input_switch2_p1_val = digitalRead(input_switch2_p1);
    input_switch3_p2_val = digitalRead(input_switch3_p2);

    // Update clocks

    // Update logic variables
    aux_input_switch_0 = input_switch1_p3_val;
    aux_input_switch_1 = input_switch2_p1_val;
    aux_input_switch_2 = input_switch3_p2_val;
    aux_or_4 = false || aux_input_switch_0;
    aux_or_5 = false || aux_input_switch_2;
    aux_or_6 = false || aux_input_switch_1;
    aux_not_7 = !aux_or_6;
    aux_not_8 = !aux_or_4;
    aux_not_9 = !aux_or_5;
    aux_and_10 = aux_or_5 && aux_not_8;
    aux_and_11 = aux_or_6 && aux_not_9;
    aux_and_12 = aux_or_5 && aux_not_8;
    aux_and_13 = aux_not_7 && aux_not_9 && aux_not_8;
    aux_xnor_14 = !(aux_not_9 ^ aux_not_8);
    aux_and_15 = aux_or_6 && aux_not_8;
    aux_and_16 = aux_or_6 && aux_not_9 && aux_or_4;
    aux_and_17 = aux_input_switch_2 && aux_not_8;
    aux_and_18 = aux_not_9 && aux_not_8;
    aux_and_19 = aux_not_7 && aux_not_8;
    aux_xnor_20 = !(aux_not_7 ^ aux_not_8);
    aux_and_22 = aux_not_7 && aux_or_5;
    aux_node_23 = false;
    aux_or_24 = aux_and_12 || aux_and_13;
    aux_or_25 = aux_and_18 || aux_and_11 || aux_and_15;
    aux_nand_26 = !(aux_not_7 && aux_or_5 && aux_not_8);
    aux_or_27 = aux_and_11 || aux_and_22 || aux_and_17;
    aux_or_28 = aux_not_7 || aux_xnor_14;
    aux_or_29 = aux_and_19 || aux_and_22 || aux_and_10 || aux_and_16;
    aux_or_30 = aux_xnor_20 || aux_or_5;

    // Write output data
    digitalWrite(seven_segment_display32_g_middle, aux_or_27);
    digitalWrite(seven_segment_display32_f_upper_left, aux_or_25);
    digitalWrite(seven_segment_display32_e_lower_left, aux_or_24);
    digitalWrite(seven_segment_display32_d_bottom, aux_or_29);
    digitalWrite(seven_segment_display32_a_top, aux_or_30);
    digitalWrite(seven_segment_display32_b_upper_right, aux_or_28);
    digitalWrite(seven_segment_display32_dp_dot, aux_node_23);
    digitalWrite(seven_segment_display32_c_lower_right, aux_nand_26);
}
