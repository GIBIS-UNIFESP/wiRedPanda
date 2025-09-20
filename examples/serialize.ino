// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


/* ========= Inputs ========== */
const int input_switch1_load__shift = A0;
const int input_switch2_clock = A1;
const int input_switch3_d0 = A2;
const int input_switch4_d1 = A3;
const int input_switch5_d2 = A4;
const int input_switch6_d3 = A5;

/* ========= Outputs ========== */
const int led37_0 = 2;
const int led38_0 = 3;

/* ====== Aux. Variables ====== */
boolean input_switch1_load__shift_val = LOW;
boolean input_switch2_clock_val = LOW;
boolean input_switch3_d0_val = LOW;
boolean input_switch4_d1_val = LOW;
boolean input_switch5_d2_val = LOW;
boolean input_switch6_d3_val = LOW;
boolean aux_input_switch_0 = LOW;
boolean aux_node_1 = LOW;
boolean aux_input_switch_2 = LOW;
boolean aux_not_3 = LOW;
boolean aux_input_switch_4 = LOW;
boolean aux_node_5 = LOW;
boolean aux_node_6 = LOW;
boolean aux_node_7 = LOW;
boolean aux_d_flip_flop_8_0_q = LOW;
boolean aux_d_flip_flop_8_0_q_inclk = LOW;
boolean aux_d_flip_flop_8_0_q_last = LOW;
boolean aux_d_flip_flop_8_1_q = HIGH;
boolean aux_d_flip_flop_8_1_q_inclk = LOW;
boolean aux_d_flip_flop_8_1_q_last = LOW;
boolean aux_node_9 = LOW;
boolean aux_input_switch_10 = LOW;
boolean aux_and_11 = LOW;
boolean aux_and_12 = LOW;
boolean aux_node_13 = LOW;
boolean aux_or_14 = LOW;
boolean aux_node_15 = LOW;
boolean aux_node_16 = LOW;
boolean aux_input_switch_17 = LOW;
boolean aux_d_flip_flop_18_0_q = LOW;
boolean aux_d_flip_flop_18_0_q_inclk = LOW;
boolean aux_d_flip_flop_18_0_q_last = LOW;
boolean aux_d_flip_flop_18_1_q = HIGH;
boolean aux_d_flip_flop_18_1_q_inclk = LOW;
boolean aux_d_flip_flop_18_1_q_last = LOW;
boolean aux_and_19 = LOW;
boolean aux_and_20 = LOW;
boolean aux_node_21 = LOW;
boolean aux_or_22 = LOW;
boolean aux_d_flip_flop_23_0_q = LOW;
boolean aux_d_flip_flop_23_0_q_inclk = LOW;
boolean aux_d_flip_flop_23_0_q_last = LOW;
boolean aux_d_flip_flop_23_1_q = HIGH;
boolean aux_d_flip_flop_23_1_q_inclk = LOW;
boolean aux_d_flip_flop_23_1_q_last = LOW;
boolean aux_node_24 = LOW;
boolean aux_input_switch_25 = LOW;
boolean aux_node_26 = LOW;
boolean aux_and_27 = LOW;
boolean aux_and_28 = LOW;
boolean aux_or_29 = LOW;
boolean aux_node_30 = LOW;
boolean aux_d_flip_flop_31_0_q = LOW;
boolean aux_d_flip_flop_31_0_q_inclk = LOW;
boolean aux_d_flip_flop_31_0_q_last = LOW;
boolean aux_d_flip_flop_31_1_q = HIGH;
boolean aux_d_flip_flop_31_1_q_inclk = LOW;
boolean aux_d_flip_flop_31_1_q_last = LOW;
boolean aux_node_32 = LOW;
boolean aux_node_33 = LOW;
boolean aux_node_34 = LOW;
boolean aux_and_35 = LOW;

void setup() {
    pinMode(input_switch1_load__shift, INPUT);
    pinMode(input_switch2_clock, INPUT);
    pinMode(input_switch3_d0, INPUT);
    pinMode(input_switch4_d1, INPUT);
    pinMode(input_switch5_d2, INPUT);
    pinMode(input_switch6_d3, INPUT);
    pinMode(led37_0, OUTPUT);
    pinMode(led38_0, OUTPUT);
}

