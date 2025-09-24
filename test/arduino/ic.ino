// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //
//
// Target Board: Arduino Mega 2560
// Pin Usage: 28/68 pins
// High I/O count Arduino board with 70 I/O pins (54 digital + 16 analog)
//


/* ========= Inputs ========== */

/* ========= Outputs ========== */
const int seven_segment_display1_g_middle = A0;
const int seven_segment_display1_f_upper_left = A1;
const int seven_segment_display1_e_lower_left = A2;
const int seven_segment_display1_d_bottom = A3;
const int seven_segment_display1_a_top = A4;
const int seven_segment_display1_b_upper_right = A5;
const int seven_segment_display1_dp_dot = A6;
const int seven_segment_display1_c_lower_right = A7;
const int seven_segment_display5_g_middle = A8;
const int seven_segment_display5_f_upper_left = A9;
const int seven_segment_display5_e_lower_left = A10;
const int seven_segment_display5_d_bottom = A11;
const int seven_segment_display5_a_top = A12;
const int seven_segment_display5_b_upper_right = A13;
const int seven_segment_display5_dp_dot = A14;
const int seven_segment_display5_c_lower_right = A15;
const int seven_segment_display7_g_middle = 2;
const int seven_segment_display7_f_upper_left = 3;
const int seven_segment_display7_e_lower_left = 4;
const int seven_segment_display7_d_bottom = 5;
const int seven_segment_display7_a_top = 6;
const int seven_segment_display7_b_upper_right = 7;
const int seven_segment_display7_dp_dot = 8;
const int seven_segment_display7_c_lower_right = 9;
const int led10_0 = 10;
const int led11_0 = 11;
const int led12_0 = 12;
const int led21_0 = 13;

