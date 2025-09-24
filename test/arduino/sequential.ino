// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //
//
// Target Board: Arduino UNO R3/R4
// Pin Usage: 6/18 pins
// Standard Arduino board with 20 I/O pins (14 digital + 6 analog)
//


/* ========= Inputs ========== */
const int push_button1_reset = A0;

/* ========= Outputs ========== */
const int led12_load_shift_0 = A1;
const int led14_l1_0 = A2;
const int led31_l3_0 = A3;
const int led33_l2_0 = A4;
const int led34_l0_0 = A5;

/* ====== Aux. Variables ====== */
bool push_button1_reset_val = false;
bool aux_node_0 = false;
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
bool aux_jk_flip_flop_4_0_q = false;
bool aux_jk_flip_flop_4_0_q_inclk = false;
bool aux_jk_flip_flop_4_1_q = true;
bool aux_jk_flip_flop_4_1_q_inclk = false;
bool aux_node_5 = false;
// IC: SERIALIZE
bool aux_serialize_6_led_0 = false;
bool aux_serialize_6_led_0_1 = false;
bool aux_serialize_6_node_0 = false;
bool aux_serialize_6_node_1 = false;
bool aux_serialize_6_node_2 = false;
bool aux_serialize_6_node_3 = false;
bool aux_serialize_6_node_4 = false;
bool aux_serialize_6_node_5 = false;
bool aux_serialize_6_node_6 = false;
bool aux_serialize_6_node_7 = false;
bool aux_serialize_6_node_8 = false;
bool aux_serialize_6_node_9 = false;
bool aux_serialize_6_node_10 = false;
bool aux_serialize_6_node_11 = false;
bool aux_serialize_6_node_12 = false;
bool aux_serialize_6_node_13 = false;
bool aux_serialize_6_node_14 = false;
bool aux_serialize_6_node_15 = false;
bool aux_serialize_6_node_16 = false;
bool aux_serialize_6_and_17 = false;
bool aux_serialize_6_node_18 = false;
bool aux_serialize_6_not_19 = false;
bool aux_serialize_6_and_20 = false;
bool aux_serialize_6_or_21 = false;
bool aux_serialize_6_and_22 = false;
bool aux_serialize_6_d_flip_flop_23_0_q = false;
bool aux_serialize_6_d_flip_flop_23_0_q_inclk = false;
bool aux_serialize_6_d_flip_flop_23_0_q_last = false;
bool aux_serialize_6_d_flip_flop_23_1_q = true;
bool aux_serialize_6_d_flip_flop_23_1_q_inclk = false;
bool aux_serialize_6_d_flip_flop_23_1_q_last = false;
bool aux_serialize_6_and_24 = false;
bool aux_serialize_6_or_25 = false;
bool aux_serialize_6_d_flip_flop_26_0_q = false;
bool aux_serialize_6_d_flip_flop_26_0_q_inclk = false;
bool aux_serialize_6_d_flip_flop_26_0_q_last = false;
bool aux_serialize_6_d_flip_flop_26_1_q = true;
bool aux_serialize_6_d_flip_flop_26_1_q_inclk = false;
bool aux_serialize_6_d_flip_flop_26_1_q_last = false;
bool aux_serialize_6_and_27 = false;
bool aux_serialize_6_d_flip_flop_28_0_q = false;
bool aux_serialize_6_d_flip_flop_28_0_q_inclk = false;
bool aux_serialize_6_d_flip_flop_28_0_q_last = false;
bool aux_serialize_6_d_flip_flop_28_1_q = true;
bool aux_serialize_6_d_flip_flop_28_1_q_inclk = false;
bool aux_serialize_6_d_flip_flop_28_1_q_last = false;
bool aux_serialize_6_node_29 = false;
bool aux_serialize_6_node_30 = false;
bool aux_serialize_6_node_31 = false;
bool aux_serialize_6_node_32 = false;
bool aux_serialize_6_and_33 = false;
bool aux_serialize_6_d_flip_flop_34_0_q = false;
bool aux_serialize_6_d_flip_flop_34_0_q_inclk = false;
bool aux_serialize_6_d_flip_flop_34_0_q_last = false;
bool aux_serialize_6_d_flip_flop_34_1_q = true;
bool aux_serialize_6_d_flip_flop_34_1_q_inclk = false;
bool aux_serialize_6_d_flip_flop_34_1_q_last = false;
bool aux_serialize_6_and_35 = false;
bool aux_serialize_6_or_36 = false;
bool aux_serialize_6_node_37 = false;
bool aux_ic_input_serialize_0 = false;
bool aux_ic_input_serialize_1 = false;
bool aux_ic_input_serialize_2 = false;
bool aux_ic_input_serialize_3 = false;
bool aux_ic_input_serialize_4 = false;
bool aux_ic_input_serialize_5 = false;
// End IC: SERIALIZE
// IC: REGISTER
bool aux_register_7_led_0 = false;
bool aux_register_7_led_0_1 = false;
bool aux_register_7_led_0_2 = false;
bool aux_register_7_led_0_3 = false;
bool aux_register_7_d_flip_flop_0_0_q = false;
bool aux_register_7_d_flip_flop_0_0_q_inclk = false;
bool aux_register_7_d_flip_flop_0_0_q_last = false;
bool aux_register_7_d_flip_flop_0_1_q = true;
bool aux_register_7_d_flip_flop_0_1_q_inclk = false;
bool aux_register_7_d_flip_flop_0_1_q_last = false;
bool aux_register_7_node_1 = false;
bool aux_register_7_node_2 = false;
bool aux_register_7_d_flip_flop_3_0_q = false;
bool aux_register_7_d_flip_flop_3_0_q_inclk = false;
bool aux_register_7_d_flip_flop_3_0_q_last = false;
bool aux_register_7_d_flip_flop_3_1_q = true;
bool aux_register_7_d_flip_flop_3_1_q_inclk = false;
bool aux_register_7_d_flip_flop_3_1_q_last = false;
bool aux_register_7_node_4 = false;
bool aux_register_7_d_flip_flop_5_0_q = false;
bool aux_register_7_d_flip_flop_5_0_q_inclk = false;
bool aux_register_7_d_flip_flop_5_0_q_last = false;
bool aux_register_7_d_flip_flop_5_1_q = true;
bool aux_register_7_d_flip_flop_5_1_q_inclk = false;
bool aux_register_7_d_flip_flop_5_1_q_last = false;
bool aux_register_7_node_6 = false;
bool aux_register_7_node_7 = false;
bool aux_register_7_node_8 = false;
bool aux_register_7_node_9 = false;
bool aux_register_7_node_10 = false;
bool aux_register_7_node_11 = false;
bool aux_register_7_node_12 = false;
bool aux_register_7_d_flip_flop_13_0_q = false;
bool aux_register_7_d_flip_flop_13_0_q_inclk = false;
bool aux_register_7_d_flip_flop_13_0_q_last = false;
bool aux_register_7_d_flip_flop_13_1_q = true;
bool aux_register_7_d_flip_flop_13_1_q_inclk = false;
bool aux_register_7_d_flip_flop_13_1_q_last = false;
bool aux_ic_input_register_0 = false;
bool aux_ic_input_register_1 = false;
// End IC: REGISTER
bool aux_node_8 = false;
bool aux_node_9 = false;
bool aux_node_10 = false;
bool aux_node_12 = false;
bool aux_and_14 = false;
bool aux_not_15 = false;
bool aux_node_16 = false;
bool aux_node_17 = false;
bool aux_and_18 = false;
bool aux_and_19 = false;
bool aux_and_20 = false;
bool aux_and_21 = false;
bool aux_node_22 = false;
bool aux_node_23 = false;
bool aux_node_24 = false;
bool aux_node_25 = false;
bool aux_node_26 = false;
bool aux_push_button_27 = false;
bool aux_not_28 = false;
bool aux_clock_29 = false;
unsigned long aux_clock_29_lastTime = 0;
const unsigned long aux_clock_29_interval = 62.5;
bool aux_clock_31 = false;
unsigned long aux_clock_31_lastTime = 0;
const unsigned long aux_clock_31_interval = 500;
bool aux_node_34 = false;
bool aux_node_35 = false;
bool aux_node_36 = false;
bool aux_node_37 = false;