void loop() {
    // Reading input data //.
    input_switch1_load__shift_val = digitalRead(input_switch1_load__shift);
    input_switch2_clock_val = digitalRead(input_switch2_clock);
    input_switch3_d0_val = digitalRead(input_switch3_d0);
    input_switch4_d1_val = digitalRead(input_switch4_d1);
    input_switch5_d2_val = digitalRead(input_switch5_d2);
    input_switch6_d3_val = digitalRead(input_switch6_d3);

    // Updating clocks. //

    // Assigning aux variables. //
    aux_input_switch_0 = input_switch1_load__shift_val;
    aux_node_1 = aux_input_switch_0;
    aux_input_switch_2 = input_switch2_clock_val;
    aux_not_3 = !aux_node_1;
    aux_input_switch_4 = input_switch3_d0_val;
    aux_node_5 = aux_input_switch_2;
    aux_node_6 = aux_input_switch_0;
    aux_node_7 = aux_node_6;
    //D FlipFlop
    if (aux_node_5 && !aux_d_flip_flop_8_0_q_inclk) { 
        aux_d_flip_flop_8_0_q = aux_d_flip_flop_8_0_q_last;
        aux_d_flip_flop_8_1_q = !aux_d_flip_flop_8_0_q_last;
    }
    if (!HIGH) { 
        aux_d_flip_flop_8_0_q = HIGH; //Preset
        aux_d_flip_flop_8_1_q = LOW;
    } else if (!HIGH) { 
        aux_d_flip_flop_8_0_q = LOW; //Clear
        aux_d_flip_flop_8_1_q = HIGH;
    }
    aux_d_flip_flop_8_0_q_inclk = aux_node_5;
    aux_d_flip_flop_8_0_q_last = aux_input_switch_4;
    //End of D FlipFlop
    aux_node_9 = aux_not_3;
    aux_input_switch_10 = input_switch4_d1_val;
    aux_and_11 = aux_node_7 && aux_input_switch_10;
    aux_and_12 = aux_node_9 && aux_d_flip_flop_8_0_q;
    aux_node_13 = aux_input_switch_2;
    aux_or_14 = aux_and_11 || aux_and_12;
    aux_node_15 = aux_node_6;
    aux_node_16 = aux_not_3;
    aux_input_switch_17 = input_switch5_d2_val;
    //D FlipFlop
    if (aux_node_13 && !aux_d_flip_flop_18_0_q_inclk) { 
        aux_d_flip_flop_18_0_q = aux_d_flip_flop_18_0_q_last;
        aux_d_flip_flop_18_1_q = !aux_d_flip_flop_18_0_q_last;
    }
    if (!HIGH) { 
        aux_d_flip_flop_18_0_q = HIGH; //Preset
        aux_d_flip_flop_18_1_q = LOW;
    } else if (!HIGH) { 
        aux_d_flip_flop_18_0_q = LOW; //Clear
        aux_d_flip_flop_18_1_q = HIGH;
    }
    aux_d_flip_flop_18_0_q_inclk = aux_node_13;
    aux_d_flip_flop_18_0_q_last = aux_or_14;
    //End of D FlipFlop
    aux_and_19 = aux_node_16 && aux_d_flip_flop_18_0_q;
    aux_and_20 = aux_node_15 && aux_input_switch_17;
    aux_node_21 = aux_input_switch_2;
    aux_or_22 = aux_and_20 || aux_and_19;
    //D FlipFlop
    if (aux_node_21 && !aux_d_flip_flop_23_0_q_inclk) { 
        aux_d_flip_flop_23_0_q = aux_d_flip_flop_23_0_q_last;
        aux_d_flip_flop_23_1_q = !aux_d_flip_flop_23_0_q_last;
    }
    if (!HIGH) { 
        aux_d_flip_flop_23_0_q = HIGH; //Preset
        aux_d_flip_flop_23_1_q = LOW;
    } else if (!HIGH) { 
        aux_d_flip_flop_23_0_q = LOW; //Clear
        aux_d_flip_flop_23_1_q = HIGH;
    }
    aux_d_flip_flop_23_0_q_inclk = aux_node_21;
    aux_d_flip_flop_23_0_q_last = aux_or_22;
    //End of D FlipFlop
    aux_node_24 = aux_not_3;
    aux_input_switch_25 = input_switch6_d3_val;
    aux_node_26 = aux_node_6;
    aux_and_27 = aux_node_26 && aux_input_switch_25;
    aux_and_28 = aux_node_24 && aux_d_flip_flop_23_0_q;
    aux_or_29 = aux_and_27 || aux_and_28;
    aux_node_30 = aux_input_switch_2;
    //D FlipFlop
    if (aux_node_30 && !aux_d_flip_flop_31_0_q_inclk) { 
        aux_d_flip_flop_31_0_q = aux_d_flip_flop_31_0_q_last;
        aux_d_flip_flop_31_1_q = !aux_d_flip_flop_31_0_q_last;
    }
    if (!HIGH) { 
        aux_d_flip_flop_31_0_q = HIGH; //Preset
        aux_d_flip_flop_31_1_q = LOW;
    } else if (!HIGH) { 
        aux_d_flip_flop_31_0_q = LOW; //Clear
        aux_d_flip_flop_31_1_q = HIGH;
    }
    aux_d_flip_flop_31_0_q_inclk = aux_node_30;
    aux_d_flip_flop_31_0_q_last = aux_or_29;
    //End of D FlipFlop
    aux_node_32 = aux_node_1;
    aux_node_33 = aux_not_3;
    aux_node_34 = aux_node_32;
    aux_and_35 = aux_node_33 && aux_d_flip_flop_31_0_q;

    // Writing output data. //
    digitalWrite(led37_0, aux_and_35);
    digitalWrite(led38_0, aux_node_34);
}
