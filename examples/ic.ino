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
const int led16_0 = A0;
const int led17_0 = A1;
const int led18_0 = A2;
const int seven_segment_display19_g_middle = A3;
const int seven_segment_display19_f_upper_left = A4;
const int seven_segment_display19_e_lower_left = A5;
const int seven_segment_display19_d_bottom = A6;
const int seven_segment_display19_a_top = A7;
const int seven_segment_display19_b_upper_right = A8;
const int seven_segment_display19_dp_dot = A9;
const int seven_segment_display19_c_lower_right = A10;
const int seven_segment_display20_g_middle = A11;
const int seven_segment_display20_f_upper_left = A12;
const int seven_segment_display20_e_lower_left = A13;
const int seven_segment_display20_d_bottom = A14;
const int seven_segment_display20_a_top = A15;
const int seven_segment_display20_b_upper_right = 2;
const int seven_segment_display20_dp_dot = 3;
const int seven_segment_display20_c_lower_right = 4;
const int led21_0 = 5;
const int seven_segment_display22_g_middle = 6;
const int seven_segment_display22_f_upper_left = 7;
const int seven_segment_display22_e_lower_left = 8;
const int seven_segment_display22_d_bottom = 9;
const int seven_segment_display22_a_top = 10;
const int seven_segment_display22_b_upper_right = 11;
const int seven_segment_display22_dp_dot = 12;
const int seven_segment_display22_c_lower_right = 13;

