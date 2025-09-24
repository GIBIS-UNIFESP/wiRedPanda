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
const int led34_load_shift_0 = A1;
const int led35_l1_0 = A2;
const int led36_l3_0 = A3;
const int led37_l2_0 = A4;
const int led38_l0_0 = A5;

/* ====== Aux. Variables ====== */
bool push_button1_reset_val = false;
bool aux_push_button_0 = false;
bool aux_not_1 = false;
bool aux_node_2 = false;
bool aux_clock_3 = false;
unsigned long aux_clock_3_lastTime = 0;
const unsigned long aux_clock_3_interval = 500;
bool aux_node_4 = false;
bool aux_jk_flip_flop_5_0_q = false;
bool aux_jk_flip_flop_5_0_q_inclk = false;
bool aux_jk_flip_flop_5_1_q = true;
bool aux_jk_flip_flop_5_1_q_inclk = false;
bool aux_node_6 = false;
bool aux_jk_flip_flop_7_0_q = false;
bool aux_jk_flip_flop_7_0_q_inclk = false;
bool aux_jk_flip_flop_7_1_q = true;
bool aux_jk_flip_flop_7_1_q_inclk = false;
bool aux_node_8 = false;
bool aux_jk_flip_flop_9_0_q = false;
bool aux_jk_flip_flop_9_0_q_inclk = false;
bool aux_jk_flip_flop_9_1_q = true;
bool aux_jk_flip_flop_9_1_q_inclk = false;
bool aux_node_10 = false;
bool aux_jk_flip_flop_11_0_q = false;
bool aux_jk_flip_flop_11_0_q_inclk = false;
bool aux_jk_flip_flop_11_1_q = true;
bool aux_jk_flip_flop_11_1_q_inclk = false;
bool aux_node_12 = false;
bool aux_node_13 = false;
bool aux_node_14 = false;
bool aux_node_15 = false;
bool aux_node_16 = false;
bool aux_node_17 = false;
bool aux_clock_18 = false;
unsigned long aux_clock_18_lastTime = 0;
const unsigned long aux_clock_18_interval = 62.5;
bool aux_node_19 = false;
// IC: SERIALIZE
bool aux_serialize_20_led_0 = false;
bool aux_serialize_20_led_0_1 = false;
bool aux_serialize_20_node_0 = false;
bool aux_serialize_20_node_1 = false;
bool aux_serialize_20_node_2 = false;
bool aux_serialize_20_node_3 = false;
bool aux_serialize_20_node_4 = false;
bool aux_serialize_20_node_5 = false;
bool aux_serialize_20_node_6 = false;
bool aux_serialize_20_node_7 = false;
bool aux_serialize_20_node_8 = false;
bool aux_serialize_20_node_9 = false;
bool aux_serialize_20_node_10 = false;
bool aux_serialize_20_node_11 = false;
bool aux_serialize_20_node_12 = false;
bool aux_serialize_20_node_13 = false;
bool aux_serialize_20_node_14 = false;
bool aux_serialize_20_node_15 = false;
bool aux_serialize_20_node_16 = false;
bool aux_serialize_20_and_17 = false;
bool aux_serialize_20_node_18 = false;
bool aux_serialize_20_not_19 = false;
bool aux_serialize_20_and_20 = false;
bool aux_serialize_20_or_21 = false;
bool aux_serialize_20_and_22 = false;
bool aux_serialize_20_d_flip_flop_23_0_q = false;
bool aux_serialize_20_d_flip_flop_23_0_q_inclk = false;
bool aux_serialize_20_d_flip_flop_23_0_q_last = false;
bool aux_serialize_20_d_flip_flop_23_1_q = true;
bool aux_serialize_20_d_flip_flop_23_1_q_inclk = false;
bool aux_serialize_20_d_flip_flop_23_1_q_last = false;
bool aux_serialize_20_and_24 = false;
bool aux_serialize_20_or_25 = false;
bool aux_serialize_20_d_flip_flop_26_0_q = false;
bool aux_serialize_20_d_flip_flop_26_0_q_inclk = false;
bool aux_serialize_20_d_flip_flop_26_0_q_last = false;
bool aux_serialize_20_d_flip_flop_26_1_q = true;
bool aux_serialize_20_d_flip_flop_26_1_q_inclk = false;
bool aux_serialize_20_d_flip_flop_26_1_q_last = false;
bool aux_serialize_20_and_27 = false;
bool aux_serialize_20_d_flip_flop_28_0_q = false;
bool aux_serialize_20_d_flip_flop_28_0_q_inclk = false;
bool aux_serialize_20_d_flip_flop_28_0_q_last = false;
bool aux_serialize_20_d_flip_flop_28_1_q = true;
bool aux_serialize_20_d_flip_flop_28_1_q_inclk = false;
bool aux_serialize_20_d_flip_flop_28_1_q_last = false;
bool aux_serialize_20_node_29 = false;
bool aux_serialize_20_node_30 = false;
bool aux_serialize_20_node_31 = false;
bool aux_serialize_20_node_32 = false;
bool aux_serialize_20_and_33 = false;
bool aux_serialize_20_d_flip_flop_34_0_q = false;
bool aux_serialize_20_d_flip_flop_34_0_q_inclk = false;
bool aux_serialize_20_d_flip_flop_34_0_q_last = false;
bool aux_serialize_20_d_flip_flop_34_1_q = true;
bool aux_serialize_20_d_flip_flop_34_1_q_inclk = false;
bool aux_serialize_20_d_flip_flop_34_1_q_last = false;
bool aux_serialize_20_and_35 = false;
bool aux_serialize_20_or_36 = false;
bool aux_serialize_20_node_37 = false;
bool aux_ic_input_serialize_0 = false;
bool aux_ic_input_serialize_1 = false;
bool aux_ic_input_serialize_2 = false;
bool aux_ic_input_serialize_3 = false;
bool aux_ic_input_serialize_4 = false;
bool aux_ic_input_serialize_5 = false;
// End IC: SERIALIZE
bool aux_node_21 = false;
bool aux_not_22 = false;
bool aux_node_23 = false;
bool aux_node_24 = false;
bool aux_and_25 = false;
bool aux_node_26 = false;
// IC: REGISTER
bool aux_register_27_led_0 = false;
bool aux_register_27_led_0_1 = false;
bool aux_register_27_led_0_2 = false;
bool aux_register_27_led_0_3 = false;
bool aux_register_27_d_flip_flop_0_0_q = false;
bool aux_register_27_d_flip_flop_0_0_q_inclk = false;
bool aux_register_27_d_flip_flop_0_0_q_last = false;
bool aux_register_27_d_flip_flop_0_1_q = true;
bool aux_register_27_d_flip_flop_0_1_q_inclk = false;
bool aux_register_27_d_flip_flop_0_1_q_last = false;
bool aux_register_27_node_1 = false;
bool aux_register_27_node_2 = false;
bool aux_register_27_d_flip_flop_3_0_q = false;
bool aux_register_27_d_flip_flop_3_0_q_inclk = false;
bool aux_register_27_d_flip_flop_3_0_q_last = false;
bool aux_register_27_d_flip_flop_3_1_q = true;
bool aux_register_27_d_flip_flop_3_1_q_inclk = false;
bool aux_register_27_d_flip_flop_3_1_q_last = false;
bool aux_register_27_node_4 = false;
bool aux_register_27_d_flip_flop_5_0_q = false;
bool aux_register_27_d_flip_flop_5_0_q_inclk = false;
bool aux_register_27_d_flip_flop_5_0_q_last = false;
bool aux_register_27_d_flip_flop_5_1_q = true;
bool aux_register_27_d_flip_flop_5_1_q_inclk = false;
bool aux_register_27_d_flip_flop_5_1_q_last = false;
bool aux_register_27_node_6 = false;
bool aux_register_27_node_7 = false;
bool aux_register_27_node_8 = false;
bool aux_register_27_node_9 = false;
bool aux_register_27_node_10 = false;
bool aux_register_27_node_11 = false;
bool aux_register_27_node_12 = false;
bool aux_register_27_d_flip_flop_13_0_q = false;
bool aux_register_27_d_flip_flop_13_0_q_inclk = false;
bool aux_register_27_d_flip_flop_13_0_q_last = false;
bool aux_register_27_d_flip_flop_13_1_q = true;
bool aux_register_27_d_flip_flop_13_1_q_inclk = false;
bool aux_register_27_d_flip_flop_13_1_q_last = false;
bool aux_ic_input_register_0 = false;
bool aux_ic_input_register_1 = false;
// End IC: REGISTER
bool aux_node_28 = false;
bool aux_and_29 = false;
bool aux_and_30 = false;
bool aux_and_31 = false;
bool aux_and_32 = false;

