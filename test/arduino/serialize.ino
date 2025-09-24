// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //
//
// Target Board: Arduino UNO R3/R4
// Pin Usage: 8/18 pins
// Standard Arduino board with 20 I/O pins (14 digital + 6 analog)
//


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
bool input_switch1_load__shift_val = false;
bool input_switch2_clock_val = false;
bool input_switch3_d0_val = false;
bool input_switch4_d1_val = false;
bool input_switch5_d2_val = false;
bool input_switch6_d3_val = false;
bool aux_input_switch_0 = false;
bool aux_input_switch_1 = false;
bool aux_node_2 = false;
bool aux_node_3 = false;
bool aux_node_4 = false;
bool aux_not_5 = false;
bool aux_input_switch_6 = false;
bool aux_node_7 = false;
bool aux_node_8 = false;
bool aux_input_switch_9 = false;
bool aux_d_flip_flop_10_0_q = false;
bool aux_d_flip_flop_10_0_q_inclk = false;
bool aux_d_flip_flop_10_0_q_last = false;
bool aux_d_flip_flop_10_1_q = true;
bool aux_d_flip_flop_10_1_q_inclk = false;
bool aux_d_flip_flop_10_1_q_last = false;
bool aux_and_11 = false;
bool aux_and_12 = false;
bool aux_node_13 = false;
bool aux_or_14 = false;
bool aux_node_15 = false;
bool aux_node_16 = false;
bool aux_d_flip_flop_17_0_q = false;
bool aux_d_flip_flop_17_0_q_inclk = false;
bool aux_d_flip_flop_17_0_q_last = false;
bool aux_d_flip_flop_17_1_q = true;
bool aux_d_flip_flop_17_1_q_inclk = false;
bool aux_d_flip_flop_17_1_q_last = false;
bool aux_input_switch_18 = false;
bool aux_and_19 = false;
bool aux_and_20 = false;
bool aux_node_21 = false;
bool aux_or_22 = false;
bool aux_node_23 = false;
bool aux_node_24 = false;
bool aux_d_flip_flop_25_0_q = false;
bool aux_d_flip_flop_25_0_q_inclk = false;
bool aux_d_flip_flop_25_0_q_last = false;
bool aux_d_flip_flop_25_1_q = true;
bool aux_d_flip_flop_25_1_q_inclk = false;
bool aux_d_flip_flop_25_1_q_last = false;
bool aux_input_switch_26 = false;
bool aux_and_27 = false;
bool aux_and_28 = false;
bool aux_node_29 = false;
bool aux_or_30 = false;
bool aux_node_31 = false;
bool aux_node_32 = false;
bool aux_d_flip_flop_33_0_q = false;
bool aux_d_flip_flop_33_0_q_inclk = false;
bool aux_d_flip_flop_33_0_q_last = false;
bool aux_d_flip_flop_33_1_q = true;
bool aux_d_flip_flop_33_1_q_inclk = false;
bool aux_d_flip_flop_33_1_q_last = false;
bool aux_node_34 = false;
bool aux_and_35 = false;

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
    // Read input data
    input_switch1_load__shift_val = digitalRead(input_switch1_load__shift);
    input_switch2_clock_val = digitalRead(input_switch2_clock);
    input_switch3_d0_val = digitalRead(input_switch3_d0);
    input_switch4_d1_val = digitalRead(input_switch4_d1);
    input_switch5_d2_val = digitalRead(input_switch5_d2);
    input_switch6_d3_val = digitalRead(input_switch6_d3);

    // Update clocks

    // Update logic variables
    aux_input_switch_0 = input_switch1_load__shift_val;
    aux_input_switch_1 = input_switch2_clock_val;
    aux_node_2 = aux_input_switch_0;
    aux_node_3 = aux_input_switch_1;
    aux_node_4 = aux_input_switch_0;
    aux_not_5 = !aux_node_2;
    aux_input_switch_6 = input_switch3_d0_val;
    aux_node_7 = aux_node_4;
    aux_node_8 = aux_not_5;
    aux_input_switch_9 = input_switch4_d1_val;
    //D FlipFlop
    if (aux_node_3 && !aux_d_flip_flop_10_0_q_inclk) { 
        aux_d_flip_flop_10_0_q = aux_d_flip_flop_10_0_q_last;
        aux_d_flip_flop_10_1_q = !aux_d_flip_flop_10_0_q_last;
    }
    if (!true) { 
        aux_d_flip_flop_10_0_q = true; // Preset
        aux_d_flip_flop_10_1_q = false;
    } else if (!true) { 
        aux_d_flip_flop_10_0_q = false; // Clear
        aux_d_flip_flop_10_1_q = true;
    }
    aux_d_flip_flop_10_0_q_inclk = aux_node_3;
    aux_d_flip_flop_10_0_q_last = aux_input_switch_6;
    //End of D FlipFlop
    aux_and_11 = aux_node_8 && aux_d_flip_flop_10_0_q;
    aux_and_12 = aux_node_7 && aux_input_switch_9;
    aux_node_13 = aux_input_switch_1;
    aux_or_14 = aux_and_12 || aux_and_11;
    aux_node_15 = aux_node_4;
    aux_node_16 = aux_not_5;
    //D FlipFlop
    if (aux_node_13 && !aux_d_flip_flop_17_0_q_inclk) { 
        aux_d_flip_flop_17_0_q = aux_d_flip_flop_17_0_q_last;
        aux_d_flip_flop_17_1_q = !aux_d_flip_flop_17_0_q_last;
    }
    if (!true) { 
        aux_d_flip_flop_17_0_q = true; // Preset
        aux_d_flip_flop_17_1_q = false;
    } else if (!true) { 
        aux_d_flip_flop_17_0_q = false; // Clear
        aux_d_flip_flop_17_1_q = true;
    }
    aux_d_flip_flop_17_0_q_inclk = aux_node_13;
    aux_d_flip_flop_17_0_q_last = aux_or_14;
    //End of D FlipFlop
    aux_input_switch_18 = input_switch5_d2_val;
    aux_and_19 = aux_node_15 && aux_input_switch_18;
    aux_and_20 = aux_node_16 && aux_d_flip_flop_17_0_q;
    aux_node_21 = aux_input_switch_1;
    aux_or_22 = aux_and_19 || aux_and_20;
    aux_node_23 = aux_node_4;
    aux_node_24 = aux_not_5;
    //D FlipFlop
    if (aux_node_21 && !aux_d_flip_flop_25_0_q_inclk) { 
        aux_d_flip_flop_25_0_q = aux_d_flip_flop_25_0_q_last;
        aux_d_flip_flop_25_1_q = !aux_d_flip_flop_25_0_q_last;
    }
    if (!true) { 
        aux_d_flip_flop_25_0_q = true; // Preset
        aux_d_flip_flop_25_1_q = false;
    } else if (!true) { 
        aux_d_flip_flop_25_0_q = false; // Clear
        aux_d_flip_flop_25_1_q = true;
    }
    aux_d_flip_flop_25_0_q_inclk = aux_node_21;
    aux_d_flip_flop_25_0_q_last = aux_or_22;
    //End of D FlipFlop
    aux_input_switch_26 = input_switch6_d3_val;
    aux_and_27 = aux_node_24 && aux_d_flip_flop_25_0_q;
    aux_and_28 = aux_node_23 && aux_input_switch_26;
    aux_node_29 = aux_input_switch_1;
    aux_or_30 = aux_and_28 || aux_and_27;
    aux_node_31 = aux_not_5;
    aux_node_32 = aux_node_2;
    //D FlipFlop
    if (aux_node_29 && !aux_d_flip_flop_33_0_q_inclk) { 
        aux_d_flip_flop_33_0_q = aux_d_flip_flop_33_0_q_last;
        aux_d_flip_flop_33_1_q = !aux_d_flip_flop_33_0_q_last;
    }
    if (!true) { 
        aux_d_flip_flop_33_0_q = true; // Preset
        aux_d_flip_flop_33_1_q = false;
    } else if (!true) { 
        aux_d_flip_flop_33_0_q = false; // Clear
        aux_d_flip_flop_33_1_q = true;
    }
    aux_d_flip_flop_33_0_q_inclk = aux_node_29;
    aux_d_flip_flop_33_0_q_last = aux_or_30;
    //End of D FlipFlop
    aux_node_34 = aux_node_32;
    aux_and_35 = aux_node_31 && aux_d_flip_flop_33_0_q;

    // Write output data
    digitalWrite(led37_0, aux_node_34);
    digitalWrite(led38_0, aux_and_35);
}