void setup() {
    pinMode(push_button1_reset, INPUT);
    pinMode(led12_load_shift_0, OUTPUT);
    pinMode(led14_l1_0, OUTPUT);
    pinMode(led31_l3_0, OUTPUT);
    pinMode(led33_l2_0, OUTPUT);
    pinMode(led34_l0_0, OUTPUT);
}

void loop() {
    // Read input data
    push_button1_reset_val = digitalRead(push_button1_reset);

    // Update clocks
    unsigned long now = millis();
    if (now - aux_clock_29_lastTime >= aux_clock_29_interval) {
        aux_clock_29_lastTime = now;
        aux_clock_29 = !aux_clock_29;
    }
    if (now - aux_clock_31_lastTime >= aux_clock_31_interval) {
        aux_clock_31_lastTime = now;
        aux_clock_31 = !aux_clock_31;
    }

    // Update logic variables
    aux_node_0 = aux_clock_29;
    //JK FlipFlop
    if (aux_jk_flip_flop_2_0_q && !aux_jk_flip_flop_1_0_q_inclk) { 
        // Toggle mode (J=1, K=1)
        bool aux = aux_jk_flip_flop_1_0_q;
        aux_jk_flip_flop_1_0_q = aux_jk_flip_flop_1_1_q;
        aux_jk_flip_flop_1_1_q = aux;
    }
    if (!aux_node_34) { 
        aux_jk_flip_flop_1_0_q = true; // Preset
        aux_jk_flip_flop_1_1_q = false;
    }
    aux_jk_flip_flop_1_0_q_inclk = aux_jk_flip_flop_2_0_q;
    //End of JK FlipFlop
    //JK FlipFlop
    if (aux_jk_flip_flop_3_0_q && !aux_jk_flip_flop_2_0_q_inclk) { 
        // Toggle mode (J=1, K=1)
        bool aux = aux_jk_flip_flop_2_0_q;
        aux_jk_flip_flop_2_0_q = aux_jk_flip_flop_2_1_q;
        aux_jk_flip_flop_2_1_q = aux;
    }
    if (!aux_node_35) { 
        aux_jk_flip_flop_2_0_q = true; // Preset
        aux_jk_flip_flop_2_1_q = false;
    }
    aux_jk_flip_flop_2_0_q_inclk = aux_jk_flip_flop_3_0_q;
    //End of JK FlipFlop
    //JK FlipFlop
    if (aux_jk_flip_flop_4_0_q && !aux_jk_flip_flop_3_0_q_inclk) { 
        // Toggle mode (J=1, K=1)
        bool aux = aux_jk_flip_flop_3_0_q;
        aux_jk_flip_flop_3_0_q = aux_jk_flip_flop_3_1_q;
        aux_jk_flip_flop_3_1_q = aux;
    }
    if (!aux_node_36) { 
        aux_jk_flip_flop_3_0_q = true; // Preset
        aux_jk_flip_flop_3_1_q = false;
    }
    aux_jk_flip_flop_3_0_q_inclk = aux_jk_flip_flop_4_0_q;
    //End of JK FlipFlop
    //JK FlipFlop
    if (aux_clock_31 && !aux_jk_flip_flop_4_0_q_inclk) { 
        // Toggle mode (J=1, K=1)
        bool aux = aux_jk_flip_flop_4_0_q;
        aux_jk_flip_flop_4_0_q = aux_jk_flip_flop_4_1_q;
        aux_jk_flip_flop_4_1_q = aux;
    }
    if (!aux_node_37) { 
        aux_jk_flip_flop_4_0_q = true; // Preset
        aux_jk_flip_flop_4_1_q = false;
    }
    aux_jk_flip_flop_4_0_q_inclk = aux_clock_31;
    //End of JK FlipFlop
    aux_node_5 = aux_jk_flip_flop_4_1_q;
    // IC: SERIALIZE
    aux_ic_input_serialize_0 = aux_node_5;
    aux_ic_input_serialize_1 = aux_node_8;
    aux_ic_input_serialize_2 = aux_node_9;
    aux_ic_input_serialize_3 = aux_node_10;
    aux_ic_input_serialize_4 = aux_node_12;
    aux_ic_input_serialize_5 = aux_clock_29;
    aux_serialize_6_node_15 = true;
    aux_serialize_6_node_37 = aux_serialize_6_node_15;
    aux_serialize_6_node_14 = aux_ic_input_serialize_5;
    aux_serialize_6_not_19 = !aux_serialize_6_node_37;
    aux_serialize_6_node_29 = aux_ic_input_serialize_0;
    aux_serialize_6_node_0 = aux_serialize_6_node_14;
    aux_serialize_6_node_4 = aux_serialize_6_node_15;
    aux_serialize_6_node_7 = aux_serialize_6_node_4;
    //D FlipFlop
    if (aux_serialize_6_node_0 && !aux_serialize_6_d_flip_flop_34_0_q_inclk) { 
        aux_serialize_6_d_flip_flop_34_0_q = aux_serialize_6_d_flip_flop_34_0_q_last;
        aux_serialize_6_d_flip_flop_34_1_q = !aux_serialize_6_d_flip_flop_34_0_q_last;
    }
    if (!true) { 
        aux_serialize_6_d_flip_flop_34_0_q = true; // Preset
        aux_serialize_6_d_flip_flop_34_1_q = false;
    } else if (!true) { 
        aux_serialize_6_d_flip_flop_34_0_q = false; // Clear
        aux_serialize_6_d_flip_flop_34_1_q = true;
    }
    aux_serialize_6_d_flip_flop_34_0_q_inclk = aux_serialize_6_node_0;
    aux_serialize_6_d_flip_flop_34_0_q_last = aux_serialize_6_node_29;
    //End of D FlipFlop
    aux_serialize_6_node_11 = aux_serialize_6_not_19;
    aux_serialize_6_node_30 = true;
    aux_serialize_6_and_35 = aux_serialize_6_node_7 && aux_serialize_6_node_30;
    aux_serialize_6_and_33 = aux_serialize_6_node_11 && aux_serialize_6_d_flip_flop_34_0_q;
    aux_serialize_6_node_1 = aux_serialize_6_node_14;
    aux_serialize_6_or_36 = aux_serialize_6_and_35 || aux_serialize_6_and_33;
    aux_serialize_6_node_6 = aux_serialize_6_node_4;
    aux_serialize_6_node_10 = aux_serialize_6_not_19;
    aux_serialize_6_node_31 = aux_ic_input_serialize_2;
    //D FlipFlop
    if (aux_serialize_6_node_1 && !aux_serialize_6_d_flip_flop_28_0_q_inclk) { 
        aux_serialize_6_d_flip_flop_28_0_q = aux_serialize_6_d_flip_flop_28_0_q_last;
        aux_serialize_6_d_flip_flop_28_1_q = !aux_serialize_6_d_flip_flop_28_0_q_last;
    }
    if (!true) { 
        aux_serialize_6_d_flip_flop_28_0_q = true; // Preset
        aux_serialize_6_d_flip_flop_28_1_q = false;
    } else if (!true) { 
        aux_serialize_6_d_flip_flop_28_0_q = false; // Clear
        aux_serialize_6_d_flip_flop_28_1_q = true;
    }
    aux_serialize_6_d_flip_flop_28_0_q_inclk = aux_serialize_6_node_1;
    aux_serialize_6_d_flip_flop_28_0_q_last = aux_serialize_6_or_36;
    //End of D FlipFlop
    aux_serialize_6_and_27 = aux_serialize_6_node_10 && aux_serialize_6_d_flip_flop_28_0_q;
    aux_serialize_6_and_24 = aux_serialize_6_node_6 && aux_serialize_6_node_31;
    aux_serialize_6_node_2 = aux_serialize_6_node_14;
    aux_serialize_6_or_25 = aux_serialize_6_and_24 || aux_serialize_6_and_27;
    //D FlipFlop
    if (aux_serialize_6_node_2 && !aux_serialize_6_d_flip_flop_26_0_q_inclk) { 
        aux_serialize_6_d_flip_flop_26_0_q = aux_serialize_6_d_flip_flop_26_0_q_last;
        aux_serialize_6_d_flip_flop_26_1_q = !aux_serialize_6_d_flip_flop_26_0_q_last;
    }
    if (!true) { 
        aux_serialize_6_d_flip_flop_26_0_q = true; // Preset
        aux_serialize_6_d_flip_flop_26_1_q = false;
    } else if (!true) { 
        aux_serialize_6_d_flip_flop_26_0_q = false; // Clear
        aux_serialize_6_d_flip_flop_26_1_q = true;
    }
    aux_serialize_6_d_flip_flop_26_0_q_inclk = aux_serialize_6_node_2;
    aux_serialize_6_d_flip_flop_26_0_q_last = aux_serialize_6_or_25;
    //End of D FlipFlop
    aux_serialize_6_node_9 = aux_serialize_6_not_19;
    aux_serialize_6_node_32 = true;
    aux_serialize_6_node_5 = aux_serialize_6_node_4;
    aux_serialize_6_and_22 = aux_serialize_6_node_5 && aux_serialize_6_node_32;
    aux_serialize_6_and_20 = aux_serialize_6_node_9 && aux_serialize_6_d_flip_flop_26_0_q;
    aux_serialize_6_or_21 = aux_serialize_6_and_22 || aux_serialize_6_and_20;
    aux_serialize_6_node_3 = aux_serialize_6_node_14;
    //D FlipFlop
    if (aux_serialize_6_node_3 && !aux_serialize_6_d_flip_flop_23_0_q_inclk) { 
        aux_serialize_6_d_flip_flop_23_0_q = aux_serialize_6_d_flip_flop_23_0_q_last;
        aux_serialize_6_d_flip_flop_23_1_q = !aux_serialize_6_d_flip_flop_23_0_q_last;
    }
    if (!true) { 
        aux_serialize_6_d_flip_flop_23_0_q = true; // Preset
        aux_serialize_6_d_flip_flop_23_1_q = false;
    } else if (!true) { 
        aux_serialize_6_d_flip_flop_23_0_q = false; // Clear
        aux_serialize_6_d_flip_flop_23_1_q = true;
    }
    aux_serialize_6_d_flip_flop_23_0_q_inclk = aux_serialize_6_node_3;
    aux_serialize_6_d_flip_flop_23_0_q_last = aux_serialize_6_or_21;
    //End of D FlipFlop
    aux_serialize_6_node_13 = aux_serialize_6_node_37;
    aux_serialize_6_node_8 = aux_serialize_6_not_19;
    aux_serialize_6_node_12 = aux_serialize_6_node_13;
    aux_serialize_6_and_17 = aux_serialize_6_node_8 && aux_serialize_6_d_flip_flop_23_0_q;
    aux_serialize_6_node_18 = aux_serialize_6_and_17;
    aux_serialize_6_node_16 = aux_serialize_6_node_12;
    aux_serialize_6_led_0 = aux_serialize_6_node_18;
    aux_serialize_6_led_0_1 = aux_serialize_6_node_16;
    // End IC: SERIALIZE
    // IC: REGISTER
    aux_ic_input_register_0 = aux_serialize_6_led_0;
    aux_ic_input_register_1 = aux_and_14;
    aux_register_7_node_6 = aux_ic_input_register_1;
    aux_register_7_node_2 = aux_ic_input_register_0;
    aux_register_7_node_12 = aux_register_7_node_6;
    aux_register_7_node_9 = aux_register_7_node_12;
    //D FlipFlop
    if (aux_register_7_node_12 && !aux_register_7_d_flip_flop_13_0_q_inclk) { 
        aux_register_7_d_flip_flop_13_0_q = aux_register_7_d_flip_flop_13_0_q_last;
        aux_register_7_d_flip_flop_13_1_q = !aux_register_7_d_flip_flop_13_0_q_last;
    }
    if (!true) { 
        aux_register_7_d_flip_flop_13_0_q = true; // Preset
        aux_register_7_d_flip_flop_13_1_q = false;
    } else if (!true) { 
        aux_register_7_d_flip_flop_13_0_q = false; // Clear
        aux_register_7_d_flip_flop_13_1_q = true;
    }
    aux_register_7_d_flip_flop_13_0_q_inclk = aux_register_7_node_12;
    aux_register_7_d_flip_flop_13_0_q_last = aux_register_7_node_2;
    //End of D FlipFlop
    aux_register_7_node_1 = aux_register_7_node_9;
    //D FlipFlop
    if (aux_register_7_node_9 && !aux_register_7_d_flip_flop_5_0_q_inclk) { 
        aux_register_7_d_flip_flop_5_0_q = aux_register_7_d_flip_flop_5_0_q_last;
        aux_register_7_d_flip_flop_5_1_q = !aux_register_7_d_flip_flop_5_0_q_last;
    }
    if (!true) { 
        aux_register_7_d_flip_flop_5_0_q = true; // Preset
        aux_register_7_d_flip_flop_5_1_q = false;
    } else if (!true) { 
        aux_register_7_d_flip_flop_5_0_q = false; // Clear
        aux_register_7_d_flip_flop_5_1_q = true;
    }
    aux_register_7_d_flip_flop_5_0_q_inclk = aux_register_7_node_9;
    aux_register_7_d_flip_flop_5_0_q_last = aux_register_7_d_flip_flop_13_0_q;
    //End of D FlipFlop
    //D FlipFlop
    if (aux_register_7_node_1 && !aux_register_7_d_flip_flop_0_0_q_inclk) { 
        aux_register_7_d_flip_flop_0_0_q = aux_register_7_d_flip_flop_0_0_q_last;
        aux_register_7_d_flip_flop_0_1_q = !aux_register_7_d_flip_flop_0_0_q_last;
    }
    if (!true) { 
        aux_register_7_d_flip_flop_0_0_q = true; // Preset
        aux_register_7_d_flip_flop_0_1_q = false;
    } else if (!true) { 
        aux_register_7_d_flip_flop_0_0_q = false; // Clear
        aux_register_7_d_flip_flop_0_1_q = true;
    }
    aux_register_7_d_flip_flop_0_0_q_inclk = aux_register_7_node_1;
    aux_register_7_d_flip_flop_0_0_q_last = aux_register_7_d_flip_flop_5_0_q;
    //End of D FlipFlop
    aux_register_7_node_11 = aux_register_7_node_1;
    //D FlipFlop
    if (aux_register_7_node_11 && !aux_register_7_d_flip_flop_3_0_q_inclk) { 
        aux_register_7_d_flip_flop_3_0_q = aux_register_7_d_flip_flop_3_0_q_last;
        aux_register_7_d_flip_flop_3_1_q = !aux_register_7_d_flip_flop_3_0_q_last;
    }
    if (!true) { 
        aux_register_7_d_flip_flop_3_0_q = true; // Preset
        aux_register_7_d_flip_flop_3_1_q = false;
    } else if (!true) { 
        aux_register_7_d_flip_flop_3_0_q = false; // Clear
        aux_register_7_d_flip_flop_3_1_q = true;
    }
    aux_register_7_d_flip_flop_3_0_q_inclk = aux_register_7_node_11;
    aux_register_7_d_flip_flop_3_0_q_last = aux_register_7_d_flip_flop_0_0_q;
    //End of D FlipFlop
    aux_register_7_node_4 = aux_register_7_d_flip_flop_13_0_q;
    aux_register_7_node_7 = aux_register_7_d_flip_flop_0_0_q;
    aux_register_7_node_8 = aux_register_7_d_flip_flop_3_0_q;
    aux_register_7_node_10 = aux_register_7_d_flip_flop_5_0_q;
    aux_register_7_led_0 = aux_register_7_node_4;
    aux_register_7_led_0_1 = aux_register_7_node_10;
    aux_register_7_led_0_2 = aux_register_7_node_7;
    aux_register_7_led_0_3 = aux_register_7_node_8;
    // End IC: REGISTER
    aux_node_8 = aux_jk_flip_flop_3_1_q;
    aux_node_9 = aux_jk_flip_flop_2_1_q;
    aux_node_10 = aux_jk_flip_flop_1_1_q;
    aux_node_12 = aux_clock_31;
    aux_and_14 = aux_not_15 && aux_node_16;
    aux_not_15 = !aux_serialize_6_led_0_1;
    aux_node_16 = aux_node_0;
    aux_node_17 = aux_node_12;
    aux_and_18 = aux_register_7_led_0_3 && aux_node_22;
    aux_and_19 = aux_register_7_led_0_2 && aux_node_23;
    aux_and_20 = aux_register_7_led_0_1 && aux_node_24;
    aux_and_21 = aux_register_7_led_0 && aux_node_25;
    aux_node_22 = aux_node_17;
    aux_node_23 = aux_node_22;
    aux_node_24 = aux_node_23;
    aux_node_25 = aux_node_24;
    aux_node_26 = aux_not_28;
    aux_push_button_27 = push_button1_reset_val;
    aux_not_28 = !aux_push_button_27;
    aux_node_34 = aux_node_35;
    aux_node_35 = aux_node_36;
    aux_node_36 = aux_node_37;
    aux_node_37 = aux_node_26;

    // Write output data
    digitalWrite(led12_load_shift_0, aux_node_17);
    digitalWrite(led14_l1_0, aux_and_20);
    digitalWrite(led31_l3_0, aux_and_18);
    digitalWrite(led33_l2_0, aux_and_19);
    digitalWrite(led34_l0_0, aux_and_21);
}
