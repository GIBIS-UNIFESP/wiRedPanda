// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //
//
// Target Board: Arduino UNO R3/R4
// Pin Usage: 4/18 pins
// Standard Arduino board with 20 I/O pins (14 digital + 6 analog)
//


/* ========= Inputs ========== */

/* ========= Outputs ========== */
const int led5_0 = A0;
const int led8_0 = A1;
const int led9_0 = A2;
const int led11_0 = A3;

/* ====== Aux. Variables ====== */
bool aux_d_flip_flop_0_0_q = false;
bool aux_d_flip_flop_0_0_q_inclk = false;
bool aux_d_flip_flop_0_0_q_last = false;
bool aux_d_flip_flop_0_1_q = true;
bool aux_d_flip_flop_0_1_q_inclk = false;
bool aux_d_flip_flop_0_1_q_last = false;
bool aux_node_1 = false;
bool aux_clock_2 = false;
unsigned long aux_clock_2_lastTime = 0;
const unsigned long aux_clock_2_interval = 500;
bool aux_d_flip_flop_3_0_q = false;
bool aux_d_flip_flop_3_0_q_inclk = false;
bool aux_d_flip_flop_3_0_q_last = false;
bool aux_d_flip_flop_3_1_q = true;
bool aux_d_flip_flop_3_1_q_inclk = false;
bool aux_d_flip_flop_3_1_q_last = false;
bool aux_d_flip_flop_5_0_q = false;
bool aux_d_flip_flop_5_0_q_inclk = false;
bool aux_d_flip_flop_5_0_q_last = false;
bool aux_d_flip_flop_5_1_q = true;
bool aux_d_flip_flop_5_1_q_inclk = false;
bool aux_d_flip_flop_5_1_q_last = false;
bool aux_clock_6 = false;
unsigned long aux_clock_6_lastTime = 0;
const unsigned long aux_clock_6_interval = 125;
bool aux_node_9 = false;
bool aux_node_11 = false;
bool aux_node_12 = false;
bool aux_d_flip_flop_13_0_q = false;
bool aux_d_flip_flop_13_0_q_inclk = false;
bool aux_d_flip_flop_13_0_q_last = false;
bool aux_d_flip_flop_13_1_q = true;
bool aux_d_flip_flop_13_1_q_inclk = false;
bool aux_d_flip_flop_13_1_q_last = false;

void setup() {
    pinMode(led5_0, OUTPUT);
    pinMode(led8_0, OUTPUT);
    pinMode(led9_0, OUTPUT);
    pinMode(led11_0, OUTPUT);
}

void loop() {
    // Read input data

    // Update clocks
    unsigned long now = millis();
    if (now - aux_clock_2_lastTime >= aux_clock_2_interval) {
        aux_clock_2_lastTime = now;
        aux_clock_2 = !aux_clock_2;
    }
    if (now - aux_clock_6_lastTime >= aux_clock_6_interval) {
        aux_clock_6_lastTime = now;
        aux_clock_6 = !aux_clock_6;
    }

    // Update logic variables
    //D FlipFlop
    if (aux_node_1 && !aux_d_flip_flop_0_0_q_inclk) { 
        aux_d_flip_flop_0_0_q = aux_d_flip_flop_0_0_q_last;
        aux_d_flip_flop_0_1_q = !aux_d_flip_flop_0_0_q_last;
    }
    if (!true) { 
        aux_d_flip_flop_0_0_q = true; // Preset
        aux_d_flip_flop_0_1_q = false;
    } else if (!true) { 
        aux_d_flip_flop_0_0_q = false; // Clear
        aux_d_flip_flop_0_1_q = true;
    }
    aux_d_flip_flop_0_0_q_inclk = aux_node_1;
    aux_d_flip_flop_0_0_q_last = aux_d_flip_flop_5_0_q;
    //End of D FlipFlop
    aux_node_1 = aux_node_9;
    //D FlipFlop
    if (aux_node_11 && !aux_d_flip_flop_3_0_q_inclk) { 
        aux_d_flip_flop_3_0_q = aux_d_flip_flop_3_0_q_last;
        aux_d_flip_flop_3_1_q = !aux_d_flip_flop_3_0_q_last;
    }
    if (!true) { 
        aux_d_flip_flop_3_0_q = true; // Preset
        aux_d_flip_flop_3_1_q = false;
    } else if (!true) { 
        aux_d_flip_flop_3_0_q = false; // Clear
        aux_d_flip_flop_3_1_q = true;
    }
    aux_d_flip_flop_3_0_q_inclk = aux_node_11;
    aux_d_flip_flop_3_0_q_last = aux_d_flip_flop_0_0_q;
    //End of D FlipFlop
    //D FlipFlop
    if (aux_node_9 && !aux_d_flip_flop_5_0_q_inclk) { 
        aux_d_flip_flop_5_0_q = aux_d_flip_flop_5_0_q_last;
        aux_d_flip_flop_5_1_q = !aux_d_flip_flop_5_0_q_last;
    }
    if (!true) { 
        aux_d_flip_flop_5_0_q = true; // Preset
        aux_d_flip_flop_5_1_q = false;
    } else if (!true) { 
        aux_d_flip_flop_5_0_q = false; // Clear
        aux_d_flip_flop_5_1_q = true;
    }
    aux_d_flip_flop_5_0_q_inclk = aux_node_9;
    aux_d_flip_flop_5_0_q_last = aux_d_flip_flop_13_0_q;
    //End of D FlipFlop
    aux_node_9 = aux_node_12;
    aux_node_11 = aux_node_1;
    aux_node_12 = aux_clock_6;
    //D FlipFlop
    if (aux_node_12 && !aux_d_flip_flop_13_0_q_inclk) { 
        aux_d_flip_flop_13_0_q = aux_d_flip_flop_13_0_q_last;
        aux_d_flip_flop_13_1_q = !aux_d_flip_flop_13_0_q_last;
    }
    if (!true) { 
        aux_d_flip_flop_13_0_q = true; // Preset
        aux_d_flip_flop_13_1_q = false;
    } else if (!true) { 
        aux_d_flip_flop_13_0_q = false; // Clear
        aux_d_flip_flop_13_1_q = true;
    }
    aux_d_flip_flop_13_0_q_inclk = aux_node_12;
    aux_d_flip_flop_13_0_q_last = aux_clock_2;
    //End of D FlipFlop

    // Write output data
    digitalWrite(led5_0, aux_d_flip_flop_13_0_q);
    digitalWrite(led8_0, aux_d_flip_flop_0_0_q);
    digitalWrite(led9_0, aux_d_flip_flop_3_0_q);
    digitalWrite(led11_0, aux_d_flip_flop_5_0_q);
}