/* ====== Aux. Variables ====== */
bool aux_jk_flip_flop_1_0_q = false;
bool aux_jk_flip_flop_1_0_q_inclk = false;
bool aux_jk_flip_flop_1_1_q = true;
bool aux_jk_flip_flop_1_1_q_inclk = false;
bool aux_jk_flip_flop_2_0_q = false;
bool aux_jk_flip_flop_2_0_q_inclk = false;
bool aux_jk_flip_flop_2_1_q = true;
bool aux_jk_flip_flop_2_1_q_inclk = false;
bool aux_jk_flip_flop_3_0_q = false;
bool aux_jk_flip_flop_3_0_q_inclk = false;
bool aux_jk_flip_flop_3_1_q = true;
bool aux_jk_flip_flop_3_1_q_inclk = false;
// IC: DISPLAY-4BITS
bool aux_display_4bits_5_display7_g_middle = false;
bool aux_display_4bits_5_display7_f_upper_left_1 = false;
bool aux_display_4bits_5_display7_e_lower_left_2 = false;
bool aux_display_4bits_5_display7_d_bottom_3 = false;
bool aux_display_4bits_5_display7_a_top_4 = false;
bool aux_display_4bits_5_display7_b_upper_right_5 = false;
bool aux_display_4bits_5_display7_dp_dot_6 = false;
bool aux_display_4bits_5_display7_c_lower_right_7 = false;
bool aux_display_4bits_5_node_0 = false;
bool aux_display_4bits_5_node_1 = false;
bool aux_display_4bits_5_node_2 = false;
bool aux_display_4bits_5_node_3 = false;
bool aux_display_4bits_5_and_4 = false;
bool aux_display_4bits_5_node_5 = false;
bool aux_display_4bits_5_node_6 = false;
bool aux_display_4bits_5_node_7 = false;
bool aux_display_4bits_5_node_8 = false;
bool aux_display_4bits_5_node_9 = false;
bool aux_display_4bits_5_node_10 = false;
bool aux_display_4bits_5_node_11 = false;
bool aux_display_4bits_5_node_12 = false;
bool aux_display_4bits_5_node_13 = false;
bool aux_display_4bits_5_node_14 = false;
bool aux_display_4bits_5_node_15 = false;
bool aux_display_4bits_5_or_16 = false;
bool aux_display_4bits_5_or_17 = false;
bool aux_display_4bits_5_node_18 = false;
bool aux_display_4bits_5_node_19 = false;
bool aux_display_4bits_5_node_20 = false;
bool aux_display_4bits_5_node_21 = false;
bool aux_display_4bits_5_node_22 = false;
bool aux_display_4bits_5_node_23 = false;
bool aux_display_4bits_5_node_24 = false;
bool aux_display_4bits_5_or_25 = false;
bool aux_display_4bits_5_or_26 = false;
bool aux_display_4bits_5_or_27 = false;
bool aux_display_4bits_5_node_28 = false;
bool aux_display_4bits_5_node_29 = false;
bool aux_display_4bits_5_node_30 = false;
bool aux_display_4bits_5_node_31 = false;
bool aux_display_4bits_5_node_32 = false;
bool aux_display_4bits_5_node_33 = false;
bool aux_display_4bits_5_node_34 = false;
bool aux_display_4bits_5_node_35 = false;
bool aux_display_4bits_5_node_36 = false;
bool aux_display_4bits_5_node_37 = false;
bool aux_display_4bits_5_node_38 = false;
bool aux_display_4bits_5_or_39 = false;
bool aux_display_4bits_5_node_40 = false;
bool aux_display_4bits_5_node_41 = false;
bool aux_display_4bits_5_node_42 = false;
bool aux_display_4bits_5_node_43 = false;
bool aux_display_4bits_5_or_44 = false;
bool aux_display_4bits_5_and_45 = false;
bool aux_display_4bits_5_and_46 = false;
bool aux_display_4bits_5_and_47 = false;
bool aux_display_4bits_5_and_48 = false;
bool aux_display_4bits_5_and_49 = false;
bool aux_display_4bits_5_and_50 = false;
bool aux_display_4bits_5_node_51 = false;
bool aux_display_4bits_5_node_52 = false;
bool aux_display_4bits_5_and_53 = false;
bool aux_display_4bits_5_node_54 = false;
bool aux_display_4bits_5_node_55 = false;
bool aux_display_4bits_5_and_56 = false;
bool aux_display_4bits_5_node_57 = false;
bool aux_display_4bits_5_node_58 = false;
bool aux_display_4bits_5_node_59 = false;
bool aux_display_4bits_5_node_60 = false;
bool aux_display_4bits_5_node_61 = false;
bool aux_display_4bits_5_node_62 = false;
bool aux_display_4bits_5_node_63 = false;
bool aux_display_4bits_5_node_64 = false;
bool aux_display_4bits_5_node_65 = false;
bool aux_display_4bits_5_node_66 = false;
bool aux_display_4bits_5_node_67 = false;
bool aux_display_4bits_5_node_68 = false;
bool aux_display_4bits_5_node_69 = false;
bool aux_display_4bits_5_node_70 = false;
bool aux_display_4bits_5_node_71 = false;
bool aux_display_4bits_5_node_72 = false;
bool aux_display_4bits_5_not_73 = false;
bool aux_display_4bits_5_not_74 = false;
bool aux_display_4bits_5_not_75 = false;
bool aux_display_4bits_5_not_76 = false;
bool aux_ic_input_display_4bits_0 = false;
bool aux_ic_input_display_4bits_1 = false;
bool aux_ic_input_display_4bits_2 = false;
bool aux_ic_input_display_4bits_3 = false;
// End IC: DISPLAY-4BITS
// IC: DISPLAY-4BITS
bool aux_display_4bits_7_display7_g_middle = false;
bool aux_display_4bits_7_display7_f_upper_left_1 = false;
bool aux_display_4bits_7_display7_e_lower_left_2 = false;
bool aux_display_4bits_7_display7_d_bottom_3 = false;
bool aux_display_4bits_7_display7_a_top_4 = false;
bool aux_display_4bits_7_display7_b_upper_right_5 = false;
bool aux_display_4bits_7_display7_dp_dot_6 = false;
bool aux_display_4bits_7_display7_c_lower_right_7 = false;
bool aux_display_4bits_7_node_0 = false;
bool aux_display_4bits_7_node_1 = false;
bool aux_display_4bits_7_node_2 = false;
bool aux_display_4bits_7_node_3 = false;
bool aux_display_4bits_7_and_4 = false;
bool aux_display_4bits_7_node_5 = false;
bool aux_display_4bits_7_node_6 = false;
bool aux_display_4bits_7_node_7 = false;
bool aux_display_4bits_7_node_8 = false;
bool aux_display_4bits_7_node_9 = false;
bool aux_display_4bits_7_node_10 = false;
bool aux_display_4bits_7_node_11 = false;
bool aux_display_4bits_7_node_12 = false;
bool aux_display_4bits_7_node_13 = false;
bool aux_display_4bits_7_node_14 = false;
bool aux_display_4bits_7_node_15 = false;
bool aux_display_4bits_7_or_16 = false;
bool aux_display_4bits_7_or_17 = false;
bool aux_display_4bits_7_node_18 = false;
bool aux_display_4bits_7_node_19 = false;
bool aux_display_4bits_7_node_20 = false;
bool aux_display_4bits_7_node_21 = false;
bool aux_display_4bits_7_node_22 = false;
bool aux_display_4bits_7_node_23 = false;
bool aux_display_4bits_7_node_24 = false;
bool aux_display_4bits_7_or_25 = false;
bool aux_display_4bits_7_or_26 = false;
bool aux_display_4bits_7_or_27 = false;
bool aux_display_4bits_7_node_28 = false;
bool aux_display_4bits_7_node_29 = false;
bool aux_display_4bits_7_node_30 = false;
bool aux_display_4bits_7_node_31 = false;
bool aux_display_4bits_7_node_32 = false;
bool aux_display_4bits_7_node_33 = false;
bool aux_display_4bits_7_node_34 = false;
bool aux_display_4bits_7_node_35 = false;
bool aux_display_4bits_7_node_36 = false;
bool aux_display_4bits_7_node_37 = false;
bool aux_display_4bits_7_node_38 = false;
bool aux_display_4bits_7_or_39 = false;
bool aux_display_4bits_7_node_40 = false;
bool aux_display_4bits_7_node_41 = false;
bool aux_display_4bits_7_node_42 = false;
bool aux_display_4bits_7_node_43 = false;
bool aux_display_4bits_7_or_44 = false;
bool aux_display_4bits_7_and_45 = false;
bool aux_display_4bits_7_and_46 = false;
bool aux_display_4bits_7_and_47 = false;
bool aux_display_4bits_7_and_48 = false;
bool aux_display_4bits_7_and_49 = false;
bool aux_display_4bits_7_and_50 = false;
bool aux_display_4bits_7_node_51 = false;
bool aux_display_4bits_7_node_52 = false;
bool aux_display_4bits_7_and_53 = false;
bool aux_display_4bits_7_node_54 = false;
bool aux_display_4bits_7_node_55 = false;
bool aux_display_4bits_7_and_56 = false;
bool aux_display_4bits_7_node_57 = false;
bool aux_display_4bits_7_node_58 = false;
bool aux_display_4bits_7_node_59 = false;
bool aux_display_4bits_7_node_60 = false;
bool aux_display_4bits_7_node_61 = false;
bool aux_display_4bits_7_node_62 = false;
bool aux_display_4bits_7_node_63 = false;
bool aux_display_4bits_7_node_64 = false;
bool aux_display_4bits_7_node_65 = false;
bool aux_display_4bits_7_node_66 = false;
bool aux_display_4bits_7_node_67 = false;
bool aux_display_4bits_7_node_68 = false;
bool aux_display_4bits_7_node_69 = false;
bool aux_display_4bits_7_node_70 = false;
bool aux_display_4bits_7_node_71 = false;
bool aux_display_4bits_7_node_72 = false;
bool aux_display_4bits_7_not_73 = false;
bool aux_display_4bits_7_not_74 = false;
bool aux_display_4bits_7_not_75 = false;
bool aux_display_4bits_7_not_76 = false;
// End IC: DISPLAY-4BITS
bool aux_clock_8 = false;
unsigned long aux_clock_8_lastTime = 0;
const unsigned long aux_clock_8_interval = 250;
bool aux_clock_12 = false;
unsigned long aux_clock_12_lastTime = 0;
const unsigned long aux_clock_12_interval = 500;
bool aux_clock_13 = false;
unsigned long aux_clock_13_lastTime = 0;
const unsigned long aux_clock_13_interval = 1000;
bool aux_clock_14 = false;
unsigned long aux_clock_14_lastTime = 0;
const unsigned long aux_clock_14_interval = 2000;
// IC: INPUT
bool aux_input_15_not_x0_0 = false;
bool aux_input_15_and_0_1 = false;
bool aux_input_15_or_0_2 = false;
bool aux_input_15_xor_0_3 = false;
bool aux_input_15_not_0 = false;
bool aux_input_15_xor_1 = false;
bool aux_input_15_or_2 = false;
bool aux_input_15_and_3 = false;
bool aux_input_15_node_4 = false;
bool aux_input_15_node_5 = false;
bool aux_input_15_node_6 = false;
bool aux_input_15_node_7 = false;
bool aux_input_15_node_8 = false;
bool aux_input_15_node_9 = false;
bool aux_ic_input_input_0 = false;
bool aux_ic_input_input_1 = false;
// End IC: INPUT
// IC: DISPLAY-3BITS
bool aux_display_3bits_16_display7_g_middle = false;
bool aux_display_3bits_16_display7_f_upper_left_1 = false;
bool aux_display_3bits_16_display7_e_lower_left_2 = false;
bool aux_display_3bits_16_display7_d_bottom_3 = false;
bool aux_display_3bits_16_display7_a_top_4 = false;
bool aux_display_3bits_16_display7_b_upper_right_5 = false;
bool aux_display_3bits_16_display7_dp_dot_6 = false;
bool aux_display_3bits_16_display7_c_lower_right_7 = false;
bool aux_display_3bits_16_node_0 = false;
bool aux_display_3bits_16_or_1 = false;
bool aux_display_3bits_16_or_2 = false;
bool aux_display_3bits_16_and_3 = false;
bool aux_display_3bits_16_or_4 = false;
bool aux_display_3bits_16_not_5 = false;
bool aux_display_3bits_16_and_6 = false;
bool aux_display_3bits_16_and_7 = false;
bool aux_display_3bits_16_and_8 = false;
bool aux_display_3bits_16_xnor_9 = false;
bool aux_display_3bits_16_and_10 = false;
bool aux_display_3bits_16_or_11 = false;
bool aux_display_3bits_16_nand_12 = false;
bool aux_display_3bits_16_or_13 = false;
bool aux_display_3bits_16_or_14 = false;
bool aux_display_3bits_16_node_15 = false;
bool aux_display_3bits_16_node_16 = false;
bool aux_display_3bits_16_node_17 = false;
bool aux_display_3bits_16_node_18 = false;
bool aux_display_3bits_16_node_19 = false;
bool aux_display_3bits_16_node_20 = false;
bool aux_display_3bits_16_node_21 = false;
bool aux_display_3bits_16_node_22 = false;
bool aux_display_3bits_16_not_23 = false;
bool aux_display_3bits_16_and_24 = false;
bool aux_display_3bits_16_and_25 = false;
bool aux_display_3bits_16_and_26 = false;
bool aux_display_3bits_16_and_27 = false;
bool aux_display_3bits_16_node_28 = false;
bool aux_display_3bits_16_or_29 = false;
bool aux_display_3bits_16_node_30 = false;
bool aux_display_3bits_16_node_31 = false;
bool aux_display_3bits_16_xnor_32 = false;
bool aux_display_3bits_16_or_34 = false;
bool aux_display_3bits_16_and_35 = false;
bool aux_display_3bits_16_not_36 = false;
bool aux_display_3bits_16_or_37 = false;
bool aux_ic_input_display_3bits_0 = false;
bool aux_ic_input_display_3bits_1 = false;
bool aux_ic_input_display_3bits_2 = false;
// End IC: DISPLAY-3BITS
// IC: JKFLIPFLOP
bool aux_jkflipflop_17_q_0 = false;
bool aux_jkflipflop_17_q_0_1 = false;
// IC: DFLIPFLOP
bool aux_dflipflop_0_led_0 = false;
bool aux_dflipflop_0_led_0_1 = false;
bool aux_dflipflop_0_node_0 = false;
bool aux_dflipflop_0_node_1 = false;
bool aux_dflipflop_0_node_2 = false;
bool aux_dflipflop_0_node_3 = false;
bool aux_dflipflop_0_nand_4 = false;
bool aux_dflipflop_0_node_5 = false;
bool aux_dflipflop_0_nand_6 = false;
bool aux_dflipflop_0_not_7 = false;
bool aux_dflipflop_0_nand_8 = false;
bool aux_dflipflop_0_nand_9 = false;
bool aux_dflipflop_0_nand_10 = false;
bool aux_dflipflop_0_nand_11 = false;
bool aux_dflipflop_0_node_12 = false;
bool aux_dflipflop_0_not_13 = false;
bool aux_dflipflop_0_nand_14 = false;
bool aux_dflipflop_0_nand_15 = false;
bool aux_dflipflop_0_node_16 = false;
bool aux_dflipflop_0_node_17 = false;
bool aux_dflipflop_0_node_18 = false;
bool aux_dflipflop_0_not_19 = false;
bool aux_dflipflop_0_node_20 = false;
bool aux_dflipflop_0_node_21 = false;
bool aux_dflipflop_0_node_22 = false;
bool aux_dflipflop_0_node_23 = false;
bool aux_dflipflop_0_node_24 = false;
bool aux_ic_input_dflipflop_0 = false;
bool aux_ic_input_dflipflop_1 = false;
bool aux_ic_input_dflipflop_2 = false;
bool aux_ic_input_dflipflop_3 = false;
// End IC: DFLIPFLOP
bool aux_jkflipflop_17_node_1 = false;
bool aux_jkflipflop_17_node_2 = false;
bool aux_jkflipflop_17_node_3 = false;
bool aux_jkflipflop_17_node_4 = false;
bool aux_jkflipflop_17_node_5 = false;
bool aux_jkflipflop_17_node_6 = false;
bool aux_jkflipflop_17_node_7 = false;
bool aux_jkflipflop_17_node_8 = false;
bool aux_jkflipflop_17_not_9 = false;
bool aux_jkflipflop_17_and_10 = false;
bool aux_jkflipflop_17_node_11 = false;
bool aux_jkflipflop_17_or_12 = false;
bool aux_jkflipflop_17_node_13 = false;
bool aux_jkflipflop_17_node_14 = false;
bool aux_jkflipflop_17_node_15 = false;
bool aux_jkflipflop_17_node_16 = false;
bool aux_jkflipflop_17_and_17 = false;
bool aux_jkflipflop_17_node_18 = false;
bool aux_jkflipflop_17_node_19 = false;
bool aux_ic_input_jkflipflop_0 = false;
bool aux_ic_input_jkflipflop_1 = false;
bool aux_ic_input_jkflipflop_2 = false;
bool aux_ic_input_jkflipflop_3 = false;
bool aux_ic_input_jkflipflop_4 = false;
// End IC: JKFLIPFLOP
bool aux_clock_18 = false;
unsigned long aux_clock_18_lastTime = 0;
const unsigned long aux_clock_18_interval = 250;
bool aux_jk_flip_flop_19_0_q = false;
bool aux_jk_flip_flop_19_0_q_inclk = false;
bool aux_jk_flip_flop_19_1_q = true;
bool aux_jk_flip_flop_19_1_q_inclk = false;
bool aux_clock_21 = false;
unsigned long aux_clock_21_lastTime = 0;
const unsigned long aux_clock_21_interval = 1000;

