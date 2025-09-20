// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


/* ========= Inputs ========== */
const int push_button1_reset = A0;

/* ========= Outputs ========== */
const int led34_load_shift_0 = A1;
const int led35_l3_0 = A2;
const int led36_l2_0 = A3;
const int led37_l0_0 = A4;
const int led38_l1_0 = A5;

/* ====== Aux. Variables ====== */
boolean push_button1_reset_val = LOW;
boolean aux_push_button_0 = LOW;
boolean aux_not_1 = LOW;
boolean aux_node_2 = LOW;
boolean aux_node_3 = LOW;
boolean aux_clock_4 = LOW;
unsigned long aux_clock_4_lastTime = 0;
const unsigned long aux_clock_4_interval = 500;
boolean aux_jk_flip_flop_5_0_q = LOW;
boolean aux_jk_flip_flop_5_0_q_inclk = LOW;
boolean aux_jk_flip_flop_5_1_q = HIGH;
boolean aux_jk_flip_flop_5_1_q_inclk = LOW;
boolean aux_node_6 = LOW;
boolean aux_jk_flip_flop_7_0_q = LOW;
boolean aux_jk_flip_flop_7_0_q_inclk = LOW;
boolean aux_jk_flip_flop_7_1_q = HIGH;
boolean aux_jk_flip_flop_7_1_q_inclk = LOW;
boolean aux_node_8 = LOW;
boolean aux_jk_flip_flop_9_0_q = LOW;
boolean aux_jk_flip_flop_9_0_q_inclk = LOW;
boolean aux_jk_flip_flop_9_1_q = HIGH;
boolean aux_jk_flip_flop_9_1_q_inclk = LOW;
boolean aux_node_10 = LOW;
boolean aux_jk_flip_flop_11_0_q = LOW;
boolean aux_jk_flip_flop_11_0_q_inclk = LOW;
boolean aux_jk_flip_flop_11_1_q = HIGH;
boolean aux_jk_flip_flop_11_1_q_inclk = LOW;
boolean aux_node_12 = LOW;
boolean aux_node_13 = LOW;
boolean aux_node_14 = LOW;
boolean aux_clock_15 = LOW;
unsigned long aux_clock_15_lastTime = 0;
const unsigned long aux_clock_15_interval = 62.5;
boolean aux_node_16 = LOW;
boolean aux_node_17 = LOW;
boolean aux_node_18 = LOW;
boolean aux_node_19 = LOW;
boolean aux_node_20 = LOW;
// IC: SERIALIZE
boolean aux_ic_21_led_0 = LOW;
boolean aux_ic_21_led_0_1 = LOW;
boolean aux_ic_21_node_0 = LOW;
boolean aux_ic_21_node_1 = LOW;
boolean aux_ic_21_node_2 = LOW;
boolean aux_ic_21_node_3 = LOW;
boolean aux_ic_21_node_4 = LOW;
boolean aux_ic_21_node_5 = LOW;
boolean aux_ic_21_node_6 = LOW;
boolean aux_ic_21_node_7 = LOW;
boolean aux_ic_21_node_8 = LOW;
boolean aux_ic_21_node_9 = LOW;
boolean aux_ic_21_node_10 = LOW;
boolean aux_ic_21_node_11 = LOW;
boolean aux_ic_21_node_12 = LOW;
boolean aux_ic_21_node_13 = LOW;
boolean aux_ic_21_node_14 = LOW;
boolean aux_ic_21_node_15 = LOW;
boolean aux_ic_21_node_16 = LOW;
boolean aux_ic_21_and_17 = LOW;
boolean aux_ic_21_node_18 = LOW;
boolean aux_ic_21_not_19 = LOW;
boolean aux_ic_21_and_20 = LOW;
boolean aux_ic_21_or_21 = LOW;
boolean aux_ic_21_and_22 = LOW;
boolean aux_ic_21_d_flip_flop_23_0_q = LOW;
boolean aux_ic_21_d_flip_flop_23_0_q_inclk = LOW;
boolean aux_ic_21_d_flip_flop_23_0_q_last = LOW;
boolean aux_ic_21_d_flip_flop_23_1_q = HIGH;
boolean aux_ic_21_d_flip_flop_23_1_q_inclk = LOW;
boolean aux_ic_21_d_flip_flop_23_1_q_last = LOW;
boolean aux_ic_21_and_24 = LOW;
boolean aux_ic_21_or_25 = LOW;
boolean aux_ic_21_d_flip_flop_26_0_q = LOW;
boolean aux_ic_21_d_flip_flop_26_0_q_inclk = LOW;
boolean aux_ic_21_d_flip_flop_26_0_q_last = LOW;
boolean aux_ic_21_d_flip_flop_26_1_q = HIGH;
boolean aux_ic_21_d_flip_flop_26_1_q_inclk = LOW;
boolean aux_ic_21_d_flip_flop_26_1_q_last = LOW;
boolean aux_ic_21_and_27 = LOW;
boolean aux_ic_21_d_flip_flop_28_0_q = LOW;
boolean aux_ic_21_d_flip_flop_28_0_q_inclk = LOW;
boolean aux_ic_21_d_flip_flop_28_0_q_last = LOW;
boolean aux_ic_21_d_flip_flop_28_1_q = HIGH;
boolean aux_ic_21_d_flip_flop_28_1_q_inclk = LOW;
boolean aux_ic_21_d_flip_flop_28_1_q_last = LOW;
boolean aux_ic_21_node_29 = LOW;
boolean aux_ic_21_node_30 = LOW;
boolean aux_ic_21_node_31 = LOW;
boolean aux_ic_21_node_32 = LOW;
boolean aux_ic_21_and_33 = LOW;
boolean aux_ic_21_d_flip_flop_34_0_q = LOW;
boolean aux_ic_21_d_flip_flop_34_0_q_inclk = LOW;
boolean aux_ic_21_d_flip_flop_34_0_q_last = LOW;
boolean aux_ic_21_d_flip_flop_34_1_q = HIGH;
boolean aux_ic_21_d_flip_flop_34_1_q_inclk = LOW;
boolean aux_ic_21_d_flip_flop_34_1_q_last = LOW;
boolean aux_ic_21_and_35 = LOW;
boolean aux_ic_21_or_36 = LOW;
boolean aux_ic_21_node_37 = LOW;
boolean aux_ic_input_ic_0 = LOW;
boolean aux_ic_input_ic_1 = LOW;
boolean aux_ic_input_ic_2 = LOW;
boolean aux_ic_input_ic_3 = LOW;
boolean aux_ic_input_ic_4 = LOW;
boolean aux_ic_input_ic_5 = LOW;
// End IC: SERIALIZE
boolean aux_node_22 = LOW;
boolean aux_node_23 = LOW;
boolean aux_not_24 = LOW;
boolean aux_node_25 = LOW;
boolean aux_and_26 = LOW;
// IC: REGISTER
boolean aux_ic_27_led_0 = LOW;
boolean aux_ic_27_led_0_1 = LOW;
boolean aux_ic_27_led_0_2 = LOW;
boolean aux_ic_27_led_0_3 = LOW;
boolean aux_ic_27_d_flip_flop_0_0_q = LOW;
boolean aux_ic_27_d_flip_flop_0_0_q_inclk = LOW;
boolean aux_ic_27_d_flip_flop_0_0_q_last = LOW;
boolean aux_ic_27_d_flip_flop_0_1_q = HIGH;
boolean aux_ic_27_d_flip_flop_0_1_q_inclk = LOW;
boolean aux_ic_27_d_flip_flop_0_1_q_last = LOW;
boolean aux_ic_27_node_1 = LOW;
boolean aux_ic_27_node_2 = LOW;
boolean aux_ic_27_d_flip_flop_3_0_q = LOW;
boolean aux_ic_27_d_flip_flop_3_0_q_inclk = LOW;
boolean aux_ic_27_d_flip_flop_3_0_q_last = LOW;
boolean aux_ic_27_d_flip_flop_3_1_q = HIGH;
boolean aux_ic_27_d_flip_flop_3_1_q_inclk = LOW;
boolean aux_ic_27_d_flip_flop_3_1_q_last = LOW;
boolean aux_ic_27_node_4 = LOW;
boolean aux_ic_27_d_flip_flop_5_0_q = LOW;
boolean aux_ic_27_d_flip_flop_5_0_q_inclk = LOW;
boolean aux_ic_27_d_flip_flop_5_0_q_last = LOW;
boolean aux_ic_27_d_flip_flop_5_1_q = HIGH;
boolean aux_ic_27_d_flip_flop_5_1_q_inclk = LOW;
boolean aux_ic_27_d_flip_flop_5_1_q_last = LOW;
boolean aux_ic_27_node_6 = LOW;
boolean aux_ic_27_node_7 = LOW;
boolean aux_ic_27_node_8 = LOW;
boolean aux_ic_27_node_9 = LOW;
boolean aux_ic_27_node_10 = LOW;
boolean aux_ic_27_node_11 = LOW;
boolean aux_ic_27_node_12 = LOW;
boolean aux_ic_27_d_flip_flop_13_0_q = LOW;
boolean aux_ic_27_d_flip_flop_13_0_q_inclk = LOW;
boolean aux_ic_27_d_flip_flop_13_0_q_last = LOW;
boolean aux_ic_27_d_flip_flop_13_1_q = HIGH;
boolean aux_ic_27_d_flip_flop_13_1_q_inclk = LOW;
boolean aux_ic_27_d_flip_flop_13_1_q_last = LOW;
// End IC: REGISTER
boolean aux_node_28 = LOW;
boolean aux_and_29 = LOW;
boolean aux_and_30 = LOW;
boolean aux_and_31 = LOW;
boolean aux_and_32 = LOW;

