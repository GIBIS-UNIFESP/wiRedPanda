// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


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
boolean push_button1_btn_val = LOW;
boolean aux_clock_0 = LOW;
unsigned long aux_clock_0_lastTime = 0;
const unsigned long aux_clock_0_interval = 200;
boolean aux_node_1 = LOW;
boolean aux_push_button_2 = LOW;
boolean aux_t_flip_flop_3_0_q = LOW;
boolean aux_t_flip_flop_3_0_q_inclk = LOW;
boolean aux_t_flip_flop_3_1_q = HIGH;
boolean aux_t_flip_flop_3_1_q_inclk = LOW;
boolean aux_node_4 = LOW;
boolean aux_t_flip_flop_5_0_q = LOW;
boolean aux_t_flip_flop_5_0_q_inclk = LOW;
boolean aux_t_flip_flop_5_1_q = HIGH;
boolean aux_t_flip_flop_5_1_q_inclk = LOW;
boolean aux_node_6 = LOW;
boolean aux_t_flip_flop_7_0_q = LOW;
boolean aux_t_flip_flop_7_0_q_inclk = LOW;
boolean aux_t_flip_flop_7_1_q = HIGH;
boolean aux_t_flip_flop_7_1_q_inclk = LOW;
boolean aux_node_8 = LOW;
boolean aux_node_9 = LOW;
boolean aux_node_10 = LOW;
boolean aux_node_11 = LOW;
boolean aux_node_12 = LOW;
boolean aux_node_13 = LOW;
// IC: DISPLAY-3BITS
boolean aux_ic_14_display7_g_middle = LOW;
boolean aux_ic_14_display7_f_upper_left_1 = LOW;
boolean aux_ic_14_display7_e_lower_left_2 = LOW;
boolean aux_ic_14_display7_d_bottom_3 = LOW;
boolean aux_ic_14_display7_a_top_4 = LOW;
boolean aux_ic_14_display7_b_upper_right_5 = LOW;
boolean aux_ic_14_display7_dp_dot_6 = LOW;
boolean aux_ic_14_display7_c_lower_right_7 = LOW;
boolean aux_ic_14_node_0 = LOW;
boolean aux_ic_14_or_1 = LOW;
boolean aux_ic_14_or_2 = LOW;
boolean aux_ic_14_and_3 = LOW;
boolean aux_ic_14_or_4 = LOW;
boolean aux_ic_14_not_5 = LOW;
boolean aux_ic_14_and_6 = LOW;
boolean aux_ic_14_and_7 = LOW;
boolean aux_ic_14_and_8 = LOW;
boolean aux_ic_14_xnor_9 = LOW;
boolean aux_ic_14_and_10 = LOW;
boolean aux_ic_14_or_11 = LOW;
boolean aux_ic_14_nand_12 = LOW;
boolean aux_ic_14_or_13 = LOW;
boolean aux_ic_14_or_14 = LOW;
boolean aux_ic_14_node_15 = LOW;
boolean aux_ic_14_node_16 = LOW;
boolean aux_ic_14_node_17 = LOW;
boolean aux_ic_14_node_18 = LOW;
boolean aux_ic_14_node_19 = LOW;
boolean aux_ic_14_node_20 = LOW;
boolean aux_ic_14_node_21 = LOW;
boolean aux_ic_14_node_22 = LOW;
boolean aux_ic_14_not_23 = LOW;
boolean aux_ic_14_and_24 = LOW;
boolean aux_ic_14_and_25 = LOW;
boolean aux_ic_14_and_26 = LOW;
boolean aux_ic_14_and_27 = LOW;
boolean aux_ic_14_node_28 = LOW;
boolean aux_ic_14_or_29 = LOW;
boolean aux_ic_14_node_30 = LOW;
boolean aux_ic_14_node_31 = LOW;
boolean aux_ic_14_xnor_32 = LOW;
boolean aux_ic_14_or_34 = LOW;
boolean aux_ic_14_and_35 = LOW;
boolean aux_ic_14_not_36 = LOW;
boolean aux_ic_14_or_37 = LOW;
boolean aux_ic_input_ic_0 = LOW;
boolean aux_ic_input_ic_1 = LOW;
boolean aux_ic_input_ic_2 = LOW;
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
    // Reading input data //.
    push_button1_btn_val = digitalRead(push_button1_btn);

    // Updating clocks. //
    unsigned long now = millis();
    if (now - aux_clock_0_lastTime >= aux_clock_0_interval) {
        aux_clock_0_lastTime = now;
        aux_clock_0 = !aux_clock_0;
    }

    // Assigning aux variables. //
    aux_node_1 = aux_clock_0;
    aux_push_button_2 = push_button1_btn_val;
    //T FlipFlop
    if (aux_node_1 && !aux_t_flip_flop_3_0_q_inclk) { 
        if (aux_push_button_2) { 
            aux_t_flip_flop_3_0_q = !aux_t_flip_flop_3_0_q;
            aux_t_flip_flop_3_1_q = !aux_t_flip_flop_3_0_q;
        }
    }
    if (!HIGH) { 
        aux_t_flip_flop_3_0_q = HIGH; //Preset
        aux_t_flip_flop_3_1_q = LOW;
    } else if (!HIGH) { 
        aux_t_flip_flop_3_0_q = LOW; //Clear
        aux_t_flip_flop_3_1_q = HIGH;
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
    if (!HIGH) { 
        aux_t_flip_flop_5_0_q = HIGH; //Preset
        aux_t_flip_flop_5_1_q = LOW;
    } else if (!HIGH) { 
        aux_t_flip_flop_5_0_q = LOW; //Clear
        aux_t_flip_flop_5_1_q = HIGH;
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
    if (!HIGH) { 
        aux_t_flip_flop_7_0_q = HIGH; //Preset
        aux_t_flip_flop_7_1_q = LOW;
    } else if (!HIGH) { 
        aux_t_flip_flop_7_0_q = LOW; //Clear
        aux_t_flip_flop_7_1_q = HIGH;
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
    aux_ic_input_ic_0 = aux_node_9;
    aux_ic_input_ic_1 = aux_node_12;
    aux_ic_input_ic_2 = aux_node_11;
    aux_ic_14_node_31 = LOW;
    aux_ic_14_node_30 = LOW;
    aux_ic_14_node_28 = LOW;
    aux_ic_14_or_1 = LOW || aux_ic_14_node_28;
    aux_ic_14_or_37 = LOW || aux_ic_14_node_30;
    aux_ic_14_or_2 = LOW || aux_ic_14_node_31;
    aux_ic_14_not_23 = !aux_ic_14_or_1;
    aux_ic_14_not_36 = !aux_ic_14_or_2;
    aux_ic_14_not_5 = !aux_ic_14_or_37;
    aux_ic_14_and_26 = aux_ic_14_not_36 && aux_ic_14_not_23;
    aux_ic_14_and_25 = aux_ic_14_node_31 && aux_ic_14_not_23;
    aux_ic_14_and_24 = aux_ic_14_or_37 && aux_ic_14_not_36 && aux_ic_14_or_1;
    aux_ic_14_and_27 = aux_ic_14_not_5 && aux_ic_14_not_23;
    aux_ic_14_and_3 = aux_ic_14_or_2 && aux_ic_14_not_23;
    aux_ic_14_xnor_32 = !(aux_ic_14_not_5 ^ aux_ic_14_not_23);
    aux_ic_14_and_35 = aux_ic_14_not_5 && aux_ic_14_or_2;
    aux_ic_14_and_6 = aux_ic_14_or_37 && aux_ic_14_not_36;
    aux_ic_14_and_10 = aux_ic_14_or_37 && aux_ic_14_not_23;
    aux_ic_14_xnor_9 = !(aux_ic_14_not_36 ^ aux_ic_14_not_23);
    aux_ic_14_and_8 = aux_ic_14_not_5 && aux_ic_14_not_36 && aux_ic_14_not_23;
    aux_ic_14_and_7 = aux_ic_14_or_2 && aux_ic_14_not_23;
    aux_ic_14_node_0 = LOW;
    aux_ic_14_or_34 = aux_ic_14_xnor_32 || aux_ic_14_or_2;
    aux_ic_14_or_4 = aux_ic_14_and_7 || aux_ic_14_and_8;
    aux_ic_14_or_29 = aux_ic_14_and_27 || aux_ic_14_and_35 || aux_ic_14_and_3 || aux_ic_14_and_24;
    aux_ic_14_or_14 = aux_ic_14_not_5 || aux_ic_14_xnor_9;
    aux_ic_14_or_13 = aux_ic_14_and_6 || aux_ic_14_and_35 || aux_ic_14_and_25;
    aux_ic_14_nand_12 = !(aux_ic_14_not_5 && aux_ic_14_or_2 && aux_ic_14_not_23);
    aux_ic_14_or_11 = aux_ic_14_and_26 || aux_ic_14_and_6 || aux_ic_14_and_10;
    aux_ic_14_node_22 = aux_ic_14_nand_12;
    aux_ic_14_node_21 = aux_ic_14_node_0;
    aux_ic_14_node_20 = aux_ic_14_or_14;
    aux_ic_14_node_19 = aux_ic_14_or_34;
    aux_ic_14_node_18 = aux_ic_14_or_29;
    aux_ic_14_node_17 = aux_ic_14_or_4;
    aux_ic_14_node_16 = aux_ic_14_or_11;
    aux_ic_14_node_15 = aux_ic_14_or_13;
    aux_ic_14_display7_g_middle = aux_ic_14_node_15;
    aux_ic_14_display7_f_upper_left_1 = aux_ic_14_node_16;
    aux_ic_14_display7_e_lower_left_2 = aux_ic_14_node_17;
    aux_ic_14_display7_d_bottom_3 = aux_ic_14_node_18;
    aux_ic_14_display7_a_top_4 = aux_ic_14_node_19;
    aux_ic_14_display7_b_upper_right_5 = aux_ic_14_node_20;
    aux_ic_14_display7_dp_dot_6 = aux_ic_14_node_21;
    aux_ic_14_display7_c_lower_right_7 = aux_ic_14_node_22;
    // End IC: DISPLAY-3BITS

    // Writing output data. //
    digitalWrite(seven_segment_display16_g_middle, aux_ic_14_display7_g_middle);
    digitalWrite(seven_segment_display16_f_upper_left, aux_ic_14_display7_f_upper_left_1);
    digitalWrite(seven_segment_display16_e_lower_left, aux_ic_14_display7_e_lower_left_2);
    digitalWrite(seven_segment_display16_d_bottom, aux_ic_14_display7_d_bottom_3);
    digitalWrite(seven_segment_display16_a_top, aux_ic_14_display7_a_top_4);
    digitalWrite(seven_segment_display16_b_upper_right, aux_ic_14_display7_b_upper_right_5);
    digitalWrite(seven_segment_display16_dp_dot, aux_node_13);
    digitalWrite(seven_segment_display16_c_lower_right, aux_ic_14_display7_c_lower_right_7);
}
