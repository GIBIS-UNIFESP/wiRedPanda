// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //
//
// Target Board: Arduino UNO R3/R4
// Pin Usage: 8/18 pins
// Standard Arduino board with 20 I/O pins (14 digital + 6 analog)
//


/* ========= Inputs ========== */
const int input_switch1_clock = A0;
const int input_switch2_load__shift = A1;
const int input_switch3_d0 = A2;
const int input_switch4_d1 = A3;
const int input_switch5_d2 = A4;
const int input_switch6_d3 = A5;

/* ========= Outputs ========== */
const int led17_0 = 2;
const int led19_0 = 3;

/* ====== Aux. Variables ====== */
bool input_switch1_clock_val = false;
bool input_switch2_load__shift_val = false;
bool input_switch3_d0_val = false;
bool input_switch4_d1_val = false;
bool input_switch5_d2_val = false;
bool input_switch6_d3_val = false;
bool aux_node_0 = false;
bool aux_node_1 = false;
bool aux_node_2 = false;
bool aux_node_3 = false;
bool aux_node_4 = false;
bool aux_node_5 = false;
bool aux_node_6 = false;
bool aux_node_7 = false;
bool aux_node_8 = false;
bool aux_node_9 = false;
bool aux_node_10 = false;
bool aux_node_11 = false;
bool aux_node_12 = false;
bool aux_node_13 = false;
bool aux_input_switch_14 = false;
bool aux_input_switch_15 = false;
bool aux_and_17 = false;
bool aux_not_19 = false;
bool aux_and_20 = false;
bool aux_or_21 = false;
bool aux_and_22 = false;
bool aux_d_flip_flop_23_0_q = false;
bool aux_d_flip_flop_23_0_q_inclk = false;
bool aux_d_flip_flop_23_0_q_last = false;
bool aux_d_flip_flop_23_1_q = true;
bool aux_d_flip_flop_23_1_q_inclk = false;
bool aux_d_flip_flop_23_1_q_last = false;
bool aux_and_24 = false;
bool aux_or_25 = false;
bool aux_d_flip_flop_26_0_q = false;
bool aux_d_flip_flop_26_0_q_inclk = false;
bool aux_d_flip_flop_26_0_q_last = false;
bool aux_d_flip_flop_26_1_q = true;
bool aux_d_flip_flop_26_1_q_inclk = false;
bool aux_d_flip_flop_26_1_q_last = false;
bool aux_and_27 = false;
bool aux_d_flip_flop_28_0_q = false;
bool aux_d_flip_flop_28_0_q_inclk = false;
bool aux_d_flip_flop_28_0_q_last = false;
bool aux_d_flip_flop_28_1_q = true;
bool aux_d_flip_flop_28_1_q_inclk = false;
bool aux_d_flip_flop_28_1_q_last = false;
bool aux_input_switch_29 = false;
bool aux_input_switch_30 = false;
bool aux_input_switch_31 = false;
bool aux_input_switch_32 = false;
bool aux_and_33 = false;
bool aux_d_flip_flop_34_0_q = false;
bool aux_d_flip_flop_34_0_q_inclk = false;
bool aux_d_flip_flop_34_0_q_last = false;
bool aux_d_flip_flop_34_1_q = true;
bool aux_d_flip_flop_34_1_q_inclk = false;
bool aux_d_flip_flop_34_1_q_last = false;
bool aux_and_35 = false;
bool aux_or_36 = false;
bool aux_node_37 = false;

void setup() {
    pinMode(input_switch1_clock, INPUT);
    pinMode(input_switch2_load__shift, INPUT);
    pinMode(input_switch3_d0, INPUT);
    pinMode(input_switch4_d1, INPUT);
    pinMode(input_switch5_d2, INPUT);
    pinMode(input_switch6_d3, INPUT);
    pinMode(led17_0, OUTPUT);
    pinMode(led19_0, OUTPUT);
}

