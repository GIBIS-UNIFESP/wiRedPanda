// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //
//
// Target Board: Arduino UNO R3/R4
// Pin Usage: 9/18 pins
// Standard Arduino board with 20 I/O pins (14 digital + 6 analog)
//


/* ========= Inputs ========== */
const int push_button1_btn = A0;

/* ========= Outputs ========== */
const int seven_segment_display16_g_middle = A1;
const int seven_segment_display16_f_upper_left = A2;
const int seven_segment_display16_e_lower_left = A3;
const int seven_segment_display16_d_bottom = A4;
const int seven_segment_display16_a_top = A5;
const int seven_segment_display16_b_upper_right = 2;
const int seven_segment_display16_dp_dot = 3;
const int seven_segment_display16_c_lower_right = 4;

/* ====== Aux. Variables ====== */
bool push_button1_btn_val = false;
bool aux_clock_0 = false;
unsigned long aux_clock_0_lastTime = 0;
const unsigned long aux_clock_0_interval = 200;
bool aux_node_1 = false;
bool aux_push_button_2 = false;
bool aux_t_flip_flop_3_0_q = false;
bool aux_t_flip_flop_3_0_q_inclk = false;
bool aux_t_flip_flop_3_1_q = true;
bool aux_t_flip_flop_3_1_q_inclk = false;
bool aux_node_4 = false;
bool aux_t_flip_flop_5_0_q = false;
bool aux_t_flip_flop_5_0_q_inclk = false;
bool aux_t_flip_flop_5_1_q = true;
bool aux_t_flip_flop_5_1_q_inclk = false;
bool aux_node_6 = false;
bool aux_t_flip_flop_7_0_q = false;
bool aux_t_flip_flop_7_0_q_inclk = false;
bool aux_t_flip_flop_7_1_q = true;
bool aux_t_flip_flop_7_1_q_inclk = false;
bool aux_node_8 = false;
bool aux_node_9 = false;
bool aux_node_10 = false;
bool aux_node_11 = false;
bool aux_node_12 = false;
bool aux_node_13 = false;
// IC: DISPLAY-3BITS
bool aux_display_3bits_14_display7_g_middle = false;
bool aux_display_3bits_14_display7_f_upper_left_1 = false;
bool aux_display_3bits_14_display7_e_lower_left_2 = false;
bool aux_display_3bits_14_display7_d_bottom_3 = false;
bool aux_display_3bits_14_display7_a_top_4 = false;
bool aux_display_3bits_14_display7_b_upper_right_5 = false;
bool aux_display_3bits_14_display7_dp_dot_6 = false;
bool aux_display_3bits_14_display7_c_lower_right_7 = false;
bool aux_display_3bits_14_node_0 = false;
bool aux_display_3bits_14_or_1 = false;
bool aux_display_3bits_14_or_2 = false;
bool aux_display_3bits_14_and_3 = false;
bool aux_display_3bits_14_or_4 = false;
bool aux_display_3bits_14_not_5 = false;
bool aux_display_3bits_14_and_6 = false;
bool aux_display_3bits_14_and_7 = false;
bool aux_display_3bits_14_and_8 = false;
bool aux_display_3bits_14_xnor_9 = false;
bool aux_display_3bits_14_and_10 = false;
bool aux_display_3bits_14_or_11 = false;
bool aux_display_3bits_14_nand_12 = false;
bool aux_display_3bits_14_or_13 = false;
bool aux_display_3bits_14_or_14 = false;
bool aux_display_3bits_14_node_15 = false;
bool aux_display_3bits_14_node_16 = false;
bool aux_display_3bits_14_node_17 = false;
bool aux_display_3bits_14_node_18 = false;
bool aux_display_3bits_14_node_19 = false;
bool aux_display_3bits_14_node_20 = false;
bool aux_display_3bits_14_node_21 = false;
bool aux_display_3bits_14_node_22 = false;
bool aux_display_3bits_14_not_23 = false;
bool aux_display_3bits_14_and_24 = false;
bool aux_display_3bits_14_and_25 = false;
bool aux_display_3bits_14_and_26 = false;
bool aux_display_3bits_14_and_27 = false;
bool aux_display_3bits_14_node_28 = false;
bool aux_display_3bits_14_or_29 = false;
bool aux_display_3bits_14_node_30 = false;
bool aux_display_3bits_14_node_31 = false;
bool aux_display_3bits_14_xnor_32 = false;
bool aux_display_3bits_14_or_34 = false;
bool aux_display_3bits_14_and_35 = false;
bool aux_display_3bits_14_not_36 = false;
bool aux_display_3bits_14_or_37 = false;
bool aux_ic_input_display_3bits_0 = false;
bool aux_ic_input_display_3bits_1 = false;
bool aux_ic_input_display_3bits_2 = false;
// End IC: DISPLAY-3BITS