void setup() {
    pinMode(seven_segment_display1_g_middle, OUTPUT);
    pinMode(seven_segment_display1_f_upper_left, OUTPUT);
    pinMode(seven_segment_display1_e_lower_left, OUTPUT);
    pinMode(seven_segment_display1_d_bottom, OUTPUT);
    pinMode(seven_segment_display1_a_top, OUTPUT);
    pinMode(seven_segment_display1_b_upper_right, OUTPUT);
    pinMode(seven_segment_display1_dp_dot, OUTPUT);
    pinMode(seven_segment_display1_c_lower_right, OUTPUT);
    pinMode(seven_segment_display5_g_middle, OUTPUT);
    pinMode(seven_segment_display5_f_upper_left, OUTPUT);
    pinMode(seven_segment_display5_e_lower_left, OUTPUT);
    pinMode(seven_segment_display5_d_bottom, OUTPUT);
    pinMode(seven_segment_display5_a_top, OUTPUT);
    pinMode(seven_segment_display5_b_upper_right, OUTPUT);
    pinMode(seven_segment_display5_dp_dot, OUTPUT);
    pinMode(seven_segment_display5_c_lower_right, OUTPUT);
    pinMode(seven_segment_display7_g_middle, OUTPUT);
    pinMode(seven_segment_display7_f_upper_left, OUTPUT);
    pinMode(seven_segment_display7_e_lower_left, OUTPUT);
    pinMode(seven_segment_display7_d_bottom, OUTPUT);
    pinMode(seven_segment_display7_a_top, OUTPUT);
    pinMode(seven_segment_display7_b_upper_right, OUTPUT);
    pinMode(seven_segment_display7_dp_dot, OUTPUT);
    pinMode(seven_segment_display7_c_lower_right, OUTPUT);
    pinMode(led10_0, OUTPUT);
    pinMode(led11_0, OUTPUT);
    pinMode(led12_0, OUTPUT);
    pinMode(led21_0, OUTPUT);
}

