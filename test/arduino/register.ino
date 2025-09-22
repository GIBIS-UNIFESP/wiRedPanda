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
const int led11_0 = A0;
const int led12_0 = A1;
const int led13_0 = A2;
const int led14_0 = A3;

/* ====== Aux. Variables ====== */
bool aux_clock_0 = false;
unsigned long aux_clock_0_lastTime = 0;
const unsigned long aux_clock_0_interval = 125;
bool aux_clock_1 = false;
unsigned long aux_clock_1_lastTime = 0;
const unsigned long aux_clock_1_interval = 500;
bool aux_node_2 = false;
bool aux_node_3 = false;
bool aux_d_flip_flop_4_0_q = false;
bool aux_d_flip_flop_4_0_q_inclk = false;
bool aux_d_flip_flop_4_0_q_last = false;
bool aux_d_flip_flop_4_1_q = true;
bool aux_d_flip_flop_4_1_q_inclk = false;
bool aux_d_flip_flop_4_1_q_last = false;
bool aux_node_5 = false;
bool aux_d_flip_flop_6_0_q = false;
bool aux_d_flip_flop_6_0_q_inclk = false;
bool aux_d_flip_flop_6_0_q_last = false;
bool aux_d_flip_flop_6_1_q = true;
bool aux_d_flip_flop_6_1_q_inclk = false;
bool aux_d_flip_flop_6_1_q_last = false;
bool aux_d_flip_flop_7_0_q = false;
bool aux_d_flip_flop_7_0_q_inclk = false;
bool aux_d_flip_flop_7_0_q_last = false;
bool aux_d_flip_flop_7_1_q = true;
bool aux_d_flip_flop_7_1_q_inclk = false;
bool aux_d_flip_flop_7_1_q_last = false;
bool aux_node_8 = false;
bool aux_d_flip_flop_9_0_q = false;
bool aux_d_flip_flop_9_0_q_inclk = false;
bool aux_d_flip_flop_9_0_q_last = false;
bool aux_d_flip_flop_9_1_q = true;
bool aux_d_flip_flop_9_1_q_inclk = false;
bool aux_d_flip_flop_9_1_q_last = false;

void setup() {
    pinMode(led11_0, OUTPUT);
    pinMode(led12_0, OUTPUT);
    pinMode(led13_0, OUTPUT);
    pinMode(led14_0, OUTPUT);
}

void loop() {
    // Read input data

    // Update clocks
    unsigned long now = millis();
    if (now - aux_clock_0_lastTime >= aux_clock_0_interval) {
        aux_clock_0_lastTime = now;
        aux_clock_0 = !aux_clock_0;
    }
    if (now - aux_clock_1_lastTime >= aux_clock_1_interval) {
        aux_clock_1_lastTime = now;
        aux_clock_1 = !aux_clock_1;
    }

    // Update logic variables
    aux_node_2 = aux_clock_0;
    aux_node_3 = aux_node_2;
    //D FlipFlop
    if (aux_node_2 && !aux_d_flip_flop_4_0_q_inclk) { 
        aux_d_flip_flop_4_0_q = aux_d_flip_flop_4_0_q_last;
        aux_d_flip_flop_4_1_q = !aux_d_flip_flop_4_0_q_last;
    }
    if (!true) { 
        aux_d_flip_flop_4_0_q = true; // Preset
        aux_d_flip_flop_4_1_q = false;
    } else if (!true) { 
        aux_d_flip_flop_4_0_q = false; // Clear
        aux_d_flip_flop_4_1_q = true;
    }
    aux_d_flip_flop_4_0_q_inclk = aux_node_2;
    aux_d_flip_flop_4_0_q_last = aux_clock_1;
    //End of D FlipFlop
    aux_node_5 = aux_node_3;
    //D FlipFlop
    if (aux_node_3 && !aux_d_flip_flop_6_0_q_inclk) { 
        aux_d_flip_flop_6_0_q = aux_d_flip_flop_6_0_q_last;
        aux_d_flip_flop_6_1_q = !aux_d_flip_flop_6_0_q_last;
    }
    if (!true) { 
        aux_d_flip_flop_6_0_q = true; // Preset
        aux_d_flip_flop_6_1_q = false;
    } else if (!true) { 
        aux_d_flip_flop_6_0_q = false; // Clear
        aux_d_flip_flop_6_1_q = true;
    }
    aux_d_flip_flop_6_0_q_inclk = aux_node_3;
    aux_d_flip_flop_6_0_q_last = aux_d_flip_flop_4_0_q;
    //End of D FlipFlop
    //D FlipFlop
    if (aux_node_5 && !aux_d_flip_flop_7_0_q_inclk) { 
        aux_d_flip_flop_7_0_q = aux_d_flip_flop_7_0_q_last;
        aux_d_flip_flop_7_1_q = !aux_d_flip_flop_7_0_q_last;
    }
    if (!true) { 
        aux_d_flip_flop_7_0_q = true; // Preset
        aux_d_flip_flop_7_1_q = false;
    } else if (!true) { 
        aux_d_flip_flop_7_0_q = false; // Clear
        aux_d_flip_flop_7_1_q = true;
    }
    aux_d_flip_flop_7_0_q_inclk = aux_node_5;
    aux_d_flip_flop_7_0_q_last = aux_d_flip_flop_6_0_q;
    //End of D FlipFlop
    aux_node_8 = aux_node_5;
    //D FlipFlop
    if (aux_node_8 && !aux_d_flip_flop_9_0_q_inclk) { 
        aux_d_flip_flop_9_0_q = aux_d_flip_flop_9_0_q_last;
        aux_d_flip_flop_9_1_q = !aux_d_flip_flop_9_0_q_last;
    }
    if (!true) { 
        aux_d_flip_flop_9_0_q = true; // Preset
        aux_d_flip_flop_9_1_q = false;
    } else if (!true) { 
        aux_d_flip_flop_9_0_q = false; // Clear
        aux_d_flip_flop_9_1_q = true;
    }
    aux_d_flip_flop_9_0_q_inclk = aux_node_8;
    aux_d_flip_flop_9_0_q_last = aux_d_flip_flop_7_0_q;
    //End of D FlipFlop

    // Write output data
    digitalWrite(led11_0, aux_d_flip_flop_4_0_q);
    digitalWrite(led12_0, aux_d_flip_flop_7_0_q);
    digitalWrite(led13_0, aux_d_flip_flop_9_0_q);
    digitalWrite(led14_0, aux_d_flip_flop_6_0_q);
}