/* ====== Aux. Variables ====== */
bool aux_clock_0 = false;
unsigned long aux_clock_0_lastTime = 0;
const unsigned long aux_clock_0_interval = 250;
// IC: JKFLIPFLOP
bool aux_ic_1_q_0 = false;
bool aux_ic_1_q_0_1 = false;
// IC: DFLIPFLOP
bool aux_ic_0_led_0 = false;
bool aux_ic_0_led_0_1 = false;
bool aux_ic_0_node_0 = false;
bool aux_ic_0_node_1 = false;
bool aux_ic_0_node_2 = false;
bool aux_ic_0_node_3 = false;
bool aux_ic_0_nand_4 = false;
bool aux_ic_0_node_5 = false;
bool aux_ic_0_nand_6 = false;
bool aux_ic_0_not_7 = false;
bool aux_ic_0_nand_8 = false;
bool aux_ic_0_nand_9 = false;
bool aux_ic_0_nand_10 = false;
bool aux_ic_0_nand_11 = false;
bool aux_ic_0_node_12 = false;
bool aux_ic_0_not_13 = false;
bool aux_ic_0_nand_14 = false;
bool aux_ic_0_nand_15 = false;
bool aux_ic_0_node_16 = false;
bool aux_ic_0_node_17 = false;
bool aux_ic_0_node_18 = false;
bool aux_ic_0_not_19 = false;
bool aux_ic_0_node_20 = false;
bool aux_ic_0_node_21 = false;
bool aux_ic_0_node_22 = false;
bool aux_ic_0_node_23 = false;
bool aux_ic_0_node_24 = false;
bool aux_ic_input_ic_0 = false;
bool aux_ic_input_ic_1 = false;
bool aux_ic_input_ic_2 = false;
bool aux_ic_input_ic_3 = false;
// End IC: DFLIPFLOP
bool aux_ic_1_node_1 = false;
bool aux_ic_1_node_2 = false;
bool aux_ic_1_node_3 = false;
bool aux_ic_1_node_4 = false;
bool aux_ic_1_node_5 = false;
bool aux_ic_1_node_6 = false;
bool aux_ic_1_node_7 = false;
bool aux_ic_1_node_8 = false;
bool aux_ic_1_not_9 = false;
bool aux_ic_1_and_10 = false;
bool aux_ic_1_node_11 = false;
bool aux_ic_1_or_12 = false;
bool aux_ic_1_node_13 = false;
bool aux_ic_1_node_14 = false;
bool aux_ic_1_node_15 = false;
bool aux_ic_1_node_16 = false;
bool aux_ic_1_and_17 = false;
bool aux_ic_1_node_18 = false;
bool aux_ic_1_node_19 = false;
bool aux_ic_input_ic_4 = false;
// End IC: JKFLIPFLOP
bool aux_jk_flip_flop_2_0_q = false;
bool aux_jk_flip_flop_2_0_q_inclk = false;
bool aux_jk_flip_flop_2_1_q = true;
bool aux_jk_flip_flop_2_1_q_inclk = false;
bool aux_jk_flip_flop_3_0_q = false;
bool aux_jk_flip_flop_3_0_q_inclk = false;
bool aux_jk_flip_flop_3_1_q = true;
bool aux_jk_flip_flop_3_1_q_inclk = false;
bool aux_clock_4 = false;
unsigned long aux_clock_4_lastTime = 0;
const unsigned long aux_clock_4_interval = 2000;
bool aux_clock_5 = false;
unsigned long aux_clock_5_lastTime = 0;
const unsigned long aux_clock_5_interval = 1000;
bool aux_jk_flip_flop_6_0_q = false;
bool aux_jk_flip_flop_6_0_q_inclk = false;
bool aux_jk_flip_flop_6_1_q = true;
bool aux_jk_flip_flop_6_1_q_inclk = false;
bool aux_clock_7 = false;
unsigned long aux_clock_7_lastTime = 0;
const unsigned long aux_clock_7_interval = 1000;
// IC: INPUT
bool aux_ic_8_not_x0_0 = false;
bool aux_ic_8_and_0_1 = false;
bool aux_ic_8_or_0_2 = false;
bool aux_ic_8_xor_0_3 = false;
bool aux_ic_8_not_0 = false;
bool aux_ic_8_xor_1 = false;
bool aux_ic_8_or_2 = false;
bool aux_ic_8_and_3 = false;
bool aux_ic_8_node_4 = false;
bool aux_ic_8_node_5 = false;
bool aux_ic_8_node_6 = false;
bool aux_ic_8_node_7 = false;
bool aux_ic_8_node_8 = false;
bool aux_ic_8_node_9 = false;
// End IC: INPUT
bool aux_clock_9 = false;
unsigned long aux_clock_9_lastTime = 0;
const unsigned long aux_clock_9_interval = 500;
bool aux_clock_10 = false;
unsigned long aux_clock_10_lastTime = 0;
const unsigned long aux_clock_10_interval = 250;
// IC: DISPLAY-4BITS
bool aux_ic_11_display7_g_middle = false;
bool aux_ic_11_display7_f_upper_left_1 = false;
bool aux_ic_11_display7_e_lower_left_2 = false;
bool aux_ic_11_display7_d_bottom_3 = false;
bool aux_ic_11_display7_a_top_4 = false;
bool aux_ic_11_display7_b_upper_right_5 = false;
bool aux_ic_11_display7_dp_dot_6 = false;
bool aux_ic_11_display7_c_lower_right_7 = false;
bool aux_ic_11_node_0 = false;
bool aux_ic_11_node_1 = false;
bool aux_ic_11_node_2 = false;
bool aux_ic_11_node_3 = false;
bool aux_ic_11_and_4 = false;
bool aux_ic_11_node_5 = false;
bool aux_ic_11_node_6 = false;
bool aux_ic_11_node_7 = false;
bool aux_ic_11_node_8 = false;
bool aux_ic_11_node_9 = false;
bool aux_ic_11_node_10 = false;
bool aux_ic_11_node_11 = false;
bool aux_ic_11_node_12 = false;
bool aux_ic_11_node_13 = false;
bool aux_ic_11_node_14 = false;
bool aux_ic_11_node_15 = false;
bool aux_ic_11_or_16 = false;
bool aux_ic_11_or_17 = false;
bool aux_ic_11_node_18 = false;
bool aux_ic_11_node_19 = false;
bool aux_ic_11_node_20 = false;
bool aux_ic_11_node_21 = false;
bool aux_ic_11_node_22 = false;
bool aux_ic_11_node_23 = false;
bool aux_ic_11_node_24 = false;
bool aux_ic_11_or_25 = false;
bool aux_ic_11_or_26 = false;
bool aux_ic_11_or_27 = false;
bool aux_ic_11_node_28 = false;
bool aux_ic_11_node_29 = false;
bool aux_ic_11_node_30 = false;
bool aux_ic_11_node_31 = false;
bool aux_ic_11_node_32 = false;
bool aux_ic_11_node_33 = false;
bool aux_ic_11_node_34 = false;
bool aux_ic_11_node_35 = false;
bool aux_ic_11_node_36 = false;
bool aux_ic_11_node_37 = false;
bool aux_ic_11_node_38 = false;
bool aux_ic_11_or_39 = false;
bool aux_ic_11_node_40 = false;
bool aux_ic_11_node_41 = false;
bool aux_ic_11_node_42 = false;
bool aux_ic_11_node_43 = false;
bool aux_ic_11_or_44 = false;
bool aux_ic_11_and_45 = false;
bool aux_ic_11_and_46 = false;
bool aux_ic_11_and_47 = false;
bool aux_ic_11_and_48 = false;
bool aux_ic_11_and_49 = false;
bool aux_ic_11_and_50 = false;
bool aux_ic_11_node_51 = false;
bool aux_ic_11_node_52 = false;
bool aux_ic_11_and_53 = false;
bool aux_ic_11_node_54 = false;
bool aux_ic_11_node_55 = false;
bool aux_ic_11_and_56 = false;
bool aux_ic_11_node_57 = false;
bool aux_ic_11_node_58 = false;
bool aux_ic_11_node_59 = false;
bool aux_ic_11_node_60 = false;
bool aux_ic_11_node_61 = false;
bool aux_ic_11_node_62 = false;
bool aux_ic_11_node_63 = false;
bool aux_ic_11_node_64 = false;
bool aux_ic_11_node_65 = false;
bool aux_ic_11_node_66 = false;
bool aux_ic_11_node_67 = false;
bool aux_ic_11_node_68 = false;
bool aux_ic_11_node_69 = false;
bool aux_ic_11_node_70 = false;
bool aux_ic_11_node_71 = false;
bool aux_ic_11_node_72 = false;
bool aux_ic_11_not_73 = false;
bool aux_ic_11_not_74 = false;
bool aux_ic_11_not_75 = false;
bool aux_ic_11_not_76 = false;
// End IC: DISPLAY-4BITS
// IC: DISPLAY-3BITS
bool aux_ic_12_display7_g_middle = false;
bool aux_ic_12_display7_f_upper_left_1 = false;
bool aux_ic_12_display7_e_lower_left_2 = false;
bool aux_ic_12_display7_d_bottom_3 = false;
bool aux_ic_12_display7_a_top_4 = false;
bool aux_ic_12_display7_b_upper_right_5 = false;
bool aux_ic_12_display7_dp_dot_6 = false;
bool aux_ic_12_display7_c_lower_right_7 = false;
bool aux_ic_12_node_0 = false;
bool aux_ic_12_or_1 = false;
bool aux_ic_12_or_2 = false;
bool aux_ic_12_and_3 = false;
bool aux_ic_12_or_4 = false;
bool aux_ic_12_not_5 = false;
bool aux_ic_12_and_6 = false;
bool aux_ic_12_and_7 = false;
bool aux_ic_12_and_8 = false;
bool aux_ic_12_xnor_9 = false;
bool aux_ic_12_and_10 = false;
bool aux_ic_12_or_11 = false;
bool aux_ic_12_nand_12 = false;
bool aux_ic_12_or_13 = false;
bool aux_ic_12_or_14 = false;
bool aux_ic_12_node_15 = false;
bool aux_ic_12_node_16 = false;
bool aux_ic_12_node_17 = false;
bool aux_ic_12_node_18 = false;
bool aux_ic_12_node_19 = false;
bool aux_ic_12_node_20 = false;
bool aux_ic_12_node_21 = false;
bool aux_ic_12_node_22 = false;
bool aux_ic_12_not_23 = false;
bool aux_ic_12_and_24 = false;
bool aux_ic_12_and_25 = false;
bool aux_ic_12_and_26 = false;
bool aux_ic_12_and_27 = false;
bool aux_ic_12_node_28 = false;
bool aux_ic_12_or_29 = false;
bool aux_ic_12_node_30 = false;
bool aux_ic_12_node_31 = false;
bool aux_ic_12_xnor_32 = false;
bool aux_ic_12_or_34 = false;
bool aux_ic_12_and_35 = false;
bool aux_ic_12_not_36 = false;
bool aux_ic_12_or_37 = false;
// End IC: DISPLAY-3BITS
// IC: DISPLAY-4BITS
bool aux_ic_13_display7_g_middle = false;
bool aux_ic_13_display7_f_upper_left_1 = false;
bool aux_ic_13_display7_e_lower_left_2 = false;
bool aux_ic_13_display7_d_bottom_3 = false;
bool aux_ic_13_display7_a_top_4 = false;
bool aux_ic_13_display7_b_upper_right_5 = false;
bool aux_ic_13_display7_dp_dot_6 = false;
bool aux_ic_13_display7_c_lower_right_7 = false;
bool aux_ic_13_node_0 = false;
bool aux_ic_13_node_1 = false;
bool aux_ic_13_node_2 = false;
bool aux_ic_13_node_3 = false;
bool aux_ic_13_and_4 = false;
bool aux_ic_13_node_5 = false;
bool aux_ic_13_node_6 = false;
bool aux_ic_13_node_7 = false;
bool aux_ic_13_node_8 = false;
bool aux_ic_13_node_9 = false;
bool aux_ic_13_node_10 = false;
bool aux_ic_13_node_11 = false;
bool aux_ic_13_node_12 = false;
bool aux_ic_13_node_13 = false;
bool aux_ic_13_node_14 = false;
bool aux_ic_13_node_15 = false;
bool aux_ic_13_or_16 = false;
bool aux_ic_13_or_17 = false;
bool aux_ic_13_node_18 = false;
bool aux_ic_13_node_19 = false;
bool aux_ic_13_node_20 = false;
bool aux_ic_13_node_21 = false;
bool aux_ic_13_node_22 = false;
bool aux_ic_13_node_23 = false;
bool aux_ic_13_node_24 = false;
bool aux_ic_13_or_25 = false;
bool aux_ic_13_or_26 = false;
bool aux_ic_13_or_27 = false;
bool aux_ic_13_node_28 = false;
bool aux_ic_13_node_29 = false;
bool aux_ic_13_node_30 = false;
bool aux_ic_13_node_31 = false;
bool aux_ic_13_node_32 = false;
bool aux_ic_13_node_33 = false;
bool aux_ic_13_node_34 = false;
bool aux_ic_13_node_35 = false;
bool aux_ic_13_node_36 = false;
bool aux_ic_13_node_37 = false;
bool aux_ic_13_node_38 = false;
bool aux_ic_13_or_39 = false;
bool aux_ic_13_node_40 = false;
bool aux_ic_13_node_41 = false;
bool aux_ic_13_node_42 = false;
bool aux_ic_13_node_43 = false;
bool aux_ic_13_or_44 = false;
bool aux_ic_13_and_45 = false;
bool aux_ic_13_and_46 = false;
bool aux_ic_13_and_47 = false;
bool aux_ic_13_and_48 = false;
bool aux_ic_13_and_49 = false;
bool aux_ic_13_and_50 = false;
bool aux_ic_13_node_51 = false;
bool aux_ic_13_node_52 = false;
bool aux_ic_13_and_53 = false;
bool aux_ic_13_node_54 = false;
bool aux_ic_13_node_55 = false;
bool aux_ic_13_and_56 = false;
bool aux_ic_13_node_57 = false;
bool aux_ic_13_node_58 = false;
bool aux_ic_13_node_59 = false;
bool aux_ic_13_node_60 = false;
bool aux_ic_13_node_61 = false;
bool aux_ic_13_node_62 = false;
bool aux_ic_13_node_63 = false;
bool aux_ic_13_node_64 = false;
bool aux_ic_13_node_65 = false;
bool aux_ic_13_node_66 = false;
bool aux_ic_13_node_67 = false;
bool aux_ic_13_node_68 = false;
bool aux_ic_13_node_69 = false;
bool aux_ic_13_node_70 = false;
bool aux_ic_13_node_71 = false;
bool aux_ic_13_node_72 = false;
bool aux_ic_13_not_73 = false;
bool aux_ic_13_not_74 = false;
bool aux_ic_13_not_75 = false;
bool aux_ic_13_not_76 = false;
// End IC: DISPLAY-4BITS
bool aux_jk_flip_flop_14_0_q = false;
bool aux_jk_flip_flop_14_0_q_inclk = false;
bool aux_jk_flip_flop_14_1_q = true;
bool aux_jk_flip_flop_14_1_q_inclk = false;