void loop() {
    // Read input data

    // Update clocks
    unsigned long now = millis();
    if (now - aux_clock_8_lastTime >= aux_clock_8_interval) {
        aux_clock_8_lastTime = now;
        aux_clock_8 = !aux_clock_8;
    }
    if (now - aux_clock_12_lastTime >= aux_clock_12_interval) {
        aux_clock_12_lastTime = now;
        aux_clock_12 = !aux_clock_12;
    }
    if (now - aux_clock_13_lastTime >= aux_clock_13_interval) {
        aux_clock_13_lastTime = now;
        aux_clock_13 = !aux_clock_13;
    }
    if (now - aux_clock_14_lastTime >= aux_clock_14_interval) {
        aux_clock_14_lastTime = now;
        aux_clock_14 = !aux_clock_14;
    }
    if (now - aux_clock_18_lastTime >= aux_clock_18_interval) {
        aux_clock_18_lastTime = now;
        aux_clock_18 = !aux_clock_18;
    }
    if (now - aux_clock_21_lastTime >= aux_clock_21_interval) {
        aux_clock_21_lastTime = now;
        aux_clock_21 = !aux_clock_21;
    }

    // Update logic variables
    //JK FlipFlop
    if (aux_jk_flip_flop_3_0_q && !aux_jk_flip_flop_1_0_q_inclk) { 
        // Toggle mode (J=1, K=1)
        bool aux = aux_jk_flip_flop_1_0_q;
        aux_jk_flip_flop_1_0_q = aux_jk_flip_flop_1_1_q;
        aux_jk_flip_flop_1_1_q = aux;
    }
    aux_jk_flip_flop_1_0_q_inclk = aux_jk_flip_flop_3_0_q;
    //End of JK FlipFlop
    //JK FlipFlop
    if (aux_jkflipflop_17_q_0 && !aux_jk_flip_flop_2_0_q_inclk) { 
        // Toggle mode (J=1, K=1)
        bool aux = aux_jk_flip_flop_2_0_q;
        aux_jk_flip_flop_2_0_q = aux_jk_flip_flop_2_1_q;
        aux_jk_flip_flop_2_1_q = aux;
    }
    aux_jk_flip_flop_2_0_q_inclk = aux_jkflipflop_17_q_0;
    //End of JK FlipFlop
    //JK FlipFlop
    if (aux_jk_flip_flop_2_0_q && !aux_jk_flip_flop_3_0_q_inclk) { 
        // Toggle mode (J=1, K=1)
        bool aux = aux_jk_flip_flop_3_0_q;
        aux_jk_flip_flop_3_0_q = aux_jk_flip_flop_3_1_q;
        aux_jk_flip_flop_3_1_q = aux;
    }
    aux_jk_flip_flop_3_0_q_inclk = aux_jk_flip_flop_2_0_q;
    //End of JK FlipFlop
    // IC: DISPLAY-4BITS
    aux_ic_input_display_4bits_0 = aux_jk_flip_flop_1_1_q;
    aux_ic_input_display_4bits_1 = aux_jk_flip_flop_3_1_q;
    aux_ic_input_display_4bits_2 = aux_jk_flip_flop_2_1_q;
    aux_ic_input_display_4bits_3 = aux_jkflipflop_17_q_0_1;
    aux_display_4bits_5_node_68 = true;
    aux_display_4bits_5_node_72 = aux_display_4bits_5_node_68;
    aux_display_4bits_5_node_67 = aux_ic_input_display_4bits_2;
    aux_display_4bits_5_node_65 = aux_ic_input_display_4bits_1;
    aux_display_4bits_5_not_76 = !aux_display_4bits_5_node_72;
    aux_display_4bits_5_node_71 = aux_display_4bits_5_node_67;
    aux_display_4bits_5_node_70 = aux_display_4bits_5_node_65;
    aux_display_4bits_5_not_75 = !aux_display_4bits_5_node_71;
    aux_display_4bits_5_not_73 = !aux_display_4bits_5_node_70;
    aux_display_4bits_5_node_66 = aux_ic_input_display_4bits_0;
    aux_display_4bits_5_node_42 = aux_display_4bits_5_node_71;
    aux_display_4bits_5_node_51 = aux_display_4bits_5_not_76;
    aux_display_4bits_5_node_36 = aux_display_4bits_5_not_75;
    aux_display_4bits_5_node_37 = aux_display_4bits_5_node_51;
    aux_display_4bits_5_node_41 = aux_display_4bits_5_node_42;
    aux_display_4bits_5_node_52 = aux_display_4bits_5_not_73;
    aux_display_4bits_5_node_54 = aux_display_4bits_5_node_72;
    aux_display_4bits_5_node_55 = aux_display_4bits_5_node_70;
    aux_display_4bits_5_node_69 = aux_display_4bits_5_node_66;
    aux_display_4bits_5_and_53 = aux_display_4bits_5_node_52 && aux_display_4bits_5_node_51;
    aux_display_4bits_5_node_15 = aux_display_4bits_5_node_37;
    aux_display_4bits_5_node_14 = aux_display_4bits_5_node_41;
    aux_display_4bits_5_node_43 = aux_display_4bits_5_node_69;
    aux_display_4bits_5_node_22 = aux_display_4bits_5_node_55;
    aux_display_4bits_5_node_23 = aux_display_4bits_5_node_36;
    aux_display_4bits_5_node_40 = aux_display_4bits_5_node_54;
    aux_display_4bits_5_and_50 = aux_display_4bits_5_node_36 && aux_display_4bits_5_node_37;
    aux_display_4bits_5_and_47 = aux_display_4bits_5_node_14 && aux_display_4bits_5_node_15;
    aux_display_4bits_5_node_38 = aux_display_4bits_5_node_52;
    aux_display_4bits_5_node_18 = aux_display_4bits_5_node_43;
    aux_display_4bits_5_node_21 = aux_display_4bits_5_and_53;
    aux_display_4bits_5_node_24 = aux_display_4bits_5_node_40;
    aux_display_4bits_5_node_10 = aux_display_4bits_5_node_22;
    aux_display_4bits_5_node_9 = aux_display_4bits_5_node_23;
    aux_display_4bits_5_node_13 = aux_display_4bits_5_node_14;
    aux_display_4bits_5_node_12 = aux_display_4bits_5_node_38;
    aux_display_4bits_5_node_20 = aux_display_4bits_5_node_21;
    aux_display_4bits_5_and_46 = aux_display_4bits_5_node_10 && aux_display_4bits_5_node_9;
    aux_display_4bits_5_node_11 = aux_display_4bits_5_node_24;
    aux_display_4bits_5_node_8 = aux_display_4bits_5_and_47;
    aux_display_4bits_5_node_0 = aux_display_4bits_5_node_18;
    aux_display_4bits_5_node_6 = aux_display_4bits_5_and_50;
    aux_display_4bits_5_node_3 = aux_display_4bits_5_node_15;
    aux_display_4bits_5_node_2 = aux_display_4bits_5_node_10;
    aux_display_4bits_5_node_7 = aux_display_4bits_5_node_8;
    aux_display_4bits_5_node_5 = aux_display_4bits_5_node_6;
    aux_display_4bits_5_and_4 = aux_display_4bits_5_node_2 && aux_display_4bits_5_node_3;
    aux_display_4bits_5_and_45 = aux_display_4bits_5_node_11 && aux_display_4bits_5_and_46;
    aux_display_4bits_5_and_48 = aux_display_4bits_5_node_12 && aux_display_4bits_5_node_13;
    aux_display_4bits_5_and_49 = aux_display_4bits_5_node_41 && aux_display_4bits_5_node_40;
    aux_display_4bits_5_node_19 = aux_display_4bits_5_node_20;
    aux_display_4bits_5_and_56 = aux_display_4bits_5_node_55 && aux_display_4bits_5_node_54;
    aux_display_4bits_5_node_1 = aux_display_4bits_5_node_0;
    aux_display_4bits_5_not_74 = !aux_display_4bits_5_node_69;
    aux_display_4bits_5_or_16 = aux_display_4bits_5_node_7 || aux_display_4bits_5_and_46 || aux_display_4bits_5_node_1 || aux_display_4bits_5_and_48;
    aux_display_4bits_5_or_17 = aux_display_4bits_5_node_5 || aux_display_4bits_5_and_4 || aux_display_4bits_5_and_46 || aux_display_4bits_5_node_0;
    aux_display_4bits_5_or_27 = aux_display_4bits_5_node_18 || aux_display_4bits_5_node_20 || aux_display_4bits_5_node_8 || aux_display_4bits_5_and_48 || aux_display_4bits_5_and_45;
    aux_display_4bits_5_or_39 = aux_display_4bits_5_and_49 || aux_display_4bits_5_node_38 || aux_display_4bits_5_node_6;
    aux_display_4bits_5_or_26 = aux_display_4bits_5_node_19 || aux_display_4bits_5_node_7;
    aux_display_4bits_5_or_25 = aux_display_4bits_5_node_22 || aux_display_4bits_5_node_23 || aux_display_4bits_5_node_24;
    aux_display_4bits_5_or_44 = aux_display_4bits_5_and_56 || aux_display_4bits_5_node_43 || aux_display_4bits_5_node_42 || aux_display_4bits_5_node_21;
    aux_display_4bits_5_node_61 = aux_display_4bits_5_node_12;
    aux_display_4bits_5_node_62 = aux_display_4bits_5_not_74;
    aux_display_4bits_5_node_63 = aux_display_4bits_5_node_1;
    aux_display_4bits_5_node_64 = aux_display_4bits_5_node_3;
    aux_display_4bits_5_node_60 = aux_display_4bits_5_node_2;
    aux_display_4bits_5_node_59 = aux_display_4bits_5_node_9;
    aux_display_4bits_5_node_58 = aux_display_4bits_5_node_13;
    aux_display_4bits_5_node_57 = aux_display_4bits_5_node_11;
    aux_display_4bits_5_node_29 = aux_display_4bits_5_or_17;
    aux_display_4bits_5_node_28 = aux_display_4bits_5_or_16;
    aux_display_4bits_5_node_35 = aux_display_4bits_5_or_25;
    aux_display_4bits_5_node_34 = false;
    aux_display_4bits_5_node_33 = aux_display_4bits_5_or_39;
    aux_display_4bits_5_node_32 = aux_display_4bits_5_or_44;
    aux_display_4bits_5_node_31 = aux_display_4bits_5_or_27;
    aux_display_4bits_5_node_30 = aux_display_4bits_5_or_26;
    aux_display_4bits_5_display7_g_middle = aux_display_4bits_5_node_28;
    aux_display_4bits_5_display7_f_upper_left_1 = aux_display_4bits_5_node_29;
    aux_display_4bits_5_display7_e_lower_left_2 = aux_display_4bits_5_node_30;
    aux_display_4bits_5_display7_d_bottom_3 = aux_display_4bits_5_node_31;
    aux_display_4bits_5_display7_a_top_4 = aux_display_4bits_5_node_32;
    aux_display_4bits_5_display7_b_upper_right_5 = aux_display_4bits_5_node_33;
    aux_display_4bits_5_display7_dp_dot_6 = aux_display_4bits_5_node_34;
    aux_display_4bits_5_display7_c_lower_right_7 = aux_display_4bits_5_node_35;
    // End IC: DISPLAY-4BITS
    // IC: DISPLAY-4BITS
    aux_ic_input_display_4bits_0 = aux_clock_14;
    aux_ic_input_display_4bits_1 = aux_clock_13;
    aux_ic_input_display_4bits_2 = aux_clock_12;
    aux_ic_input_display_4bits_3 = aux_clock_8;
    aux_display_4bits_7_node_68 = true;
    aux_display_4bits_7_node_72 = aux_display_4bits_7_node_68;
    aux_display_4bits_7_node_67 = aux_ic_input_display_4bits_2;
    aux_display_4bits_7_node_65 = aux_ic_input_display_4bits_1;
    aux_display_4bits_7_not_76 = !aux_display_4bits_7_node_72;
    aux_display_4bits_7_node_71 = aux_display_4bits_7_node_67;
    aux_display_4bits_7_node_70 = aux_display_4bits_7_node_65;
    aux_display_4bits_7_not_75 = !aux_display_4bits_7_node_71;
    aux_display_4bits_7_not_73 = !aux_display_4bits_7_node_70;
    aux_display_4bits_7_node_66 = aux_ic_input_display_4bits_0;
    aux_display_4bits_7_node_42 = aux_display_4bits_7_node_71;
    aux_display_4bits_7_node_51 = aux_display_4bits_7_not_76;
    aux_display_4bits_7_node_36 = aux_display_4bits_7_not_75;
    aux_display_4bits_7_node_37 = aux_display_4bits_7_node_51;
    aux_display_4bits_7_node_41 = aux_display_4bits_7_node_42;
    aux_display_4bits_7_node_52 = aux_display_4bits_7_not_73;
    aux_display_4bits_7_node_54 = aux_display_4bits_7_node_72;
    aux_display_4bits_7_node_55 = aux_display_4bits_7_node_70;
    aux_display_4bits_7_node_69 = aux_display_4bits_7_node_66;
    aux_display_4bits_7_and_53 = aux_display_4bits_7_node_52 && aux_display_4bits_7_node_51;
    aux_display_4bits_7_node_15 = aux_display_4bits_7_node_37;
    aux_display_4bits_7_node_14 = aux_display_4bits_7_node_41;
    aux_display_4bits_7_node_43 = aux_display_4bits_7_node_69;
    aux_display_4bits_7_node_22 = aux_display_4bits_7_node_55;
    aux_display_4bits_7_node_23 = aux_display_4bits_7_node_36;
    aux_display_4bits_7_node_40 = aux_display_4bits_7_node_54;
    aux_display_4bits_7_and_50 = aux_display_4bits_7_node_36 && aux_display_4bits_7_node_37;
    aux_display_4bits_7_and_47 = aux_display_4bits_7_node_14 && aux_display_4bits_7_node_15;
    aux_display_4bits_7_node_38 = aux_display_4bits_7_node_52;
    aux_display_4bits_7_node_18 = aux_display_4bits_7_node_43;
    aux_display_4bits_7_node_21 = aux_display_4bits_7_and_53;
    aux_display_4bits_7_node_24 = aux_display_4bits_7_node_40;
    aux_display_4bits_7_node_10 = aux_display_4bits_7_node_22;
    aux_display_4bits_7_node_9 = aux_display_4bits_7_node_23;
    aux_display_4bits_7_node_13 = aux_display_4bits_7_node_14;
    aux_display_4bits_7_node_12 = aux_display_4bits_7_node_38;
    aux_display_4bits_7_node_20 = aux_display_4bits_7_node_21;
    aux_display_4bits_7_and_46 = aux_display_4bits_7_node_10 && aux_display_4bits_7_node_9;
    aux_display_4bits_7_node_11 = aux_display_4bits_7_node_24;
    aux_display_4bits_7_node_8 = aux_display_4bits_7_and_47;
    aux_display_4bits_7_node_0 = aux_display_4bits_7_node_18;
    aux_display_4bits_7_node_6 = aux_display_4bits_7_and_50;
    aux_display_4bits_7_node_3 = aux_display_4bits_7_node_15;
    aux_display_4bits_7_node_2 = aux_display_4bits_7_node_10;
    aux_display_4bits_7_node_7 = aux_display_4bits_7_node_8;
    aux_display_4bits_7_node_5 = aux_display_4bits_7_node_6;
    aux_display_4bits_7_and_4 = aux_display_4bits_7_node_2 && aux_display_4bits_7_node_3;
    aux_display_4bits_7_and_45 = aux_display_4bits_7_node_11 && aux_display_4bits_7_and_46;
    aux_display_4bits_7_and_48 = aux_display_4bits_7_node_12 && aux_display_4bits_7_node_13;
    aux_display_4bits_7_and_49 = aux_display_4bits_7_node_41 && aux_display_4bits_7_node_40;
    aux_display_4bits_7_node_19 = aux_display_4bits_7_node_20;
    aux_display_4bits_7_and_56 = aux_display_4bits_7_node_55 && aux_display_4bits_7_node_54;
    aux_display_4bits_7_node_1 = aux_display_4bits_7_node_0;
    aux_display_4bits_7_not_74 = !aux_display_4bits_7_node_69;
    aux_display_4bits_7_or_16 = aux_display_4bits_7_node_7 || aux_display_4bits_7_and_46 || aux_display_4bits_7_node_1 || aux_display_4bits_7_and_48;
    aux_display_4bits_7_or_17 = aux_display_4bits_7_node_5 || aux_display_4bits_7_and_4 || aux_display_4bits_7_and_46 || aux_display_4bits_7_node_0;
    aux_display_4bits_7_or_27 = aux_display_4bits_7_node_18 || aux_display_4bits_7_node_20 || aux_display_4bits_7_node_8 || aux_display_4bits_7_and_48 || aux_display_4bits_7_and_45;
    aux_display_4bits_7_or_39 = aux_display_4bits_7_and_49 || aux_display_4bits_7_node_38 || aux_display_4bits_7_node_6;
    aux_display_4bits_7_or_26 = aux_display_4bits_7_node_19 || aux_display_4bits_7_node_7;
    aux_display_4bits_7_or_25 = aux_display_4bits_7_node_22 || aux_display_4bits_7_node_23 || aux_display_4bits_7_node_24;
    aux_display_4bits_7_or_44 = aux_display_4bits_7_and_56 || aux_display_4bits_7_node_43 || aux_display_4bits_7_node_42 || aux_display_4bits_7_node_21;
    aux_display_4bits_7_node_61 = aux_display_4bits_7_node_12;
    aux_display_4bits_7_node_62 = aux_display_4bits_7_not_74;
    aux_display_4bits_7_node_63 = aux_display_4bits_7_node_1;
    aux_display_4bits_7_node_64 = aux_display_4bits_7_node_3;
    aux_display_4bits_7_node_60 = aux_display_4bits_7_node_2;
    aux_display_4bits_7_node_59 = aux_display_4bits_7_node_9;
    aux_display_4bits_7_node_58 = aux_display_4bits_7_node_13;
    aux_display_4bits_7_node_57 = aux_display_4bits_7_node_11;
    aux_display_4bits_7_node_29 = aux_display_4bits_7_or_17;
    aux_display_4bits_7_node_28 = aux_display_4bits_7_or_16;
    aux_display_4bits_7_node_35 = aux_display_4bits_7_or_25;
    aux_display_4bits_7_node_34 = false;
    aux_display_4bits_7_node_33 = aux_display_4bits_7_or_39;
    aux_display_4bits_7_node_32 = aux_display_4bits_7_or_44;
    aux_display_4bits_7_node_31 = aux_display_4bits_7_or_27;
    aux_display_4bits_7_node_30 = aux_display_4bits_7_or_26;
    aux_display_4bits_7_display7_g_middle = aux_display_4bits_7_node_28;
    aux_display_4bits_7_display7_f_upper_left_1 = aux_display_4bits_7_node_29;
    aux_display_4bits_7_display7_e_lower_left_2 = aux_display_4bits_7_node_30;
    aux_display_4bits_7_display7_d_bottom_3 = aux_display_4bits_7_node_31;
    aux_display_4bits_7_display7_a_top_4 = aux_display_4bits_7_node_32;
    aux_display_4bits_7_display7_b_upper_right_5 = aux_display_4bits_7_node_33;
    aux_display_4bits_7_display7_dp_dot_6 = aux_display_4bits_7_node_34;
    aux_display_4bits_7_display7_c_lower_right_7 = aux_display_4bits_7_node_35;
    // End IC: DISPLAY-4BITS
    // IC: INPUT
    aux_ic_input_input_0 = aux_clock_14;
    aux_ic_input_input_1 = aux_clock_13;
    aux_input_15_node_4 = aux_ic_input_input_0;
    aux_input_15_node_5 = aux_ic_input_input_1;
    aux_input_15_not_0 = !aux_input_15_node_4;
    aux_input_15_xor_1 = aux_input_15_node_4 ^ aux_input_15_node_5;
    aux_input_15_or_2 = aux_input_15_node_4 || aux_input_15_node_5;
    aux_input_15_and_3 = aux_input_15_node_4 && aux_input_15_node_5;
    aux_input_15_node_6 = aux_input_15_xor_1;
    aux_input_15_node_7 = aux_input_15_not_0;
    aux_input_15_node_8 = aux_input_15_and_3;
    aux_input_15_node_9 = aux_input_15_or_2;
    aux_input_15_not_x0_0 = aux_input_15_node_7;
    aux_input_15_and_0_1 = aux_input_15_node_8;
    aux_input_15_or_0_2 = aux_input_15_node_9;
    aux_input_15_xor_0_3 = aux_input_15_node_6;
    // End IC: INPUT
    // IC: DISPLAY-3BITS
    aux_ic_input_display_3bits_0 = aux_input_15_not_x0_0;
    aux_ic_input_display_3bits_1 = aux_input_15_and_0_1;
    aux_ic_input_display_3bits_2 = aux_input_15_or_0_2;
    aux_display_3bits_16_node_31 = aux_ic_input_display_3bits_1;
    aux_display_3bits_16_node_30 = aux_ic_input_display_3bits_0;
    aux_display_3bits_16_node_28 = aux_ic_input_display_3bits_2;
    aux_display_3bits_16_or_1 = false || aux_display_3bits_16_node_28;
    aux_display_3bits_16_or_37 = false || aux_display_3bits_16_node_30;
    aux_display_3bits_16_or_2 = false || aux_display_3bits_16_node_31;
    aux_display_3bits_16_not_23 = !aux_display_3bits_16_or_1;
    aux_display_3bits_16_not_36 = !aux_display_3bits_16_or_2;
    aux_display_3bits_16_not_5 = !aux_display_3bits_16_or_37;
    aux_display_3bits_16_and_26 = aux_display_3bits_16_not_36 && aux_display_3bits_16_not_23;
    aux_display_3bits_16_and_25 = aux_display_3bits_16_node_31 && aux_display_3bits_16_not_23;
    aux_display_3bits_16_and_24 = aux_display_3bits_16_or_37 && aux_display_3bits_16_not_36 && aux_display_3bits_16_or_1;
    aux_display_3bits_16_and_27 = aux_display_3bits_16_not_5 && aux_display_3bits_16_not_23;
    aux_display_3bits_16_and_3 = aux_display_3bits_16_or_2 && aux_display_3bits_16_not_23;
    aux_display_3bits_16_xnor_32 = !(aux_display_3bits_16_not_5 ^ aux_display_3bits_16_not_23);
    aux_display_3bits_16_and_35 = aux_display_3bits_16_not_5 && aux_display_3bits_16_or_2;
    aux_display_3bits_16_and_6 = aux_display_3bits_16_or_37 && aux_display_3bits_16_not_36;
    aux_display_3bits_16_and_10 = aux_display_3bits_16_or_37 && aux_display_3bits_16_not_23;
    aux_display_3bits_16_xnor_9 = !(aux_display_3bits_16_not_36 ^ aux_display_3bits_16_not_23);
    aux_display_3bits_16_and_8 = aux_display_3bits_16_not_5 && aux_display_3bits_16_not_36 && aux_display_3bits_16_not_23;
    aux_display_3bits_16_and_7 = aux_display_3bits_16_or_2 && aux_display_3bits_16_not_23;
    aux_display_3bits_16_node_0 = false;
    aux_display_3bits_16_or_34 = aux_display_3bits_16_xnor_32 || aux_display_3bits_16_or_2;
    aux_display_3bits_16_or_4 = aux_display_3bits_16_and_7 || aux_display_3bits_16_and_8;
    aux_display_3bits_16_or_29 = aux_display_3bits_16_and_27 || aux_display_3bits_16_and_35 || aux_display_3bits_16_and_3 || aux_display_3bits_16_and_24;
    aux_display_3bits_16_or_14 = aux_display_3bits_16_not_5 || aux_display_3bits_16_xnor_9;
    aux_display_3bits_16_or_13 = aux_display_3bits_16_and_6 || aux_display_3bits_16_and_35 || aux_display_3bits_16_and_25;
    aux_display_3bits_16_nand_12 = !(aux_display_3bits_16_not_5 && aux_display_3bits_16_or_2 && aux_display_3bits_16_not_23);
    aux_display_3bits_16_or_11 = aux_display_3bits_16_and_26 || aux_display_3bits_16_and_6 || aux_display_3bits_16_and_10;
    aux_display_3bits_16_node_22 = aux_display_3bits_16_nand_12;
    aux_display_3bits_16_node_21 = aux_display_3bits_16_node_0;
    aux_display_3bits_16_node_20 = aux_display_3bits_16_or_14;
    aux_display_3bits_16_node_19 = aux_display_3bits_16_or_34;
    aux_display_3bits_16_node_18 = aux_display_3bits_16_or_29;
    aux_display_3bits_16_node_17 = aux_display_3bits_16_or_4;
    aux_display_3bits_16_node_16 = aux_display_3bits_16_or_11;
    aux_display_3bits_16_node_15 = aux_display_3bits_16_or_13;
    aux_display_3bits_16_display7_g_middle = aux_display_3bits_16_node_15;
    aux_display_3bits_16_display7_f_upper_left_1 = aux_display_3bits_16_node_16;
    aux_display_3bits_16_display7_e_lower_left_2 = aux_display_3bits_16_node_17;
    aux_display_3bits_16_display7_d_bottom_3 = aux_display_3bits_16_node_18;
    aux_display_3bits_16_display7_a_top_4 = aux_display_3bits_16_node_19;
    aux_display_3bits_16_display7_b_upper_right_5 = aux_display_3bits_16_node_20;
    aux_display_3bits_16_display7_dp_dot_6 = aux_display_3bits_16_node_21;
    aux_display_3bits_16_display7_c_lower_right_7 = aux_display_3bits_16_node_22;
    // End IC: DISPLAY-3BITS
    // IC: JKFLIPFLOP
    aux_ic_input_jkflipflop_0 = true;
    aux_ic_input_jkflipflop_1 = true;
    aux_ic_input_jkflipflop_2 = aux_clock_18;
    aux_ic_input_jkflipflop_3 = true;
    aux_ic_input_jkflipflop_4 = true;
    aux_jkflipflop_17_node_13 = true;
    aux_jkflipflop_17_node_11 = true;
    aux_jkflipflop_17_node_8 = aux_ic_input_jkflipflop_2;
    aux_jkflipflop_17_node_1 = aux_jkflipflop_17_node_11;
    aux_jkflipflop_17_node_3 = aux_jkflipflop_17_node_8;
    aux_jkflipflop_17_node_2 = aux_jkflipflop_17_node_13;
    // IC: DFLIPFLOP
    aux_ic_input_dflipflop_0 = aux_jkflipflop_17_node_1;
    aux_ic_input_dflipflop_1 = aux_jkflipflop_17_or_12;
    aux_ic_input_dflipflop_2 = aux_jkflipflop_17_node_3;
    aux_ic_input_dflipflop_3 = aux_jkflipflop_17_node_2;
    aux_dflipflop_0_node_18 = aux_ic_input_dflipflop_2;
    aux_dflipflop_0_not_19 = !aux_dflipflop_0_node_18;
    aux_dflipflop_0_not_7 = !aux_dflipflop_0_not_19;
    aux_dflipflop_0_node_20 = aux_dflipflop_0_not_19;
    aux_dflipflop_0_node_1 = aux_dflipflop_0_not_7;
    aux_dflipflop_0_node_2 = aux_dflipflop_0_node_20;
    aux_dflipflop_0_node_3 = aux_ic_input_dflipflop_1;
    aux_dflipflop_0_node_16 = aux_ic_input_dflipflop_0;
    aux_dflipflop_0_node_17 = aux_ic_input_dflipflop_3;
    aux_dflipflop_0_not_13 = !aux_dflipflop_0_node_3;
    aux_dflipflop_0_node_21 = aux_dflipflop_0_node_1;
    aux_dflipflop_0_nand_4 = !(aux_dflipflop_0_node_3 && aux_dflipflop_0_node_2);
    aux_dflipflop_0_nand_9 = !(aux_dflipflop_0_node_16 && aux_dflipflop_0_nand_4 && aux_dflipflop_0_nand_8);
    aux_dflipflop_0_node_22 = aux_dflipflop_0_node_17;
    aux_dflipflop_0_node_0 = aux_dflipflop_0_node_21;
    aux_dflipflop_0_nand_15 = !(aux_dflipflop_0_node_20 && aux_dflipflop_0_not_13);
    aux_dflipflop_0_node_24 = aux_dflipflop_0_node_16;
    aux_dflipflop_0_nand_10 = !(aux_dflipflop_0_nand_9 && aux_dflipflop_0_node_0);
    aux_dflipflop_0_nand_8 = !(aux_dflipflop_0_nand_9 && aux_dflipflop_0_nand_15 && aux_dflipflop_0_node_22);
    aux_dflipflop_0_nand_14 = !(aux_dflipflop_0_node_21 && aux_dflipflop_0_nand_8);
    aux_dflipflop_0_nand_6 = !(aux_dflipflop_0_node_24 && aux_dflipflop_0_nand_10 && aux_dflipflop_0_nand_11);
    aux_dflipflop_0_node_23 = aux_dflipflop_0_node_17;
    aux_dflipflop_0_nand_11 = !(aux_dflipflop_0_nand_6 && aux_dflipflop_0_nand_14 && aux_dflipflop_0_node_23);
    aux_dflipflop_0_node_12 = aux_dflipflop_0_nand_11;
    aux_dflipflop_0_node_5 = aux_dflipflop_0_nand_6;
    aux_dflipflop_0_led_0 = aux_dflipflop_0_node_5;
    aux_dflipflop_0_led_0_1 = aux_dflipflop_0_node_12;
    // End IC: DFLIPFLOP
    aux_jkflipflop_17_node_6 = aux_dflipflop_0_led_0_1;
    aux_jkflipflop_17_node_7 = aux_dflipflop_0_led_0;
    aux_jkflipflop_17_node_14 = true;
    aux_jkflipflop_17_node_15 = true;
    aux_jkflipflop_17_node_4 = aux_jkflipflop_17_node_7;
    aux_jkflipflop_17_node_5 = aux_jkflipflop_17_node_6;
    aux_jkflipflop_17_not_9 = !aux_jkflipflop_17_node_14;
    aux_jkflipflop_17_node_16 = aux_jkflipflop_17_node_15;
    aux_jkflipflop_17_and_17 = aux_jkflipflop_17_node_4 && aux_jkflipflop_17_not_9;
    aux_jkflipflop_17_and_10 = aux_jkflipflop_17_node_16 && aux_jkflipflop_17_node_5;
    aux_jkflipflop_17_or_12 = aux_jkflipflop_17_and_17 || aux_jkflipflop_17_and_10;
    aux_jkflipflop_17_node_18 = aux_dflipflop_0_led_0;
    aux_jkflipflop_17_node_19 = aux_dflipflop_0_led_0_1;
    aux_jkflipflop_17_q_0 = aux_jkflipflop_17_node_18;
    aux_jkflipflop_17_q_0_1 = aux_jkflipflop_17_node_19;
    // End IC: JKFLIPFLOP
    //JK FlipFlop
    if (aux_clock_21 && !aux_jk_flip_flop_19_0_q_inclk) { 
        // Toggle mode (J=1, K=1)
        bool aux = aux_jk_flip_flop_19_0_q;
        aux_jk_flip_flop_19_0_q = aux_jk_flip_flop_19_1_q;
        aux_jk_flip_flop_19_1_q = aux;
    }
    aux_jk_flip_flop_19_0_q_inclk = aux_clock_21;
    //End of JK FlipFlop

    // Write output data
    digitalWrite(seven_segment_display1_g_middle, aux_display_3bits_16_display7_g_middle);
    digitalWrite(seven_segment_display1_f_upper_left, aux_display_3bits_16_display7_f_upper_left_1);
    digitalWrite(seven_segment_display1_e_lower_left, aux_display_3bits_16_display7_e_lower_left_2);
    digitalWrite(seven_segment_display1_d_bottom, aux_display_3bits_16_display7_d_bottom_3);
    digitalWrite(seven_segment_display1_a_top, aux_display_3bits_16_display7_a_top_4);
    digitalWrite(seven_segment_display1_b_upper_right, aux_display_3bits_16_display7_b_upper_right_5);
    digitalWrite(seven_segment_display1_dp_dot, aux_display_3bits_16_display7_dp_dot_6);
    digitalWrite(seven_segment_display1_c_lower_right, aux_display_3bits_16_display7_c_lower_right_7);
    digitalWrite(seven_segment_display5_g_middle, aux_display_4bits_5_display7_g_middle);
    digitalWrite(seven_segment_display5_f_upper_left, aux_display_4bits_5_display7_f_upper_left_1);
    digitalWrite(seven_segment_display5_e_lower_left, aux_display_4bits_5_display7_e_lower_left_2);
    digitalWrite(seven_segment_display5_d_bottom, aux_display_4bits_5_display7_d_bottom_3);
    digitalWrite(seven_segment_display5_a_top, aux_display_4bits_5_display7_a_top_4);
    digitalWrite(seven_segment_display5_b_upper_right, aux_display_4bits_5_display7_b_upper_right_5);
    digitalWrite(seven_segment_display5_dp_dot, aux_display_4bits_5_display7_dp_dot_6);
    digitalWrite(seven_segment_display5_c_lower_right, aux_display_4bits_5_display7_c_lower_right_7);
    digitalWrite(seven_segment_display7_g_middle, aux_display_4bits_7_display7_g_middle);
    digitalWrite(seven_segment_display7_f_upper_left, aux_display_4bits_7_display7_f_upper_left_1);
    digitalWrite(seven_segment_display7_e_lower_left, aux_display_4bits_7_display7_e_lower_left_2);
    digitalWrite(seven_segment_display7_d_bottom, aux_display_4bits_7_display7_d_bottom_3);
    digitalWrite(seven_segment_display7_a_top, aux_display_4bits_7_display7_a_top_4);
    digitalWrite(seven_segment_display7_b_upper_right, aux_display_4bits_7_display7_b_upper_right_5);
    digitalWrite(seven_segment_display7_dp_dot, aux_display_4bits_7_display7_dp_dot_6);
    digitalWrite(seven_segment_display7_c_lower_right, aux_display_4bits_7_display7_c_lower_right_7);
    digitalWrite(led10_0, aux_input_15_and_0_1);
    digitalWrite(led11_0, aux_input_15_not_x0_0);
    digitalWrite(led12_0, aux_input_15_or_0_2);
    digitalWrite(led21_0, aux_jk_flip_flop_19_0_q);
}