void setup() {
    pinMode(push_button1_reset, INPUT);
    pinMode(led34_load_shift_0, OUTPUT);
    pinMode(led35_l3_0, OUTPUT);
    pinMode(led36_l2_0, OUTPUT);
    pinMode(led37_l0_0, OUTPUT);
    pinMode(led38_l1_0, OUTPUT);
}

void loop() {
    // Reading input data //.
    push_button1_reset_val = digitalRead(push_button1_reset);

    // Updating clocks. //
    unsigned long now = millis();
    if (now - aux_clock_4_lastTime >= aux_clock_4_interval) {
        aux_clock_4_lastTime = now;
        aux_clock_4 = !aux_clock_4;
    }
    unsigned long now = millis();
    if (now - aux_clock_15_lastTime >= aux_clock_15_interval) {
        aux_clock_15_lastTime = now;
        aux_clock_15 = !aux_clock_15;
    }

    // Assigning aux variables. //
    aux_push_button_0 = push_button1_reset_val;
    aux_not_1 = !aux_push_button_0;
    aux_node_2 = aux_not_1;
    aux_node_3 = aux_node_2;
    //JK FlipFlop
    if (aux_clock_4 && !aux_jk_flip_flop_5_0_q_inclk) { 
        if (HIGH && HIGH) { 
            boolean aux = aux_jk_flip_flop_5_0_q;
            aux_jk_flip_flop_5_0_q = aux_jk_flip_flop_5_1_q;
            aux_jk_flip_flop_5_1_q = aux;
        } else if (HIGH) {
            aux_jk_flip_flop_5_0_q = HIGH;
            aux_jk_flip_flop_5_1_q = LOW;
        } else if (HIGH) {
            aux_jk_flip_flop_5_0_q = LOW;
            aux_jk_flip_flop_5_1_q = HIGH;
        }
    }
    if (!aux_node_3) { 
        aux_jk_flip_flop_5_0_q = HIGH; //Preset
        aux_jk_flip_flop_5_1_q = LOW;
    } else if (!HIGH) { 
        aux_jk_flip_flop_5_0_q = LOW; //Clear
        aux_jk_flip_flop_5_1_q = HIGH;
    }
    aux_jk_flip_flop_5_0_q_inclk = aux_clock_4;
    //End of JK FlipFlop
    aux_node_6 = aux_node_3;
    //JK FlipFlop
    if (aux_jk_flip_flop_5_0_q && !aux_jk_flip_flop_7_0_q_inclk) { 
        if (HIGH && HIGH) { 
            boolean aux = aux_jk_flip_flop_7_0_q;
            aux_jk_flip_flop_7_0_q = aux_jk_flip_flop_7_1_q;
            aux_jk_flip_flop_7_1_q = aux;
        } else if (HIGH) {
            aux_jk_flip_flop_7_0_q = HIGH;
            aux_jk_flip_flop_7_1_q = LOW;
        } else if (HIGH) {
            aux_jk_flip_flop_7_0_q = LOW;
            aux_jk_flip_flop_7_1_q = HIGH;
        }
    }
    if (!aux_node_6) { 
        aux_jk_flip_flop_7_0_q = HIGH; //Preset
        aux_jk_flip_flop_7_1_q = LOW;
    } else if (!HIGH) { 
        aux_jk_flip_flop_7_0_q = LOW; //Clear
        aux_jk_flip_flop_7_1_q = HIGH;
    }
    aux_jk_flip_flop_7_0_q_inclk = aux_jk_flip_flop_5_0_q;
    //End of JK FlipFlop
    aux_node_8 = aux_node_6;
    //JK FlipFlop
    if (aux_jk_flip_flop_7_0_q && !aux_jk_flip_flop_9_0_q_inclk) { 
        if (HIGH && HIGH) { 
            boolean aux = aux_jk_flip_flop_9_0_q;
            aux_jk_flip_flop_9_0_q = aux_jk_flip_flop_9_1_q;
            aux_jk_flip_flop_9_1_q = aux;
        } else if (HIGH) {
            aux_jk_flip_flop_9_0_q = HIGH;
            aux_jk_flip_flop_9_1_q = LOW;
        } else if (HIGH) {
            aux_jk_flip_flop_9_0_q = LOW;
            aux_jk_flip_flop_9_1_q = HIGH;
        }
    }
    if (!aux_node_8) { 
        aux_jk_flip_flop_9_0_q = HIGH; //Preset
        aux_jk_flip_flop_9_1_q = LOW;
    } else if (!HIGH) { 
        aux_jk_flip_flop_9_0_q = LOW; //Clear
        aux_jk_flip_flop_9_1_q = HIGH;
    }
    aux_jk_flip_flop_9_0_q_inclk = aux_jk_flip_flop_7_0_q;
    //End of JK FlipFlop
    aux_node_10 = aux_node_8;
    //JK FlipFlop
    if (aux_jk_flip_flop_9_0_q && !aux_jk_flip_flop_11_0_q_inclk) { 
        if (HIGH && HIGH) { 
            boolean aux = aux_jk_flip_flop_11_0_q;
            aux_jk_flip_flop_11_0_q = aux_jk_flip_flop_11_1_q;
            aux_jk_flip_flop_11_1_q = aux;
        } else if (HIGH) {
            aux_jk_flip_flop_11_0_q = HIGH;
            aux_jk_flip_flop_11_1_q = LOW;
        } else if (HIGH) {
            aux_jk_flip_flop_11_0_q = LOW;
            aux_jk_flip_flop_11_1_q = HIGH;
        }
    }
    if (!aux_node_10) { 
        aux_jk_flip_flop_11_0_q = HIGH; //Preset
        aux_jk_flip_flop_11_1_q = LOW;
    } else if (!HIGH) { 
        aux_jk_flip_flop_11_0_q = LOW; //Clear
        aux_jk_flip_flop_11_1_q = HIGH;
    }
    aux_jk_flip_flop_11_0_q_inclk = aux_jk_flip_flop_9_0_q;
    //End of JK FlipFlop
    aux_node_12 = aux_clock_4;
    aux_node_13 = aux_jk_flip_flop_9_1_q;
    aux_node_14 = aux_jk_flip_flop_11_1_q;
    aux_node_16 = aux_node_12;
    aux_node_17 = aux_jk_flip_flop_5_1_q;
    aux_node_18 = aux_jk_flip_flop_7_1_q;
    aux_node_19 = aux_clock_15;
    aux_node_20 = aux_node_16;
    // IC: SERIALIZE
    aux_ic_input_ic_0 = aux_node_17;
    aux_ic_input_ic_1 = aux_node_18;
    aux_ic_input_ic_2 = aux_node_13;
    aux_ic_input_ic_3 = aux_node_14;
    aux_ic_input_ic_4 = aux_node_12;
    aux_ic_input_ic_5 = aux_clock_15;
    aux_ic_21_node_15 = HIGH;
    aux_ic_21_node_37 = aux_ic_21_node_15;
    aux_ic_21_node_14 = LOW;
    aux_ic_21_not_19 = !aux_ic_21_node_37;
    aux_ic_21_node_29 = LOW;
    aux_ic_21_node_0 = aux_ic_21_node_14;
    aux_ic_21_node_4 = aux_ic_21_node_15;
    aux_ic_21_node_7 = aux_ic_21_node_4;
    //D FlipFlop
    if (aux_ic_21_node_0 && !aux_ic_21_d_flip_flop_34_0_q_inclk) { 
        aux_ic_21_d_flip_flop_34_0_q = aux_ic_21_d_flip_flop_34_0_q_last;
        aux_ic_21_d_flip_flop_34_1_q = !aux_ic_21_d_flip_flop_34_0_q_last;
    }
    if (!HIGH) { 
        aux_ic_21_d_flip_flop_34_0_q = HIGH; //Preset
        aux_ic_21_d_flip_flop_34_1_q = LOW;
    } else if (!HIGH) { 
        aux_ic_21_d_flip_flop_34_0_q = LOW; //Clear
        aux_ic_21_d_flip_flop_34_1_q = HIGH;
    }
    aux_ic_21_d_flip_flop_34_0_q_inclk = aux_ic_21_node_0;
    aux_ic_21_d_flip_flop_34_0_q_last = aux_ic_21_node_29;
    //End of D FlipFlop
    aux_ic_21_node_11 = aux_ic_21_not_19;
    aux_ic_21_node_30 = HIGH;
    aux_ic_21_and_35 = aux_ic_21_node_7 && aux_ic_21_node_30;
    aux_ic_21_and_33 = aux_ic_21_node_11 && aux_ic_21_d_flip_flop_34_0_q;
    aux_ic_21_node_1 = aux_ic_21_node_14;
    aux_ic_21_or_36 = aux_ic_21_and_35 || aux_ic_21_and_33;
    aux_ic_21_node_6 = aux_ic_21_node_4;
    aux_ic_21_node_10 = aux_ic_21_not_19;
    aux_ic_21_node_31 = LOW;
    //D FlipFlop
    if (aux_ic_21_node_1 && !aux_ic_21_d_flip_flop_28_0_q_inclk) { 
        aux_ic_21_d_flip_flop_28_0_q = aux_ic_21_d_flip_flop_28_0_q_last;
        aux_ic_21_d_flip_flop_28_1_q = !aux_ic_21_d_flip_flop_28_0_q_last;
    }
    if (!HIGH) { 
        aux_ic_21_d_flip_flop_28_0_q = HIGH; //Preset
        aux_ic_21_d_flip_flop_28_1_q = LOW;
    } else if (!HIGH) { 
        aux_ic_21_d_flip_flop_28_0_q = LOW; //Clear
        aux_ic_21_d_flip_flop_28_1_q = HIGH;
    }
    aux_ic_21_d_flip_flop_28_0_q_inclk = aux_ic_21_node_1;
    aux_ic_21_d_flip_flop_28_0_q_last = aux_ic_21_or_36;
    //End of D FlipFlop
    aux_ic_21_and_27 = aux_ic_21_node_10 && aux_ic_21_d_flip_flop_28_0_q;
    aux_ic_21_and_24 = aux_ic_21_node_6 && aux_ic_21_node_31;
    aux_ic_21_node_2 = aux_ic_21_node_14;
    aux_ic_21_or_25 = aux_ic_21_and_24 || aux_ic_21_and_27;
    //D FlipFlop
    if (aux_ic_21_node_2 && !aux_ic_21_d_flip_flop_26_0_q_inclk) { 
        aux_ic_21_d_flip_flop_26_0_q = aux_ic_21_d_flip_flop_26_0_q_last;
        aux_ic_21_d_flip_flop_26_1_q = !aux_ic_21_d_flip_flop_26_0_q_last;
    }
    if (!HIGH) { 
        aux_ic_21_d_flip_flop_26_0_q = HIGH; //Preset
        aux_ic_21_d_flip_flop_26_1_q = LOW;
    } else if (!HIGH) { 
        aux_ic_21_d_flip_flop_26_0_q = LOW; //Clear
        aux_ic_21_d_flip_flop_26_1_q = HIGH;
    }
    aux_ic_21_d_flip_flop_26_0_q_inclk = aux_ic_21_node_2;
    aux_ic_21_d_flip_flop_26_0_q_last = aux_ic_21_or_25;
    //End of D FlipFlop
    aux_ic_21_node_9 = aux_ic_21_not_19;
    aux_ic_21_node_32 = HIGH;
    aux_ic_21_node_5 = aux_ic_21_node_4;
    aux_ic_21_and_22 = aux_ic_21_node_5 && aux_ic_21_node_32;
    aux_ic_21_and_20 = aux_ic_21_node_9 && aux_ic_21_d_flip_flop_26_0_q;
    aux_ic_21_or_21 = aux_ic_21_and_22 || aux_ic_21_and_20;
    aux_ic_21_node_3 = aux_ic_21_node_14;
    //D FlipFlop
    if (aux_ic_21_node_3 && !aux_ic_21_d_flip_flop_23_0_q_inclk) { 
        aux_ic_21_d_flip_flop_23_0_q = aux_ic_21_d_flip_flop_23_0_q_last;
        aux_ic_21_d_flip_flop_23_1_q = !aux_ic_21_d_flip_flop_23_0_q_last;
    }
    if (!HIGH) { 
        aux_ic_21_d_flip_flop_23_0_q = HIGH; //Preset
        aux_ic_21_d_flip_flop_23_1_q = LOW;
    } else if (!HIGH) { 
        aux_ic_21_d_flip_flop_23_0_q = LOW; //Clear
        aux_ic_21_d_flip_flop_23_1_q = HIGH;
    }
    aux_ic_21_d_flip_flop_23_0_q_inclk = aux_ic_21_node_3;
    aux_ic_21_d_flip_flop_23_0_q_last = aux_ic_21_or_21;
    //End of D FlipFlop
    aux_ic_21_node_13 = aux_ic_21_node_37;
    aux_ic_21_node_8 = aux_ic_21_not_19;
    aux_ic_21_node_12 = aux_ic_21_node_13;
    aux_ic_21_and_17 = aux_ic_21_node_8 && aux_ic_21_d_flip_flop_23_0_q;
    aux_ic_21_node_18 = aux_ic_21_and_17;
    aux_ic_21_node_16 = aux_ic_21_node_12;
    aux_ic_21_led_0 = aux_ic_21_node_18;
    aux_ic_21_led_0_1 = aux_ic_21_node_16;
    // End IC: SERIALIZE
    aux_node_22 = aux_node_20;
    aux_node_23 = aux_node_19;
    aux_not_24 = !aux_ic_21_led_0_1;
    aux_node_25 = aux_node_22;
    aux_and_26 = aux_not_24 && aux_node_23;
    // IC: REGISTER
    aux_ic_input_ic_0 = aux_ic_21_led_0;
    aux_ic_input_ic_1 = aux_and_26;
    aux_ic_27_node_6 = LOW;
    aux_ic_27_node_2 = LOW;
    aux_ic_27_node_12 = aux_ic_27_node_6;
    aux_ic_27_node_9 = aux_ic_27_node_12;
    //D FlipFlop
    if (aux_ic_27_node_12 && !aux_ic_27_d_flip_flop_13_0_q_inclk) { 
        aux_ic_27_d_flip_flop_13_0_q = aux_ic_27_d_flip_flop_13_0_q_last;
        aux_ic_27_d_flip_flop_13_1_q = !aux_ic_27_d_flip_flop_13_0_q_last;
    }
    if (!HIGH) { 
        aux_ic_27_d_flip_flop_13_0_q = HIGH; //Preset
        aux_ic_27_d_flip_flop_13_1_q = LOW;
    } else if (!HIGH) { 
        aux_ic_27_d_flip_flop_13_0_q = LOW; //Clear
        aux_ic_27_d_flip_flop_13_1_q = HIGH;
    }
    aux_ic_27_d_flip_flop_13_0_q_inclk = aux_ic_27_node_12;
    aux_ic_27_d_flip_flop_13_0_q_last = aux_ic_27_node_2;
    //End of D FlipFlop
    aux_ic_27_node_1 = aux_ic_27_node_9;
    //D FlipFlop
    if (aux_ic_27_node_9 && !aux_ic_27_d_flip_flop_5_0_q_inclk) { 
        aux_ic_27_d_flip_flop_5_0_q = aux_ic_27_d_flip_flop_5_0_q_last;
        aux_ic_27_d_flip_flop_5_1_q = !aux_ic_27_d_flip_flop_5_0_q_last;
    }
    if (!HIGH) { 
        aux_ic_27_d_flip_flop_5_0_q = HIGH; //Preset
        aux_ic_27_d_flip_flop_5_1_q = LOW;
    } else if (!HIGH) { 
        aux_ic_27_d_flip_flop_5_0_q = LOW; //Clear
        aux_ic_27_d_flip_flop_5_1_q = HIGH;
    }
    aux_ic_27_d_flip_flop_5_0_q_inclk = aux_ic_27_node_9;
    aux_ic_27_d_flip_flop_5_0_q_last = aux_ic_27_d_flip_flop_13_0_q;
    //End of D FlipFlop
    //D FlipFlop
    if (aux_ic_27_node_1 && !aux_ic_27_d_flip_flop_0_0_q_inclk) { 
        aux_ic_27_d_flip_flop_0_0_q = aux_ic_27_d_flip_flop_0_0_q_last;
        aux_ic_27_d_flip_flop_0_1_q = !aux_ic_27_d_flip_flop_0_0_q_last;
    }
    if (!HIGH) { 
        aux_ic_27_d_flip_flop_0_0_q = HIGH; //Preset
        aux_ic_27_d_flip_flop_0_1_q = LOW;
    } else if (!HIGH) { 
        aux_ic_27_d_flip_flop_0_0_q = LOW; //Clear
        aux_ic_27_d_flip_flop_0_1_q = HIGH;
    }
    aux_ic_27_d_flip_flop_0_0_q_inclk = aux_ic_27_node_1;
    aux_ic_27_d_flip_flop_0_0_q_last = aux_ic_27_d_flip_flop_5_0_q;
    //End of D FlipFlop
    aux_ic_27_node_11 = aux_ic_27_node_1;
    //D FlipFlop
    if (aux_ic_27_node_11 && !aux_ic_27_d_flip_flop_3_0_q_inclk) { 
        aux_ic_27_d_flip_flop_3_0_q = aux_ic_27_d_flip_flop_3_0_q_last;
        aux_ic_27_d_flip_flop_3_1_q = !aux_ic_27_d_flip_flop_3_0_q_last;
    }
    if (!HIGH) { 
        aux_ic_27_d_flip_flop_3_0_q = HIGH; //Preset
        aux_ic_27_d_flip_flop_3_1_q = LOW;
    } else if (!HIGH) { 
        aux_ic_27_d_flip_flop_3_0_q = LOW; //Clear
        aux_ic_27_d_flip_flop_3_1_q = HIGH;
    }
    aux_ic_27_d_flip_flop_3_0_q_inclk = aux_ic_27_node_11;
    aux_ic_27_d_flip_flop_3_0_q_last = aux_ic_27_d_flip_flop_0_0_q;
    //End of D FlipFlop
    aux_ic_27_node_4 = aux_ic_27_d_flip_flop_13_0_q;
    aux_ic_27_node_7 = aux_ic_27_d_flip_flop_0_0_q;
    aux_ic_27_node_8 = aux_ic_27_d_flip_flop_3_0_q;
    aux_ic_27_node_10 = aux_ic_27_d_flip_flop_5_0_q;
    aux_ic_27_led_0 = aux_ic_27_node_4;
    aux_ic_27_led_0_1 = aux_ic_27_node_10;
    aux_ic_27_led_0_2 = aux_ic_27_node_7;
    aux_ic_27_led_0_3 = aux_ic_27_node_8;
    // End IC: REGISTER
    aux_node_28 = aux_node_25;
    aux_and_29 = aux_ic_27_led_0 && aux_node_28;
    aux_and_30 = aux_ic_27_led_0_1 && aux_node_25;
    aux_and_31 = aux_ic_27_led_0_2 && aux_node_22;
    aux_and_32 = aux_ic_27_led_0_3 && aux_node_20;

    // Writing output data. //
    digitalWrite(led34_load_shift_0, aux_node_16);
    digitalWrite(led35_l3_0, aux_and_32);
    digitalWrite(led36_l2_0, aux_and_31);
    digitalWrite(led37_l0_0, aux_and_29);
    digitalWrite(led38_l1_0, aux_and_30);
}
