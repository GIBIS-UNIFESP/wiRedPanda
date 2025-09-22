// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //
//
// Target Board: Arduino UNO R3/R4
// Pin Usage: 3/18 pins
// Standard Arduino board with 20 I/O pins (14 digital + 6 analog)
//


/* ========= Inputs ========== */
const int input_switch1 = A0;

/* ========= Outputs ========== */
const int led22_0 = A1;
const int led24_0 = A2;

/* ====== Aux. Variables ====== */
bool input_switch1_val = false;
bool aux_clock_0 = false;
unsigned long aux_clock_0_lastTime = 0;
const unsigned long aux_clock_0_interval = 1000;
bool aux_not_1 = false;
bool aux_node_2 = false;
bool aux_node_3 = false;
bool aux_input_switch_4 = false;
bool aux_not_5 = false;
bool aux_not_6 = false;
bool aux_node_7 = false;
bool aux_node_8 = false;
bool aux_nand_9 = false;
bool aux_node_10 = false;
bool aux_nand_11 = false;
bool aux_nand_12 = false;
bool aux_node_13 = false;
bool aux_node_14 = false;
bool aux_nand_15 = false;
bool aux_nand_16 = false;
bool aux_node_17 = false;
bool aux_nand_18 = false;
bool aux_d_flip_flop_19_0_q = false;
bool aux_d_flip_flop_19_0_q_inclk = false;
bool aux_d_flip_flop_19_0_q_last = false;
bool aux_d_flip_flop_19_1_q = true;
bool aux_d_flip_flop_19_1_q_inclk = false;
bool aux_d_flip_flop_19_1_q_last = false;
bool aux_nand_20 = false;
bool aux_nand_22 = false;

void setup() {
    pinMode(input_switch1, INPUT);
    pinMode(led22_0, OUTPUT);
    pinMode(led24_0, OUTPUT);
}

void loop() {
    // Read input data
    input_switch1_val = digitalRead(input_switch1);

    // Update clocks
    unsigned long now = millis();
    if (now - aux_clock_0_lastTime >= aux_clock_0_interval) {
        aux_clock_0_lastTime = now;
        aux_clock_0 = !aux_clock_0;
    }

    // Update logic variables
    aux_not_1 = !aux_clock_0;
    aux_node_2 = aux_not_1;
    aux_node_3 = aux_node_2;
    aux_input_switch_4 = input_switch1_val;
    aux_not_5 = !aux_input_switch_4;
    aux_not_6 = !aux_node_3;
    aux_node_7 = aux_not_6;
    aux_node_8 = aux_input_switch_4;
    aux_nand_9 = !(aux_node_2 && aux_not_5);
    aux_node_10 = aux_node_7;
    aux_nand_11 = !(aux_node_8 && aux_not_1);
    aux_nand_12 = !(aux_nand_15 && aux_nand_9);
    aux_node_13 = aux_node_10;
    aux_node_14 = aux_clock_0;
    aux_nand_15 = !(aux_nand_11 && aux_nand_12);
    aux_nand_16 = !(aux_nand_15 && aux_node_13);
    aux_node_17 = aux_node_14;
    aux_nand_18 = !(aux_node_10 && aux_nand_12);
    //D FlipFlop
    if (aux_node_17 && !aux_d_flip_flop_19_0_q_inclk) { 
        aux_d_flip_flop_19_0_q = aux_d_flip_flop_19_0_q_last;
        aux_d_flip_flop_19_1_q = !aux_d_flip_flop_19_0_q_last;
    }
    if (!true) { 
        aux_d_flip_flop_19_0_q = true; // Preset
        aux_d_flip_flop_19_1_q = false;
    } else if (!true) { 
        aux_d_flip_flop_19_0_q = false; // Clear
        aux_d_flip_flop_19_1_q = true;
    }
    aux_d_flip_flop_19_0_q_inclk = aux_node_17;
    aux_d_flip_flop_19_0_q_last = aux_node_8;
    //End of D FlipFlop
    aux_nand_20 = !(aux_nand_16 && aux_nand_22);
    aux_nand_22 = !(aux_nand_20 && aux_nand_18);

    // Write output data
    digitalWrite(led22_0, aux_nand_20);
    digitalWrite(led24_0, aux_d_flip_flop_19_0_q);
}