void setup() {
    pinMode(led16_0, OUTPUT);
    pinMode(led17_0, OUTPUT);
    pinMode(led18_0, OUTPUT);
    pinMode(seven_segment_display19_g_middle, OUTPUT);
    pinMode(seven_segment_display19_f_upper_left, OUTPUT);
    pinMode(seven_segment_display19_e_lower_left, OUTPUT);
    pinMode(seven_segment_display19_d_bottom, OUTPUT);
    pinMode(seven_segment_display19_a_top, OUTPUT);
    pinMode(seven_segment_display19_b_upper_right, OUTPUT);
    pinMode(seven_segment_display19_dp_dot, OUTPUT);
    pinMode(seven_segment_display19_c_lower_right, OUTPUT);
    pinMode(seven_segment_display20_g_middle, OUTPUT);
    pinMode(seven_segment_display20_f_upper_left, OUTPUT);
    pinMode(seven_segment_display20_e_lower_left, OUTPUT);
    pinMode(seven_segment_display20_d_bottom, OUTPUT);
    pinMode(seven_segment_display20_a_top, OUTPUT);
    pinMode(seven_segment_display20_b_upper_right, OUTPUT);
    pinMode(seven_segment_display20_dp_dot, OUTPUT);
    pinMode(seven_segment_display20_c_lower_right, OUTPUT);
    pinMode(led21_0, OUTPUT);
    pinMode(seven_segment_display22_g_middle, OUTPUT);
    pinMode(seven_segment_display22_f_upper_left, OUTPUT);
    pinMode(seven_segment_display22_e_lower_left, OUTPUT);
    pinMode(seven_segment_display22_d_bottom, OUTPUT);
    pinMode(seven_segment_display22_a_top, OUTPUT);
    pinMode(seven_segment_display22_b_upper_right, OUTPUT);
    pinMode(seven_segment_display22_dp_dot, OUTPUT);
    pinMode(seven_segment_display22_c_lower_right, OUTPUT);
}