void loop() {
    // Read input data
    input_switch1_clock_val = digitalRead(input_switch1_clock);
    input_switch2_load__shift_val = digitalRead(input_switch2_load__shift);
    input_switch3_d0_val = digitalRead(input_switch3_d0);
    input_switch4_d1_val = digitalRead(input_switch4_d1);
    input_switch5_d2_val = digitalRead(input_switch5_d2);
    input_switch6_d3_val = digitalRead(input_switch6_d3);

    // Update clocks

    // Update logic variables
    aux_node_0 = aux_input_switch_14;
    aux_node_1 = aux_input_switch_14;
    aux_node_2 = aux_input_switch_14;
    aux_node_3 = aux_input_switch_14;
    aux_node_4 = aux_input_switch_15;
    aux_node_5 = aux_node_4;
    aux_node_6 = aux_node_4;
    aux_node_7 = aux_node_4;
    aux_node_8 = aux_not_19;
    aux_node_9 = aux_not_19;
    aux_node_10 = aux_not_19;
    aux_node_11 = aux_not_19;
    aux_node_12 = aux_node_13;
    aux_node_13 = aux_node_37;
    aux_input_switch_14 = input_switch1_clock_val;
    aux_input_switch_15 = input_switch2_load__shift_val;
    aux_and_17 = aux_node_8 && aux_d_flip_flop_23_0_q;
    aux_not_19 = !aux_node_37;
    aux_and_20 = aux_node_9 && aux_d_flip_flop_26_0_q;
    aux_or_21 = aux_and_22 || aux_and_20;
    aux_and_22 = aux_node_5 && aux_input_switch_32;
    //D FlipFlop
    if (aux_node_3 && !aux_d_flip_flop_23_0_q_inclk) { 
        aux_d_flip_flop_23_0_q = aux_d_flip_flop_23_0_q_last;
        aux_d_flip_flop_23_1_q = !aux_d_flip_flop_23_0_q_last;
    }
    if (!true) { 
        aux_d_flip_flop_23_0_q = true; // Preset
        aux_d_flip_flop_23_1_q = false;
    } else if (!true) { 
        aux_d_flip_flop_23_0_q = false; // Clear
        aux_d_flip_flop_23_1_q = true;
    }
    aux_d_flip_flop_23_0_q_inclk = aux_node_3;
    aux_d_flip_flop_23_0_q_last = aux_or_21;
    //End of D FlipFlop
    aux_and_24 = aux_node_6 && aux_input_switch_31;
    aux_or_25 = aux_and_24 || aux_and_27;
    //D FlipFlop
    if (aux_node_2 && !aux_d_flip_flop_26_0_q_inclk) { 
        aux_d_flip_flop_26_0_q = aux_d_flip_flop_26_0_q_last;
        aux_d_flip_flop_26_1_q = !aux_d_flip_flop_26_0_q_last;
    }
    if (!true) { 
        aux_d_flip_flop_26_0_q = true; // Preset
        aux_d_flip_flop_26_1_q = false;
    } else if (!true) { 
        aux_d_flip_flop_26_0_q = false; // Clear
        aux_d_flip_flop_26_1_q = true;
    }
    aux_d_flip_flop_26_0_q_inclk = aux_node_2;
    aux_d_flip_flop_26_0_q_last = aux_or_25;
    //End of D FlipFlop
    aux_and_27 = aux_node_10 && aux_d_flip_flop_28_0_q;
    //D FlipFlop
    if (aux_node_1 && !aux_d_flip_flop_28_0_q_inclk) { 
        aux_d_flip_flop_28_0_q = aux_d_flip_flop_28_0_q_last;
        aux_d_flip_flop_28_1_q = !aux_d_flip_flop_28_0_q_last;
    }
    if (!true) { 
        aux_d_flip_flop_28_0_q = true; // Preset
        aux_d_flip_flop_28_1_q = false;
    } else if (!true) { 
        aux_d_flip_flop_28_0_q = false; // Clear
        aux_d_flip_flop_28_1_q = true;
    }
    aux_d_flip_flop_28_0_q_inclk = aux_node_1;
    aux_d_flip_flop_28_0_q_last = aux_or_36;
    //End of D FlipFlop
    aux_input_switch_29 = input_switch3_d0_val;
    aux_input_switch_30 = input_switch4_d1_val;
    aux_input_switch_31 = input_switch5_d2_val;
    aux_input_switch_32 = input_switch6_d3_val;
    aux_and_33 = aux_node_11 && aux_d_flip_flop_34_0_q;
    //D FlipFlop
    if (aux_node_0 && !aux_d_flip_flop_34_0_q_inclk) { 
        aux_d_flip_flop_34_0_q = aux_d_flip_flop_34_0_q_last;
        aux_d_flip_flop_34_1_q = !aux_d_flip_flop_34_0_q_last;
    }
    if (!true) { 
        aux_d_flip_flop_34_0_q = true; // Preset
        aux_d_flip_flop_34_1_q = false;
    } else if (!true) { 
        aux_d_flip_flop_34_0_q = false; // Clear
        aux_d_flip_flop_34_1_q = true;
    }
    aux_d_flip_flop_34_0_q_inclk = aux_node_0;
    aux_d_flip_flop_34_0_q_last = aux_input_switch_29;
    //End of D FlipFlop
    aux_and_35 = aux_node_7 && aux_input_switch_30;
    aux_or_36 = aux_and_35 || aux_and_33;
    aux_node_37 = aux_input_switch_15;

    // Write output data
    digitalWrite(led17_0, aux_node_12);
    digitalWrite(led19_0, aux_and_17);
}
