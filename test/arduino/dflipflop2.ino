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
const int led5_0 = A1;
const int led15_0 = A2;

/* ====== Aux. Variables ====== */
bool input_switch1_val = false;
bool aux_node_0 = false;
bool aux_node_1 = false;
bool aux_node_2 = false;
bool aux_node_3 = false;
bool aux_not_5 = false;
bool aux_nand_6 = false;
bool aux_nand_7 = false;
bool aux_d_flip_flop_8_0_q = false;
bool aux_d_flip_flop_8_0_q_inclk = false;
bool aux_d_flip_flop_8_0_q_last = false;
bool aux_d_flip_flop_8_1_q = true;
bool aux_d_flip_flop_8_1_q_inclk = false;
bool aux_d_flip_flop_8_1_q_last = false;
bool aux_nand_9 = false;
bool aux_nand_10 = false;
bool aux_input_switch_11 = false;
bool aux_nand_12 = false;
bool aux_not_13 = false;
bool aux_nand_15 = false;
bool aux_clock_16 = false;
unsigned long aux_clock_16_lastTime = 0;
const unsigned long aux_clock_16_interval = 1000;
bool aux_nand_17 = false;
bool aux_nand_18 = false;
bool aux_not_19 = false;
bool aux_node_20 = false;
bool aux_node_21 = false;
bool aux_node_22 = false;
bool aux_node_23 = false;

void setup() {
    pinMode(input_switch1, INPUT);
    pinMode(led5_0, OUTPUT);
    pinMode(led15_0, OUTPUT);
}

void loop() {
    // Read input data
    input_switch1_val = digitalRead(input_switch1);

    // Update clocks
    unsigned long now = millis();
    if (now - aux_clock_16_lastTime >= aux_clock_16_interval) {
        aux_clock_16_lastTime = now;
        aux_clock_16 = !aux_clock_16;
    }

    // Update logic variables
    aux_node_0 = aux_node_20;
    aux_node_1 = aux_node_23;
    aux_node_2 = aux_not_19;
    aux_node_3 = aux_node_21;
    aux_not_5 = !aux_input_switch_11;
    aux_nand_6 = !(aux_nand_7 && aux_nand_12);
    aux_nand_7 = !(aux_nand_9 && aux_node_1);
    //D FlipFlop
    if (aux_node_3 && !aux_d_flip_flop_8_0_q_inclk) { 
        aux_d_flip_flop_8_0_q = aux_d_flip_flop_8_0_q_last;
        aux_d_flip_flop_8_1_q = !aux_d_flip_flop_8_0_q_last;
    }
    if (!true) { 
        aux_d_flip_flop_8_0_q = true; // Preset
        aux_d_flip_flop_8_1_q = false;
    } else if (!true) { 
        aux_d_flip_flop_8_0_q = false; // Clear
        aux_d_flip_flop_8_1_q = true;
    }
    aux_d_flip_flop_8_0_q_inclk = aux_node_3;
    aux_d_flip_flop_8_0_q_last = aux_node_22;
    //End of D FlipFlop
    aux_nand_9 = !(aux_nand_18 && aux_nand_15);
    aux_nand_10 = !(aux_node_23 && aux_nand_15);
    aux_input_switch_11 = input_switch1_val;
    aux_nand_12 = !(aux_nand_6 && aux_nand_10);
    aux_not_13 = !aux_clock_16;
    aux_nand_15 = !(aux_nand_9 && aux_nand_17);
    aux_nand_17 = !(aux_node_20 && aux_not_5);
    aux_nand_18 = !(aux_node_22 && aux_not_13);
    aux_not_19 = !aux_node_0;
    aux_node_20 = aux_not_13;
    aux_node_21 = aux_clock_16;
    aux_node_22 = aux_input_switch_11;
    aux_node_23 = aux_node_2;

    // Write output data
    digitalWrite(led5_0, aux_d_flip_flop_8_0_q);
    digitalWrite(led15_0, aux_nand_6);
}
