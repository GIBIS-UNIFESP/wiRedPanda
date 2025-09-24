// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //
//
// Target Board: Arduino UNO R3/R4
// Pin Usage: 3/18 pins
// Standard Arduino board with 20 I/O pins (14 digital + 6 analog)
//


/* ========= Inputs ========== */

/* ========= Outputs ========== */
const int led5_0 = A0;
const int led6_0 = A1;
const int led7_0 = A2;

/* ====== Aux. Variables ====== */
bool aux_clock_0 = false;
unsigned long aux_clock_0_lastTime = 0;
const unsigned long aux_clock_0_interval = 250;
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

void setup() {
    pinMode(led5_0, OUTPUT);
    pinMode(led6_0, OUTPUT);
    pinMode(led7_0, OUTPUT);
}

void loop() {
    // Read input data

    // Update clocks
    unsigned long now = millis();
    if (now - aux_clock_0_lastTime >= aux_clock_0_interval) {
        aux_clock_0_lastTime = now;
        aux_clock_0 = !aux_clock_0;
    }

    // Update logic variables
    //JK FlipFlop
    if (aux_clock_0 && !aux_jk_flip_flop_1_0_q_inclk) { 
        // Toggle mode (J=1, K=1)
        bool aux = aux_jk_flip_flop_1_0_q;
        aux_jk_flip_flop_1_0_q = aux_jk_flip_flop_1_1_q;
        aux_jk_flip_flop_1_1_q = aux;
    }
    aux_jk_flip_flop_1_0_q_inclk = aux_clock_0;
    //End of JK FlipFlop
    //JK FlipFlop
    if (aux_jk_flip_flop_1_0_q && !aux_jk_flip_flop_2_0_q_inclk) { 
        // Toggle mode (J=1, K=1)
        bool aux = aux_jk_flip_flop_2_0_q;
        aux_jk_flip_flop_2_0_q = aux_jk_flip_flop_2_1_q;
        aux_jk_flip_flop_2_1_q = aux;
    }
    aux_jk_flip_flop_2_0_q_inclk = aux_jk_flip_flop_1_0_q;
    //End of JK FlipFlop
    //JK FlipFlop
    if (aux_jk_flip_flop_2_0_q && !aux_jk_flip_flop_3_0_q_inclk) { 
        // Toggle mode (J=1, K=1)
        bool aux = aux_jk_flip_flop_3_0_q;
        aux_jk_flip_flop_3_0_q = aux_jk_flip_flop_3_1_q;
        aux_jk_flip_flop_3_1_q = aux;
    }
    aux_jk_flip_flop_3_0_q_inclk = aux_jk_flip_flop_2_0_q;
    //End of JK FlipFlop

    // Write output data
    digitalWrite(led5_0, aux_jk_flip_flop_3_1_q);
    digitalWrite(led6_0, aux_jk_flip_flop_2_1_q);
    digitalWrite(led7_0, aux_jk_flip_flop_1_1_q);
}