void loop() {
    // Read input data

    // Update clocks
    unsigned long now = millis();
    if (now - aux_clock_0_lastTime >= aux_clock_0_interval) {
        aux_clock_0_lastTime = now;
        aux_clock_0 = !aux_clock_0;
    }
    if (now - aux_clock_4_lastTime >= aux_clock_4_interval) {
        aux_clock_4_lastTime = now;
        aux_clock_4 = !aux_clock_4;
    }
    if (now - aux_clock_5_lastTime >= aux_clock_5_interval) {
        aux_clock_5_lastTime = now;
        aux_clock_5 = !aux_clock_5;
    }
    if (now - aux_clock_7_lastTime >= aux_clock_7_interval) {
        aux_clock_7_lastTime = now;
        aux_clock_7 = !aux_clock_7;
    }
    if (now - aux_clock_9_lastTime >= aux_clock_9_interval) {
        aux_clock_9_lastTime = now;
        aux_clock_9 = !aux_clock_9;
    }
    if (now - aux_clock_10_lastTime >= aux_clock_10_interval) {
        aux_clock_10_lastTime = now;
        aux_clock_10 = !aux_clock_10;
    }

    // Update logic variables
    // IC: JKFLIPFLOP
    aux_ic_input_ic_0 = true;
    aux_ic_input_ic_1 = true;
    aux_ic_input_ic_2 = aux_clock_0;
    aux_ic_input_ic_3 = true;
    aux_ic_input_ic_4 = true;
    aux_ic_1_node_13 = true;
    aux_ic_1_node_11 = true;
    aux_ic_1_node_8 = aux_ic_input_ic_2;
    aux_ic_1_node_1 = aux_ic_1_node_11;
    aux_ic_1_node_3 = aux_ic_1_node_8;
    aux_ic_1_node_2 = aux_ic_1_node_13;
    // IC: DFLIPFLOP
    aux_ic_input_ic_0 = aux_ic_1_node_1;
    aux_ic_input_ic_1 = aux_ic_1_or_12;
    aux_ic_input_ic_2 = aux_ic_1_node_3;
    aux_ic_input_ic_3 = aux_ic_1_node_2;
    aux_ic_0_node_18 = aux_ic_input_ic_2;
    aux_ic_0_not_19 = !aux_ic_0_node_18;
    aux_ic_0_not_7 = !aux_ic_0_not_19;
    aux_ic_0_node_20 = aux_ic_0_not_19;
    aux_ic_0_node_1 = aux_ic_0_not_7;
    aux_ic_0_node_2 = aux_ic_0_node_20;
    aux_ic_0_node_3 = aux_ic_input_ic_1;
    aux_ic_0_node_16 = aux_ic_input_ic_0;
    aux_ic_0_node_17 = aux_ic_input_ic_3;
    aux_ic_0_not_13 = !aux_ic_0_node_3;
    aux_ic_0_node_21 = aux_ic_0_node_1;
    aux_ic_0_nand_4 = !(aux_ic_0_node_3 && aux_ic_0_node_2);
    aux_ic_0_nand_9 = !(aux_ic_0_node_16 && aux_ic_0_nand_4 && aux_ic_0_nand_8);
    aux_ic_0_node_22 = aux_ic_0_node_17;
    aux_ic_0_node_0 = aux_ic_0_node_21;
    aux_ic_0_nand_15 = !(aux_ic_0_node_20 && aux_ic_0_not_13);
    aux_ic_0_node_24 = aux_ic_0_node_16;
    aux_ic_0_nand_10 = !(aux_ic_0_nand_9 && aux_ic_0_node_0);
    aux_ic_0_nand_8 = !(aux_ic_0_nand_9 && aux_ic_0_nand_15 && aux_ic_0_node_22);
    aux_ic_0_nand_14 = !(aux_ic_0_node_21 && aux_ic_0_nand_8);
    aux_ic_0_nand_6 = !(aux_ic_0_node_24 && aux_ic_0_nand_10 && aux_ic_0_nand_11);
    aux_ic_0_node_23 = aux_ic_0_node_17;
    aux_ic_0_nand_11 = !(aux_ic_0_nand_6 && aux_ic_0_nand_14 && aux_ic_0_node_23);
    aux_ic_0_node_12 = aux_ic_0_nand_11;
    aux_ic_0_node_5 = aux_ic_0_nand_6;
    aux_ic_0_led_0 = aux_ic_0_node_5;
    aux_ic_0_led_0_1 = aux_ic_0_node_12;
    // End IC: DFLIPFLOP
    aux_ic_1_node_6 = aux_ic_0_led_0_1;
    aux_ic_1_node_7 = aux_ic_0_led_0;
    aux_ic_1_node_14 = true;
    aux_ic_1_node_15 = true;
    aux_ic_1_node_4 = aux_ic_1_node_7;
    aux_ic_1_node_5 = aux_ic_1_node_6;
    aux_ic_1_not_9 = !aux_ic_1_node_14;
    aux_ic_1_node_16 = aux_ic_1_node_15;
    aux_ic_1_and_17 = aux_ic_1_node_4 && aux_ic_1_not_9;
    aux_ic_1_and_10 = aux_ic_1_node_16 && aux_ic_1_node_5;
    aux_ic_1_or_12 = aux_ic_1_and_17 || aux_ic_1_and_10;
    aux_ic_1_node_18 = aux_ic_0_led_0;
    aux_ic_1_node_19 = aux_ic_0_led_0_1;
    aux_ic_1_q_0 = aux_ic_1_node_18;
    aux_ic_1_q_0_1 = aux_ic_1_node_19;
    // End IC: JKFLIPFLOP
    //JK FlipFlop
    if (aux_ic_1_q_0 && !aux_jk_flip_flop_2_0_q_inclk) { 
        // Toggle mode (J=1, K=1)
        bool aux = aux_jk_flip_flop_2_0_q;
        aux_jk_flip_flop_2_0_q = aux_jk_flip_flop_2_1_q;
        aux_jk_flip_flop_2_1_q = aux;
    }
    aux_jk_flip_flop_2_0_q_inclk = aux_ic_1_q_0;
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
    //JK FlipFlop
    if (aux_jk_flip_flop_3_0_q && !aux_jk_flip_flop_6_0_q_inclk) { 
        // Toggle mode (J=1, K=1)
        bool aux = aux_jk_flip_flop_6_0_q;
        aux_jk_flip_flop_6_0_q = aux_jk_flip_flop_6_1_q;
        aux_jk_flip_flop_6_1_q = aux;
    }
    aux_jk_flip_flop_6_0_q_inclk = aux_jk_flip_flop_3_0_q;
    //End of JK FlipFlop
    // IC: INPUT
    aux_ic_input_ic_0 = aux_clock_4;
    aux_ic_input_ic_1 = aux_clock_5;
    aux_ic_8_node_4 = aux_ic_input_ic_0;
    aux_ic_8_node_5 = aux_ic_input_ic_1;
    aux_ic_8_not_0 = !aux_ic_8_node_4;
    aux_ic_8_xor_1 = aux_ic_8_node_4 ^ aux_ic_8_node_5;
    aux_ic_8_or_2 = aux_ic_8_node_4 || aux_ic_8_node_5;
    aux_ic_8_and_3 = aux_ic_8_node_4 && aux_ic_8_node_5;
    aux_ic_8_node_6 = aux_ic_8_xor_1;
    aux_ic_8_node_7 = aux_ic_8_not_0;
    aux_ic_8_node_8 = aux_ic_8_and_3;
    aux_ic_8_node_9 = aux_ic_8_or_2;
    aux_ic_8_not_x0_0 = aux_ic_8_node_7;
    aux_ic_8_and_0_1 = aux_ic_8_node_8;
    aux_ic_8_or_0_2 = aux_ic_8_node_9;
    aux_ic_8_xor_0_3 = aux_ic_8_node_6;
    // End IC: INPUT
    // IC: DISPLAY-4BITS
    aux_ic_input_ic_0 = aux_clock_4;
    aux_ic_input_ic_1 = aux_clock_5;
    aux_ic_input_ic_2 = aux_clock_9;
    aux_ic_input_ic_3 = aux_clock_10;
    aux_ic_11_node_68 = true;
    aux_ic_11_node_72 = aux_ic_11_node_68;
    aux_ic_11_node_67 = aux_ic_input_ic_2;
    aux_ic_11_node_65 = aux_ic_input_ic_1;
    aux_ic_11_not_76 = !aux_ic_11_node_72;
    aux_ic_11_node_71 = aux_ic_11_node_67;
    aux_ic_11_node_70 = aux_ic_11_node_65;
    aux_ic_11_not_75 = !aux_ic_11_node_71;
    aux_ic_11_not_73 = !aux_ic_11_node_70;
    aux_ic_11_node_66 = aux_ic_input_ic_0;
    aux_ic_11_node_42 = aux_ic_11_node_71;
    aux_ic_11_node_51 = aux_ic_11_not_76;
    aux_ic_11_node_36 = aux_ic_11_not_75;
    aux_ic_11_node_37 = aux_ic_11_node_51;
    aux_ic_11_node_41 = aux_ic_11_node_42;
    aux_ic_11_node_52 = aux_ic_11_not_73;
    aux_ic_11_node_54 = aux_ic_11_node_72;
    aux_ic_11_node_55 = aux_ic_11_node_70;
    aux_ic_11_node_69 = aux_ic_11_node_66;
    aux_ic_11_and_53 = aux_ic_11_node_52 && aux_ic_11_node_51;
    aux_ic_11_node_15 = aux_ic_11_node_37;
    aux_ic_11_node_14 = aux_ic_11_node_41;
    aux_ic_11_node_43 = aux_ic_11_node_69;
    aux_ic_11_node_22 = aux_ic_11_node_55;
    aux_ic_11_node_23 = aux_ic_11_node_36;
    aux_ic_11_node_40 = aux_ic_11_node_54;
    aux_ic_11_and_50 = aux_ic_11_node_36 && aux_ic_11_node_37;
    aux_ic_11_and_47 = aux_ic_11_node_14 && aux_ic_11_node_15;
    aux_ic_11_node_38 = aux_ic_11_node_52;
    aux_ic_11_node_18 = aux_ic_11_node_43;
    aux_ic_11_node_21 = aux_ic_11_and_53;
    aux_ic_11_node_24 = aux_ic_11_node_40;
    aux_ic_11_node_10 = aux_ic_11_node_22;
    aux_ic_11_node_9 = aux_ic_11_node_23;
    aux_ic_11_node_13 = aux_ic_11_node_14;
    aux_ic_11_node_12 = aux_ic_11_node_38;
    aux_ic_11_node_20 = aux_ic_11_node_21;
    aux_ic_11_and_46 = aux_ic_11_node_10 && aux_ic_11_node_9;
    aux_ic_11_node_11 = aux_ic_11_node_24;
    aux_ic_11_node_8 = aux_ic_11_and_47;
    aux_ic_11_node_0 = aux_ic_11_node_18;
    aux_ic_11_node_6 = aux_ic_11_and_50;
    aux_ic_11_node_3 = aux_ic_11_node_15;
    aux_ic_11_node_2 = aux_ic_11_node_10;
    aux_ic_11_node_7 = aux_ic_11_node_8;
    aux_ic_11_node_5 = aux_ic_11_node_6;
    aux_ic_11_and_4 = aux_ic_11_node_2 && aux_ic_11_node_3;
    aux_ic_11_and_45 = aux_ic_11_node_11 && aux_ic_11_and_46;
    aux_ic_11_and_48 = aux_ic_11_node_12 && aux_ic_11_node_13;
    aux_ic_11_and_49 = aux_ic_11_node_41 && aux_ic_11_node_40;
    aux_ic_11_node_19 = aux_ic_11_node_20;
    aux_ic_11_and_56 = aux_ic_11_node_55 && aux_ic_11_node_54;
    aux_ic_11_node_1 = aux_ic_11_node_0;
    aux_ic_11_not_74 = !aux_ic_11_node_69;
    aux_ic_11_or_16 = aux_ic_11_node_7 || aux_ic_11_and_46 || aux_ic_11_node_1 || aux_ic_11_and_48;
    aux_ic_11_or_17 = aux_ic_11_node_5 || aux_ic_11_and_4 || aux_ic_11_and_46 || aux_ic_11_node_0;
    aux_ic_11_or_27 = aux_ic_11_node_18 || aux_ic_11_node_20 || aux_ic_11_node_8 || aux_ic_11_and_48 || aux_ic_11_and_45;
    aux_ic_11_or_39 = aux_ic_11_and_49 || aux_ic_11_node_38 || aux_ic_11_node_6;
    aux_ic_11_or_26 = aux_ic_11_node_19 || aux_ic_11_node_7;
    aux_ic_11_or_25 = aux_ic_11_node_22 || aux_ic_11_node_23 || aux_ic_11_node_24;
    aux_ic_11_or_44 = aux_ic_11_and_56 || aux_ic_11_node_43 || aux_ic_11_node_42 || aux_ic_11_node_21;
    aux_ic_11_node_61 = aux_ic_11_node_12;
    aux_ic_11_node_62 = aux_ic_11_not_74;
    aux_ic_11_node_63 = aux_ic_11_node_1;
    aux_ic_11_node_64 = aux_ic_11_node_3;
    aux_ic_11_node_60 = aux_ic_11_node_2;
    aux_ic_11_node_59 = aux_ic_11_node_9;
    aux_ic_11_node_58 = aux_ic_11_node_13;
    aux_ic_11_node_57 = aux_ic_11_node_11;
    aux_ic_11_node_29 = aux_ic_11_or_17;
    aux_ic_11_node_28 = aux_ic_11_or_16;
    aux_ic_11_node_35 = aux_ic_11_or_25;
    aux_ic_11_node_34 = false;
    aux_ic_11_node_33 = aux_ic_11_or_39;
    aux_ic_11_node_32 = aux_ic_11_or_44;
    aux_ic_11_node_31 = aux_ic_11_or_27;
    aux_ic_11_node_30 = aux_ic_11_or_26;
    aux_ic_11_display7_g_middle = aux_ic_11_node_28;
    aux_ic_11_display7_f_upper_left_1 = aux_ic_11_node_29;
    aux_ic_11_display7_e_lower_left_2 = aux_ic_11_node_30;
    aux_ic_11_display7_d_bottom_3 = aux_ic_11_node_31;
    aux_ic_11_display7_a_top_4 = aux_ic_11_node_32;
    aux_ic_11_display7_b_upper_right_5 = aux_ic_11_node_33;
    aux_ic_11_display7_dp_dot_6 = aux_ic_11_node_34;
    aux_ic_11_display7_c_lower_right_7 = aux_ic_11_node_35;
    // End IC: DISPLAY-4BITS
    // IC: DISPLAY-3BITS
    aux_ic_input_ic_0 = aux_ic_8_not_x0_0;
    aux_ic_input_ic_1 = aux_ic_8_and_0_1;
    aux_ic_input_ic_2 = aux_ic_8_or_0_2;
    aux_ic_12_node_31 = aux_ic_input_ic_1;
    aux_ic_12_node_30 = aux_ic_input_ic_0;
    aux_ic_12_node_28 = aux_ic_input_ic_2;
    aux_ic_12_or_1 = false || aux_ic_12_node_28;
    aux_ic_12_or_37 = false || aux_ic_12_node_30;
    aux_ic_12_or_2 = false || aux_ic_12_node_31;
    aux_ic_12_not_23 = !aux_ic_12_or_1;
    aux_ic_12_not_36 = !aux_ic_12_or_2;
    aux_ic_12_not_5 = !aux_ic_12_or_37;
    aux_ic_12_and_26 = aux_ic_12_not_36 && aux_ic_12_not_23;
    aux_ic_12_and_25 = aux_ic_12_node_31 && aux_ic_12_not_23;
    aux_ic_12_and_24 = aux_ic_12_or_37 && aux_ic_12_not_36 && aux_ic_12_or_1;
    aux_ic_12_and_27 = aux_ic_12_not_5 && aux_ic_12_not_23;
    aux_ic_12_and_3 = aux_ic_12_or_2 && aux_ic_12_not_23;
    aux_ic_12_xnor_32 = !(aux_ic_12_not_5 ^ aux_ic_12_not_23);
    aux_ic_12_and_35 = aux_ic_12_not_5 && aux_ic_12_or_2;
    aux_ic_12_and_6 = aux_ic_12_or_37 && aux_ic_12_not_36;
    aux_ic_12_and_10 = aux_ic_12_or_37 && aux_ic_12_not_23;
    aux_ic_12_xnor_9 = !(aux_ic_12_not_36 ^ aux_ic_12_not_23);
    aux_ic_12_and_8 = aux_ic_12_not_5 && aux_ic_12_not_36 && aux_ic_12_not_23;
    aux_ic_12_and_7 = aux_ic_12_or_2 && aux_ic_12_not_23;
    aux_ic_12_node_0 = false;
    aux_ic_12_or_34 = aux_ic_12_xnor_32 || aux_ic_12_or_2;
    aux_ic_12_or_4 = aux_ic_12_and_7 || aux_ic_12_and_8;
    aux_ic_12_or_29 = aux_ic_12_and_27 || aux_ic_12_and_35 || aux_ic_12_and_3 || aux_ic_12_and_24;
    aux_ic_12_or_14 = aux_ic_12_not_5 || aux_ic_12_xnor_9;
    aux_ic_12_or_13 = aux_ic_12_and_6 || aux_ic_12_and_35 || aux_ic_12_and_25;
    aux_ic_12_nand_12 = !(aux_ic_12_not_5 && aux_ic_12_or_2 && aux_ic_12_not_23);
    aux_ic_12_or_11 = aux_ic_12_and_26 || aux_ic_12_and_6 || aux_ic_12_and_10;
    aux_ic_12_node_22 = aux_ic_12_nand_12;
    aux_ic_12_node_21 = aux_ic_12_node_0;
    aux_ic_12_node_20 = aux_ic_12_or_14;
    aux_ic_12_node_19 = aux_ic_12_or_34;
    aux_ic_12_node_18 = aux_ic_12_or_29;
    aux_ic_12_node_17 = aux_ic_12_or_4;
    aux_ic_12_node_16 = aux_ic_12_or_11;
    aux_ic_12_node_15 = aux_ic_12_or_13;
    aux_ic_12_display7_g_middle = aux_ic_12_node_15;
    aux_ic_12_display7_f_upper_left_1 = aux_ic_12_node_16;
    aux_ic_12_display7_e_lower_left_2 = aux_ic_12_node_17;
    aux_ic_12_display7_d_bottom_3 = aux_ic_12_node_18;
    aux_ic_12_display7_a_top_4 = aux_ic_12_node_19;
    aux_ic_12_display7_b_upper_right_5 = aux_ic_12_node_20;
    aux_ic_12_display7_dp_dot_6 = aux_ic_12_node_21;
    aux_ic_12_display7_c_lower_right_7 = aux_ic_12_node_22;
    // End IC: DISPLAY-3BITS
    // IC: DISPLAY-4BITS
    aux_ic_input_ic_0 = aux_jk_flip_flop_6_1_q;
    aux_ic_input_ic_1 = aux_jk_flip_flop_3_1_q;
    aux_ic_input_ic_2 = aux_jk_flip_flop_2_1_q;
    aux_ic_input_ic_3 = aux_ic_1_q_0_1;
    aux_ic_13_node_68 = true;
    aux_ic_13_node_72 = aux_ic_13_node_68;
    aux_ic_13_node_67 = aux_ic_input_ic_2;
    aux_ic_13_node_65 = aux_ic_input_ic_1;
    aux_ic_13_not_76 = !aux_ic_13_node_72;
    aux_ic_13_node_71 = aux_ic_13_node_67;
    aux_ic_13_node_70 = aux_ic_13_node_65;
    aux_ic_13_not_75 = !aux_ic_13_node_71;
    aux_ic_13_not_73 = !aux_ic_13_node_70;
    aux_ic_13_node_66 = aux_ic_input_ic_0;
    aux_ic_13_node_42 = aux_ic_13_node_71;
    aux_ic_13_node_51 = aux_ic_13_not_76;
    aux_ic_13_node_36 = aux_ic_13_not_75;
    aux_ic_13_node_37 = aux_ic_13_node_51;
    aux_ic_13_node_41 = aux_ic_13_node_42;
    aux_ic_13_node_52 = aux_ic_13_not_73;
    aux_ic_13_node_54 = aux_ic_13_node_72;
    aux_ic_13_node_55 = aux_ic_13_node_70;
    aux_ic_13_node_69 = aux_ic_13_node_66;
    aux_ic_13_and_53 = aux_ic_13_node_52 && aux_ic_13_node_51;
    aux_ic_13_node_15 = aux_ic_13_node_37;
    aux_ic_13_node_14 = aux_ic_13_node_41;
    aux_ic_13_node_43 = aux_ic_13_node_69;
    aux_ic_13_node_22 = aux_ic_13_node_55;
    aux_ic_13_node_23 = aux_ic_13_node_36;
    aux_ic_13_node_40 = aux_ic_13_node_54;
    aux_ic_13_and_50 = aux_ic_13_node_36 && aux_ic_13_node_37;
    aux_ic_13_and_47 = aux_ic_13_node_14 && aux_ic_13_node_15;
    aux_ic_13_node_38 = aux_ic_13_node_52;
    aux_ic_13_node_18 = aux_ic_13_node_43;
    aux_ic_13_node_21 = aux_ic_13_and_53;
    aux_ic_13_node_24 = aux_ic_13_node_40;
    aux_ic_13_node_10 = aux_ic_13_node_22;
    aux_ic_13_node_9 = aux_ic_13_node_23;
    aux_ic_13_node_13 = aux_ic_13_node_14;
    aux_ic_13_node_12 = aux_ic_13_node_38;
    aux_ic_13_node_20 = aux_ic_13_node_21;
    aux_ic_13_and_46 = aux_ic_13_node_10 && aux_ic_13_node_9;
    aux_ic_13_node_11 = aux_ic_13_node_24;
    aux_ic_13_node_8 = aux_ic_13_and_47;
    aux_ic_13_node_0 = aux_ic_13_node_18;
    aux_ic_13_node_6 = aux_ic_13_and_50;
    aux_ic_13_node_3 = aux_ic_13_node_15;
    aux_ic_13_node_2 = aux_ic_13_node_10;
    aux_ic_13_node_7 = aux_ic_13_node_8;
    aux_ic_13_node_5 = aux_ic_13_node_6;
    aux_ic_13_and_4 = aux_ic_13_node_2 && aux_ic_13_node_3;
    aux_ic_13_and_45 = aux_ic_13_node_11 && aux_ic_13_and_46;
    aux_ic_13_and_48 = aux_ic_13_node_12 && aux_ic_13_node_13;
    aux_ic_13_and_49 = aux_ic_13_node_41 && aux_ic_13_node_40;
    aux_ic_13_node_19 = aux_ic_13_node_20;
    aux_ic_13_and_56 = aux_ic_13_node_55 && aux_ic_13_node_54;
    aux_ic_13_node_1 = aux_ic_13_node_0;
    aux_ic_13_not_74 = !aux_ic_13_node_69;
    aux_ic_13_or_16 = aux_ic_13_node_7 || aux_ic_13_and_46 || aux_ic_13_node_1 || aux_ic_13_and_48;
    aux_ic_13_or_17 = aux_ic_13_node_5 || aux_ic_13_and_4 || aux_ic_13_and_46 || aux_ic_13_node_0;
    aux_ic_13_or_27 = aux_ic_13_node_18 || aux_ic_13_node_20 || aux_ic_13_node_8 || aux_ic_13_and_48 || aux_ic_13_and_45;
    aux_ic_13_or_39 = aux_ic_13_and_49 || aux_ic_13_node_38 || aux_ic_13_node_6;
    aux_ic_13_or_26 = aux_ic_13_node_19 || aux_ic_13_node_7;
    aux_ic_13_or_25 = aux_ic_13_node_22 || aux_ic_13_node_23 || aux_ic_13_node_24;
    aux_ic_13_or_44 = aux_ic_13_and_56 || aux_ic_13_node_43 || aux_ic_13_node_42 || aux_ic_13_node_21;
    aux_ic_13_node_61 = aux_ic_13_node_12;
    aux_ic_13_node_62 = aux_ic_13_not_74;
    aux_ic_13_node_63 = aux_ic_13_node_1;
    aux_ic_13_node_64 = aux_ic_13_node_3;
    aux_ic_13_node_60 = aux_ic_13_node_2;
    aux_ic_13_node_59 = aux_ic_13_node_9;
    aux_ic_13_node_58 = aux_ic_13_node_13;
    aux_ic_13_node_57 = aux_ic_13_node_11;
    aux_ic_13_node_29 = aux_ic_13_or_17;
    aux_ic_13_node_28 = aux_ic_13_or_16;
    aux_ic_13_node_35 = aux_ic_13_or_25;
    aux_ic_13_node_34 = false;
    aux_ic_13_node_33 = aux_ic_13_or_39;
    aux_ic_13_node_32 = aux_ic_13_or_44;
    aux_ic_13_node_31 = aux_ic_13_or_27;
    aux_ic_13_node_30 = aux_ic_13_or_26;
    aux_ic_13_display7_g_middle = aux_ic_13_node_28;
    aux_ic_13_display7_f_upper_left_1 = aux_ic_13_node_29;
    aux_ic_13_display7_e_lower_left_2 = aux_ic_13_node_30;
    aux_ic_13_display7_d_bottom_3 = aux_ic_13_node_31;
    aux_ic_13_display7_a_top_4 = aux_ic_13_node_32;
    aux_ic_13_display7_b_upper_right_5 = aux_ic_13_node_33;
    aux_ic_13_display7_dp_dot_6 = aux_ic_13_node_34;
    aux_ic_13_display7_c_lower_right_7 = aux_ic_13_node_35;
    // End IC: DISPLAY-4BITS
    //JK FlipFlop
    if (aux_clock_7 && !aux_jk_flip_flop_14_0_q_inclk) { 
        // Toggle mode (J=1, K=1)
        bool aux = aux_jk_flip_flop_14_0_q;
        aux_jk_flip_flop_14_0_q = aux_jk_flip_flop_14_1_q;
        aux_jk_flip_flop_14_1_q = aux;
    }
    aux_jk_flip_flop_14_0_q_inclk = aux_clock_7;
    //End of JK FlipFlop

    // Write output data
    digitalWrite(led16_0, aux_ic_8_not_x0_0);
    digitalWrite(led17_0, aux_ic_8_or_0_2);
    digitalWrite(led18_0, aux_ic_8_and_0_1);
    digitalWrite(seven_segment_display19_g_middle, aux_ic_11_display7_g_middle);
    digitalWrite(seven_segment_display19_f_upper_left, aux_ic_11_display7_f_upper_left_1);
    digitalWrite(seven_segment_display19_e_lower_left, aux_ic_11_display7_e_lower_left_2);
    digitalWrite(seven_segment_display19_d_bottom, aux_ic_11_display7_d_bottom_3);
    digitalWrite(seven_segment_display19_a_top, aux_ic_11_display7_a_top_4);
    digitalWrite(seven_segment_display19_b_upper_right, aux_ic_11_display7_b_upper_right_5);
    digitalWrite(seven_segment_display19_dp_dot, aux_ic_11_display7_dp_dot_6);
    digitalWrite(seven_segment_display19_c_lower_right, aux_ic_11_display7_c_lower_right_7);
    digitalWrite(seven_segment_display20_g_middle, aux_ic_13_display7_g_middle);
    digitalWrite(seven_segment_display20_f_upper_left, aux_ic_13_display7_f_upper_left_1);
    digitalWrite(seven_segment_display20_e_lower_left, aux_ic_13_display7_e_lower_left_2);
    digitalWrite(seven_segment_display20_d_bottom, aux_ic_13_display7_d_bottom_3);
    digitalWrite(seven_segment_display20_a_top, aux_ic_13_display7_a_top_4);
    digitalWrite(seven_segment_display20_b_upper_right, aux_ic_13_display7_b_upper_right_5);
    digitalWrite(seven_segment_display20_dp_dot, aux_ic_13_display7_dp_dot_6);
    digitalWrite(seven_segment_display20_c_lower_right, aux_ic_13_display7_c_lower_right_7);
    digitalWrite(led21_0, aux_jk_flip_flop_14_0_q);
    digitalWrite(seven_segment_display22_g_middle, aux_ic_12_display7_g_middle);
    digitalWrite(seven_segment_display22_f_upper_left, aux_ic_12_display7_f_upper_left_1);
    digitalWrite(seven_segment_display22_e_lower_left, aux_ic_12_display7_e_lower_left_2);
    digitalWrite(seven_segment_display22_d_bottom, aux_ic_12_display7_d_bottom_3);
    digitalWrite(seven_segment_display22_a_top, aux_ic_12_display7_a_top_4);
    digitalWrite(seven_segment_display22_b_upper_right, aux_ic_12_display7_b_upper_right_5);
    digitalWrite(seven_segment_display22_dp_dot, aux_ic_12_display7_dp_dot_6);
    digitalWrite(seven_segment_display22_c_lower_right, aux_ic_12_display7_c_lower_right_7);
}