void setup() {
    pinMode(push_button1_reset, INPUT);
    pinMode(led34_load_shift_0, OUTPUT);
    pinMode(led35_l1_0, OUTPUT);
    pinMode(led36_l3_0, OUTPUT);
    pinMode(led37_l2_0, OUTPUT);
    pinMode(led38_l0_0, OUTPUT);
}

void loop() {
    // Read input data
    push_button1_reset_val = digitalRead(push_button1_reset);

    // Update clocks
    unsigned long now = millis();
    if (now - aux_clock_3_lastTime >= aux_clock_3_interval) {
        aux_clock_3_lastTime = now;
        aux_clock_3 = !aux_clock_3;
    }
    if (now - aux_clock_18_lastTime >= aux_clock_18_interval) {
        aux_clock_18_lastTime = now;
        aux_clock_18 = !aux_clock_18;
    }

    // Update logic variables
    aux_push_button_0 = push_button1_reset_val;
    aux_not_1 = !aux_push_button_0;
    aux_node_2 = aux_not_1;
    aux_node_4 = aux_node_2;
    //JK FlipFlop
    if (aux_clock_3 && !aux_jk_flip_flop_5_0_q_inclk) { 
        // Toggle mode (J=1, K=1)
        bool aux = aux_jk_flip_flop_5_0_q;
        aux_jk_flip_flop_5_0_q = aux_jk_flip_flop_5_1_q;
        aux_jk_flip_flop_5_1_q = aux;
    }
    if (!aux_node_4) { 
        aux_jk_flip_flop_5_0_q = true; // Preset
        aux_jk_flip_flop_5_1_q = false;
    }
    aux_jk_flip_flop_5_0_q_inclk = aux_clock_3;
    //End of JK FlipFlop
    aux_node_6 = aux_node_4;
    //JK FlipFlop
    if (aux_jk_flip_flop_5_0_q && !aux_jk_flip_flop_7_0_q_inclk) { 
        // Toggle mode (J=1, K=1)
        bool aux = aux_jk_flip_flop_7_0_q;
        aux_jk_flip_flop_7_0_q = aux_jk_flip_flop_7_1_q;
        aux_jk_flip_flop_7_1_q = aux;
    }
    if (!aux_node_6) { 
        aux_jk_flip_flop_7_0_q = true; // Preset
        aux_jk_flip_flop_7_1_q = false;
    }
    aux_jk_flip_flop_7_0_q_inclk = aux_jk_flip_flop_5_0_q;
    //End of JK FlipFlop
    aux_node_8 = aux_node_6;
    //JK FlipFlop
    if (aux_jk_flip_flop_7_0_q && !aux_jk_flip_flop_9_0_q_inclk) { 
        // Toggle mode (J=1, K=1)
        bool aux = aux_jk_flip_flop_9_0_q;
        aux_jk_flip_flop_9_0_q = aux_jk_flip_flop_9_1_q;
        aux_jk_flip_flop_9_1_q = aux;
    }
    if (!aux_node_8) { 
        aux_jk_flip_flop_9_0_q = true; // Preset
        aux_jk_flip_flop_9_1_q = false;
    }
    aux_jk_flip_flop_9_0_q_inclk = aux_jk_flip_flop_7_0_q;
    //End of JK FlipFlop
    aux_node_10 = aux_node_8;
    //JK FlipFlop
    if (aux_jk_flip_flop_9_0_q && !aux_jk_flip_flop_11_0_q_inclk) { 
        // Toggle mode (J=1, K=1)
        bool aux = aux_jk_flip_flop_11_0_q;
        aux_jk_flip_flop_11_0_q = aux_jk_flip_flop_11_1_q;
        aux_jk_flip_flop_11_1_q = aux;
    }
    if (!aux_node_10) { 
        aux_jk_flip_flop_11_0_q = true; // Preset
        aux_jk_flip_flop_11_1_q = false;
    }
    aux_jk_flip_flop_11_0_q_inclk = aux_jk_flip_flop_9_0_q;
    //End of JK FlipFlop
    aux_node_12 = aux_clock_3;
    aux_node_13 = aux_jk_flip_flop_5_1_q;
    aux_node_14 = aux_jk_flip_flop_7_1_q;
    aux_node_15 = aux_jk_flip_flop_9_1_q;
    aux_node_16 = aux_jk_flip_flop_11_1_q;
    aux_node_17 = aux_node_12;
    aux_node_19 = aux_clock_18;
    // IC: SERIALIZE
    aux_ic_input_serialize_0 = aux_node_13;
    aux_ic_input_serialize_1 = aux_node_14;
    aux_ic_input_serialize_2 = aux_node_15;
    aux_ic_input_serialize_3 = aux_node_16;
    aux_ic_input_serialize_4 = aux_node_12;
    aux_ic_input_serialize_5 = aux_clock_18;
    aux_serialize_20_node_15 = true;
    aux_serialize_20_node_14 = aux_ic_input_serialize_5;
    aux_serialize_20_node_37 = aux_serialize_20_node_15;
    aux_serialize_20_node_0 = aux_serialize_20_node_14;
    aux_serialize_20_node_4 = aux_serialize_20_node_15;
    aux_serialize_20_not_19 = !aux_serialize_20_node_37;
    aux_serialize_20_node_29 = aux_ic_input_serialize_0;
    aux_serialize_20_node_7 = aux_serialize_20_node_4;
    aux_serialize_20_node_11 = aux_serialize_20_not_19;
    aux_serialize_20_node_30 = true;
    //D FlipFlop
    if (aux_serialize_20_node_0 && !aux_serialize_20_d_flip_flop_34_0_q_inclk) { 
        aux_serialize_20_d_flip_flop_34_0_q = aux_serialize_20_d_flip_flop_34_0_q_last;
        aux_serialize_20_d_flip_flop_34_1_q = !aux_serialize_20_d_flip_flop_34_0_q_last;
    }
    if (!true) { 
        aux_serialize_20_d_flip_flop_34_0_q = true; // Preset
        aux_serialize_20_d_flip_flop_34_1_q = false;
    } else if (!true) { 
        aux_serialize_20_d_flip_flop_34_0_q = false; // Clear
        aux_serialize_20_d_flip_flop_34_1_q = true;
    }
    aux_serialize_20_d_flip_flop_34_0_q_inclk = aux_serialize_20_node_0;
    aux_serialize_20_d_flip_flop_34_0_q_last = aux_serialize_20_node_29;
    //End of D FlipFlop
    aux_serialize_20_and_33 = aux_serialize_20_node_11 && aux_serialize_20_d_flip_flop_34_0_q;
    aux_serialize_20_and_35 = aux_serialize_20_node_7 && aux_serialize_20_node_30;
    aux_serialize_20_node_1 = aux_serialize_20_node_14;
    aux_serialize_20_or_36 = aux_serialize_20_and_35 || aux_serialize_20_and_33;
    aux_serialize_20_node_6 = aux_serialize_20_node_4;
    aux_serialize_20_node_10 = aux_serialize_20_not_19;
    //D FlipFlop
    if (aux_serialize_20_node_1 && !aux_serialize_20_d_flip_flop_28_0_q_inclk) { 
        aux_serialize_20_d_flip_flop_28_0_q = aux_serialize_20_d_flip_flop_28_0_q_last;
        aux_serialize_20_d_flip_flop_28_1_q = !aux_serialize_20_d_flip_flop_28_0_q_last;
    }
    if (!true) { 
        aux_serialize_20_d_flip_flop_28_0_q = true; // Preset
        aux_serialize_20_d_flip_flop_28_1_q = false;
    } else if (!true) { 
        aux_serialize_20_d_flip_flop_28_0_q = false; // Clear
        aux_serialize_20_d_flip_flop_28_1_q = true;
    }
    aux_serialize_20_d_flip_flop_28_0_q_inclk = aux_serialize_20_node_1;
    aux_serialize_20_d_flip_flop_28_0_q_last = aux_serialize_20_or_36;
    //End of D FlipFlop
    aux_serialize_20_node_31 = aux_ic_input_serialize_2;
    aux_serialize_20_and_24 = aux_serialize_20_node_6 && aux_serialize_20_node_31;
    aux_serialize_20_and_27 = aux_serialize_20_node_10 && aux_serialize_20_d_flip_flop_28_0_q;
    aux_serialize_20_node_2 = aux_serialize_20_node_14;
    aux_serialize_20_or_25 = aux_serialize_20_and_24 || aux_serialize_20_and_27;
    aux_serialize_20_node_5 = aux_serialize_20_node_4;
    aux_serialize_20_node_9 = aux_serialize_20_not_19;
    //D FlipFlop
    if (aux_serialize_20_node_2 && !aux_serialize_20_d_flip_flop_26_0_q_inclk) { 
        aux_serialize_20_d_flip_flop_26_0_q = aux_serialize_20_d_flip_flop_26_0_q_last;
        aux_serialize_20_d_flip_flop_26_1_q = !aux_serialize_20_d_flip_flop_26_0_q_last;
    }
    if (!true) { 
        aux_serialize_20_d_flip_flop_26_0_q = true; // Preset
        aux_serialize_20_d_flip_flop_26_1_q = false;
    } else if (!true) { 
        aux_serialize_20_d_flip_flop_26_0_q = false; // Clear
        aux_serialize_20_d_flip_flop_26_1_q = true;
    }
    aux_serialize_20_d_flip_flop_26_0_q_inclk = aux_serialize_20_node_2;
    aux_serialize_20_d_flip_flop_26_0_q_last = aux_serialize_20_or_25;
    //End of D FlipFlop
    aux_serialize_20_node_32 = true;
    aux_serialize_20_and_20 = aux_serialize_20_node_9 && aux_serialize_20_d_flip_flop_26_0_q;
    aux_serialize_20_and_22 = aux_serialize_20_node_5 && aux_serialize_20_node_32;
    aux_serialize_20_node_3 = aux_serialize_20_node_14;
    aux_serialize_20_or_21 = aux_serialize_20_and_22 || aux_serialize_20_and_20;
    aux_serialize_20_node_8 = aux_serialize_20_not_19;
    aux_serialize_20_node_13 = aux_serialize_20_node_37;
    //D FlipFlop
    if (aux_serialize_20_node_3 && !aux_serialize_20_d_flip_flop_23_0_q_inclk) { 
        aux_serialize_20_d_flip_flop_23_0_q = aux_serialize_20_d_flip_flop_23_0_q_last;
        aux_serialize_20_d_flip_flop_23_1_q = !aux_serialize_20_d_flip_flop_23_0_q_last;
    }
    if (!true) { 
        aux_serialize_20_d_flip_flop_23_0_q = true; // Preset
        aux_serialize_20_d_flip_flop_23_1_q = false;
    } else if (!true) { 
        aux_serialize_20_d_flip_flop_23_0_q = false; // Clear
        aux_serialize_20_d_flip_flop_23_1_q = true;
    }
    aux_serialize_20_d_flip_flop_23_0_q_inclk = aux_serialize_20_node_3;
    aux_serialize_20_d_flip_flop_23_0_q_last = aux_serialize_20_or_21;
    //End of D FlipFlop
    aux_serialize_20_node_12 = aux_serialize_20_node_13;
    aux_serialize_20_and_17 = aux_serialize_20_node_8 && aux_serialize_20_d_flip_flop_23_0_q;
    aux_serialize_20_node_16 = aux_serialize_20_node_12;
    aux_serialize_20_node_18 = aux_serialize_20_and_17;
    aux_serialize_20_led_0 = aux_serialize_20_node_18;
    aux_serialize_20_led_0_1 = aux_serialize_20_node_16;
    // End IC: SERIALIZE
    aux_node_21 = aux_node_17;
    aux_not_22 = !aux_serialize_20_led_0_1;
    aux_node_23 = aux_node_19;
    aux_node_24 = aux_node_21;
    aux_and_25 = aux_not_22 && aux_node_23;
    aux_node_26 = aux_node_24;
    // IC: REGISTER
    aux_ic_input_register_0 = aux_serialize_20_led_0;
    aux_ic_input_register_1 = aux_and_25;
    aux_register_27_node_6 = aux_ic_input_register_1;
    aux_register_27_node_2 = aux_ic_input_register_0;
    aux_register_27_node_12 = aux_register_27_node_6;
    aux_register_27_node_9 = aux_register_27_node_12;
    //D FlipFlop
    if (aux_register_27_node_12 && !aux_register_27_d_flip_flop_13_0_q_inclk) { 
        aux_register_27_d_flip_flop_13_0_q = aux_register_27_d_flip_flop_13_0_q_last;
        aux_register_27_d_flip_flop_13_1_q = !aux_register_27_d_flip_flop_13_0_q_last;
    }
    if (!true) { 
        aux_register_27_d_flip_flop_13_0_q = true; // Preset
        aux_register_27_d_flip_flop_13_1_q = false;
    } else if (!true) { 
        aux_register_27_d_flip_flop_13_0_q = false; // Clear
        aux_register_27_d_flip_flop_13_1_q = true;
    }
    aux_register_27_d_flip_flop_13_0_q_inclk = aux_register_27_node_12;
    aux_register_27_d_flip_flop_13_0_q_last = aux_register_27_node_2;
    //End of D FlipFlop
    aux_register_27_node_1 = aux_register_27_node_9;
    //D FlipFlop
    if (aux_register_27_node_9 && !aux_register_27_d_flip_flop_5_0_q_inclk) { 
        aux_register_27_d_flip_flop_5_0_q = aux_register_27_d_flip_flop_5_0_q_last;
        aux_register_27_d_flip_flop_5_1_q = !aux_register_27_d_flip_flop_5_0_q_last;
    }
    if (!true) { 
        aux_register_27_d_flip_flop_5_0_q = true; // Preset
        aux_register_27_d_flip_flop_5_1_q = false;
    } else if (!true) { 
        aux_register_27_d_flip_flop_5_0_q = false; // Clear
        aux_register_27_d_flip_flop_5_1_q = true;
    }
    aux_register_27_d_flip_flop_5_0_q_inclk = aux_register_27_node_9;
    aux_register_27_d_flip_flop_5_0_q_last = aux_register_27_d_flip_flop_13_0_q;
    //End of D FlipFlop
    //D FlipFlop
    if (aux_register_27_node_1 && !aux_register_27_d_flip_flop_0_0_q_inclk) { 
        aux_register_27_d_flip_flop_0_0_q = aux_register_27_d_flip_flop_0_0_q_last;
        aux_register_27_d_flip_flop_0_1_q = !aux_register_27_d_flip_flop_0_0_q_last;
    }
    if (!true) { 
        aux_register_27_d_flip_flop_0_0_q = true; // Preset
        aux_register_27_d_flip_flop_0_1_q = false;
    } else if (!true) { 
        aux_register_27_d_flip_flop_0_0_q = false; // Clear
        aux_register_27_d_flip_flop_0_1_q = true;
    }
    aux_register_27_d_flip_flop_0_0_q_inclk = aux_register_27_node_1;
    aux_register_27_d_flip_flop_0_0_q_last = aux_register_27_d_flip_flop_5_0_q;
    //End of D FlipFlop
    aux_register_27_node_11 = aux_register_27_node_1;
    //D FlipFlop
    if (aux_register_27_node_11 && !aux_register_27_d_flip_flop_3_0_q_inclk) { 
        aux_register_27_d_flip_flop_3_0_q = aux_register_27_d_flip_flop_3_0_q_last;
        aux_register_27_d_flip_flop_3_1_q = !aux_register_27_d_flip_flop_3_0_q_last;
    }
    if (!true) { 
        aux_register_27_d_flip_flop_3_0_q = true; // Preset
        aux_register_27_d_flip_flop_3_1_q = false;
    } else if (!true) { 
        aux_register_27_d_flip_flop_3_0_q = false; // Clear
        aux_register_27_d_flip_flop_3_1_q = true;
    }
    aux_register_27_d_flip_flop_3_0_q_inclk = aux_register_27_node_11;
    aux_register_27_d_flip_flop_3_0_q_last = aux_register_27_d_flip_flop_0_0_q;
    //End of D FlipFlop
    aux_register_27_node_4 = aux_register_27_d_flip_flop_13_0_q;
    aux_register_27_node_7 = aux_register_27_d_flip_flop_0_0_q;
    aux_register_27_node_8 = aux_register_27_d_flip_flop_3_0_q;
    aux_register_27_node_10 = aux_register_27_d_flip_flop_5_0_q;
    aux_register_27_led_0 = aux_register_27_node_4;
    aux_register_27_led_0_1 = aux_register_27_node_10;
    aux_register_27_led_0_2 = aux_register_27_node_7;
    aux_register_27_led_0_3 = aux_register_27_node_8;
    // End IC: REGISTER
    aux_node_28 = aux_node_26;
    aux_and_29 = aux_register_27_led_0_3 && aux_node_21;
    aux_and_30 = aux_register_27_led_0_2 && aux_node_24;
    aux_and_31 = aux_register_27_led_0_1 && aux_node_26;
    aux_and_32 = aux_register_27_led_0 && aux_node_28;

    // Write output data
    digitalWrite(led34_load_shift_0, aux_node_17);
    digitalWrite(led35_l1_0, aux_and_31);
    digitalWrite(led36_l3_0, aux_and_29);
    digitalWrite(led37_l2_0, aux_and_30);
    digitalWrite(led38_l0_0, aux_and_32);
}