void setup() {
    pinMode(push_button1_btn, INPUT);
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
    push_button1_btn_val = digitalRead(push_button1_btn);

    // Update clocks
    unsigned long now = millis();
    if (now - aux_clock_0_lastTime >= aux_clock_0_interval) {
        aux_clock_0_lastTime = now;
        aux_clock_0 = !aux_clock_0;
    }

    // Update logic variables
    aux_node_1 = aux_clock_0;
    aux_push_button_2 = push_button1_btn_val;
    //T FlipFlop
    if (aux_node_1 && !aux_t_flip_flop_3_0_q_inclk) { 
        if (aux_push_button_2) { 
            aux_t_flip_flop_3_0_q = !aux_t_flip_flop_3_0_q;
            aux_t_flip_flop_3_1_q = !aux_t_flip_flop_3_0_q;
        }
    }
    if (!true) { 
        aux_t_flip_flop_3_0_q = true; // Preset
        aux_t_flip_flop_3_1_q = false;
    } else if (!true) { 
        aux_t_flip_flop_3_0_q = false; // Clear
        aux_t_flip_flop_3_1_q = true;
    }
    aux_t_flip_flop_3_0_q_inclk = aux_node_1;
    //End of T FlipFlop
    aux_node_4 = aux_push_button_2;
    //T FlipFlop
    if (aux_t_flip_flop_3_1_q && !aux_t_flip_flop_5_0_q_inclk) { 
        if (aux_node_4) { 
            aux_t_flip_flop_5_0_q = !aux_t_flip_flop_5_0_q;
            aux_t_flip_flop_5_1_q = !aux_t_flip_flop_5_0_q;
        }
    }
    if (!true) { 
        aux_t_flip_flop_5_0_q = true; // Preset
        aux_t_flip_flop_5_1_q = false;
    } else if (!true) { 
        aux_t_flip_flop_5_0_q = false; // Clear
        aux_t_flip_flop_5_1_q = true;
    }
    aux_t_flip_flop_5_0_q_inclk = aux_t_flip_flop_3_1_q;
    //End of T FlipFlop
    aux_node_6 = aux_node_4;
    //T FlipFlop
    if (aux_t_flip_flop_5_1_q && !aux_t_flip_flop_7_0_q_inclk) { 
        if (aux_node_6) { 
            aux_t_flip_flop_7_0_q = !aux_t_flip_flop_7_0_q;
            aux_t_flip_flop_7_1_q = !aux_t_flip_flop_7_0_q;
        }
    }
    if (!true) { 
        aux_t_flip_flop_7_0_q = true; // Preset
        aux_t_flip_flop_7_1_q = false;
    } else if (!true) { 
        aux_t_flip_flop_7_0_q = false; // Clear
        aux_t_flip_flop_7_1_q = true;
    }
    aux_t_flip_flop_7_0_q_inclk = aux_t_flip_flop_5_1_q;
    //End of T FlipFlop
    aux_node_8 = aux_t_flip_flop_7_1_q;
    aux_node_9 = aux_node_8;
    aux_node_10 = aux_clock_0;
    aux_node_11 = aux_t_flip_flop_3_1_q;
    aux_node_12 = aux_t_flip_flop_5_1_q;
    aux_node_13 = aux_node_10;
    // IC: DISPLAY-3BITS
    aux_ic_input_display_3bits_0 = aux_node_9;
    aux_ic_input_display_3bits_1 = aux_node_12;
    aux_ic_input_display_3bits_2 = aux_node_11;
    aux_display_3bits_14_node_28 = aux_ic_input_display_3bits_2;
    aux_display_3bits_14_node_30 = aux_ic_input_display_3bits_0;
    aux_display_3bits_14_node_31 = aux_ic_input_display_3bits_1;
    aux_display_3bits_14_or_1 = false || aux_display_3bits_14_node_28;
    aux_display_3bits_14_or_2 = false || aux_display_3bits_14_node_31;
    aux_display_3bits_14_or_37 = false || aux_display_3bits_14_node_30;
    aux_display_3bits_14_not_5 = !aux_display_3bits_14_or_37;
    aux_display_3bits_14_not_23 = !aux_display_3bits_14_or_1;
    aux_display_3bits_14_not_36 = !aux_display_3bits_14_or_2;
    aux_display_3bits_14_and_3 = aux_display_3bits_14_or_2 && aux_display_3bits_14_not_23;
    aux_display_3bits_14_and_6 = aux_display_3bits_14_or_37 && aux_display_3bits_14_not_36;
    aux_display_3bits_14_and_7 = aux_display_3bits_14_or_2 && aux_display_3bits_14_not_23;
    aux_display_3bits_14_and_8 = aux_display_3bits_14_not_5 && aux_display_3bits_14_not_36 && aux_display_3bits_14_not_23;
    aux_display_3bits_14_xnor_9 = !(aux_display_3bits_14_not_36 ^ aux_display_3bits_14_not_23);
    aux_display_3bits_14_and_10 = aux_display_3bits_14_or_37 && aux_display_3bits_14_not_23;
    aux_display_3bits_14_and_24 = aux_display_3bits_14_or_37 && aux_display_3bits_14_not_36 && aux_display_3bits_14_or_1;
    aux_display_3bits_14_and_25 = aux_display_3bits_14_node_31 && aux_display_3bits_14_not_23;
    aux_display_3bits_14_and_26 = aux_display_3bits_14_not_36 && aux_display_3bits_14_not_23;
    aux_display_3bits_14_and_27 = aux_display_3bits_14_not_5 && aux_display_3bits_14_not_23;
    aux_display_3bits_14_xnor_32 = !(aux_display_3bits_14_not_5 ^ aux_display_3bits_14_not_23);
    aux_display_3bits_14_and_35 = aux_display_3bits_14_not_5 && aux_display_3bits_14_or_2;
    aux_display_3bits_14_node_0 = false;
    aux_display_3bits_14_or_4 = aux_display_3bits_14_and_7 || aux_display_3bits_14_and_8;
    aux_display_3bits_14_or_11 = aux_display_3bits_14_and_26 || aux_display_3bits_14_and_6 || aux_display_3bits_14_and_10;
    aux_display_3bits_14_nand_12 = !(aux_display_3bits_14_not_5 && aux_display_3bits_14_or_2 && aux_display_3bits_14_not_23);
    aux_display_3bits_14_or_13 = aux_display_3bits_14_and_6 || aux_display_3bits_14_and_35 || aux_display_3bits_14_and_25;
    aux_display_3bits_14_or_14 = aux_display_3bits_14_not_5 || aux_display_3bits_14_xnor_9;
    aux_display_3bits_14_or_29 = aux_display_3bits_14_and_27 || aux_display_3bits_14_and_35 || aux_display_3bits_14_and_3 || aux_display_3bits_14_and_24;
    aux_display_3bits_14_or_34 = aux_display_3bits_14_xnor_32 || aux_display_3bits_14_or_2;
    aux_display_3bits_14_node_15 = aux_display_3bits_14_or_13;
    aux_display_3bits_14_node_16 = aux_display_3bits_14_or_11;
    aux_display_3bits_14_node_17 = aux_display_3bits_14_or_4;
    aux_display_3bits_14_node_18 = aux_display_3bits_14_or_29;
    aux_display_3bits_14_node_19 = aux_display_3bits_14_or_34;
    aux_display_3bits_14_node_20 = aux_display_3bits_14_or_14;
    aux_display_3bits_14_node_21 = aux_display_3bits_14_node_0;
    aux_display_3bits_14_node_22 = aux_display_3bits_14_nand_12;
    aux_display_3bits_14_display7_g_middle = aux_display_3bits_14_node_15;
    aux_display_3bits_14_display7_f_upper_left_1 = aux_display_3bits_14_node_16;
    aux_display_3bits_14_display7_e_lower_left_2 = aux_display_3bits_14_node_17;
    aux_display_3bits_14_display7_d_bottom_3 = aux_display_3bits_14_node_18;
    aux_display_3bits_14_display7_a_top_4 = aux_display_3bits_14_node_19;
    aux_display_3bits_14_display7_b_upper_right_5 = aux_display_3bits_14_node_20;
    aux_display_3bits_14_display7_dp_dot_6 = aux_display_3bits_14_node_21;
    aux_display_3bits_14_display7_c_lower_right_7 = aux_display_3bits_14_node_22;
    // End IC: DISPLAY-3BITS

    // Write output data
    digitalWrite(seven_segment_display16_g_middle, aux_display_3bits_14_display7_g_middle);
    digitalWrite(seven_segment_display16_f_upper_left, aux_display_3bits_14_display7_f_upper_left_1);
    digitalWrite(seven_segment_display16_e_lower_left, aux_display_3bits_14_display7_e_lower_left_2);
    digitalWrite(seven_segment_display16_d_bottom, aux_display_3bits_14_display7_d_bottom_3);
    digitalWrite(seven_segment_display16_a_top, aux_display_3bits_14_display7_a_top_4);
    digitalWrite(seven_segment_display16_b_upper_right, aux_display_3bits_14_display7_b_upper_right_5);
    digitalWrite(seven_segment_display16_dp_dot, aux_node_13);
    digitalWrite(seven_segment_display16_c_lower_right, aux_display_3bits_14_display7_c_lower_right_7);
}
