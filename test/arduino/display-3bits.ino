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
const int seven_segment_display16_g_middle = A3;
const int seven_segment_display16_f_upper_left = A4;
const int seven_segment_display16_e_lower_left = A5;
const int seven_segment_display16_d_bottom = 2;
const int seven_segment_display16_a_top = 3;
const int seven_segment_display16_b_upper_right = 4;
const int seven_segment_display16_dp_dot = 5;
const int seven_segment_display16_c_lower_right = 6;

/* ====== Aux. Variables ====== */
bool input_switch1_p3_val = false;
bool input_switch2_p1_val = false;
bool input_switch3_p2_val = false;
bool aux_node_0 = false;
bool aux_or_1 = false;
bool aux_or_2 = false;
bool aux_and_3 = false;
bool aux_or_4 = false;
bool aux_not_5 = false;
bool aux_and_6 = false;
bool aux_and_7 = false;
bool aux_and_8 = false;
bool aux_xnor_9 = false;
bool aux_and_10 = false;
bool aux_or_11 = false;
bool aux_nand_12 = false;
bool aux_or_13 = false;
bool aux_or_14 = false;
bool aux_not_16 = false;
bool aux_and_17 = false;
bool aux_and_18 = false;
bool aux_and_19 = false;
bool aux_and_20 = false;
bool aux_input_switch_21 = false;
bool aux_or_22 = false;
bool aux_input_switch_23 = false;
bool aux_input_switch_24 = false;
bool aux_xnor_25 = false;
bool aux_or_27 = false;
bool aux_and_28 = false;
bool aux_not_29 = false;
bool aux_or_30 = false;

void setup() {
    pinMode(input_switch1_p3, INPUT);
    pinMode(input_switch2_p1, INPUT);
    pinMode(input_switch3_p2, INPUT);
    pinMode(seven_segment_display16_g_middle, OUTPUT);
    pinMode(seven_segment_display16_f_upper_left, OUTPUT);
    pinMode(seven_segment_display16_e_lower_left, OUTPUT);
    pinMode(seven_segment_display16_d_bottom, OUTPUT);
    pinMode(seven_segment_display16_a_top, OUTPUT);
    pinMode(seven_segment_display16_b_upper_right, OUTPUT);
    pinMode(seven_segment_display16_dp_dot, OUTPUT);
    pinMode(seven_segment_display16_c_lower_right, OUTPUT);
}

void loop() {
    // Read input data
    input_switch1_p3_val = digitalRead(input_switch1_p3);
    input_switch2_p1_val = digitalRead(input_switch2_p1);
    input_switch3_p2_val = digitalRead(input_switch3_p2);

    // Update clocks

    // Update logic variables
    aux_node_0 = false;
    aux_or_1 = false || aux_input_switch_21;
    aux_or_2 = false || aux_input_switch_24;
    aux_and_3 = aux_or_2 && aux_not_16;
    aux_or_4 = aux_and_7 || aux_and_8;
    aux_not_5 = !aux_or_30;
    aux_and_6 = aux_or_30 && aux_not_29;
    aux_and_7 = aux_or_2 && aux_not_16;
    aux_and_8 = aux_not_5 && aux_not_29 && aux_not_16;
    aux_xnor_9 = !(aux_not_29 ^ aux_not_16);
    aux_and_10 = aux_or_30 && aux_not_16;
    aux_or_11 = aux_and_19 || aux_and_6 || aux_and_10;
    aux_nand_12 = !(aux_not_5 && aux_or_2 && aux_not_16);
    aux_or_13 = aux_and_6 || aux_and_28 || aux_and_18;
    aux_or_14 = aux_not_5 || aux_xnor_9;
    aux_not_16 = !aux_or_1;
    aux_and_17 = aux_or_30 && aux_not_29 && aux_or_1;
    aux_and_18 = aux_input_switch_24 && aux_not_16;
    aux_and_19 = aux_not_29 && aux_not_16;
    aux_and_20 = aux_not_5 && aux_not_16;
    aux_input_switch_21 = input_switch1_p3_val;
    aux_or_22 = aux_and_20 || aux_and_28 || aux_and_3 || aux_and_17;
    aux_input_switch_23 = input_switch2_p1_val;
    aux_input_switch_24 = input_switch3_p2_val;
    aux_xnor_25 = !(aux_not_5 ^ aux_not_16);
    aux_or_27 = aux_xnor_25 || aux_or_2;
    aux_and_28 = aux_not_5 && aux_or_2;
    aux_not_29 = !aux_or_2;
    aux_or_30 = false || aux_input_switch_23;

    // Write output data
    digitalWrite(seven_segment_display16_g_middle, aux_or_13);
    digitalWrite(seven_segment_display16_f_upper_left, aux_or_11);
    digitalWrite(seven_segment_display16_e_lower_left, aux_or_4);
    digitalWrite(seven_segment_display16_d_bottom, aux_or_22);
    digitalWrite(seven_segment_display16_a_top, aux_or_27);
    digitalWrite(seven_segment_display16_b_upper_right, aux_or_14);
    digitalWrite(seven_segment_display16_dp_dot, aux_node_0);
    digitalWrite(seven_segment_display16_c_lower_right, aux_nand_12);
}
