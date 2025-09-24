// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //
//
// Target Board: Arduino UNO R3/R4
// Pin Usage: 6/18 pins
// Standard Arduino board with 20 I/O pins (14 digital + 6 analog)
//


/* ========= Inputs ========== */
const int input_switch1__preset = A0;
const int input_switch2__clear = A1;
const int input_switch3_j = A2;
const int input_switch4_k = A3;

/* ========= Outputs ========== */
const int led19_q_0 = A4;
const int led20_q_0 = A5;

/* ====== Aux. Variables ====== */
bool input_switch1__preset_val = false;
bool input_switch2__clear_val = false;
bool input_switch3_j_val = false;
bool input_switch4_k_val = false;
// IC: DFLIPFLOP
bool aux_dflipflop_0_led_0 = false;
bool aux_dflipflop_0_led_0_1 = false;
bool aux_dflipflop_0_node_0 = false;
bool aux_dflipflop_0_node_1 = false;
bool aux_dflipflop_0_node_2 = false;
bool aux_dflipflop_0_node_3 = false;
bool aux_dflipflop_0_nand_4 = false;
bool aux_dflipflop_0_node_5 = false;
bool aux_dflipflop_0_nand_6 = false;
bool aux_dflipflop_0_not_7 = false;
bool aux_dflipflop_0_nand_8 = false;
bool aux_dflipflop_0_nand_9 = false;
bool aux_dflipflop_0_nand_10 = false;
bool aux_dflipflop_0_nand_11 = false;
bool aux_dflipflop_0_node_12 = false;
bool aux_dflipflop_0_not_13 = false;
bool aux_dflipflop_0_nand_14 = false;
bool aux_dflipflop_0_nand_15 = false;
bool aux_dflipflop_0_node_16 = false;
bool aux_dflipflop_0_node_17 = false;
bool aux_dflipflop_0_node_18 = false;
bool aux_dflipflop_0_not_19 = false;
bool aux_dflipflop_0_node_20 = false;
bool aux_dflipflop_0_node_21 = false;
bool aux_dflipflop_0_node_22 = false;
bool aux_dflipflop_0_node_23 = false;
bool aux_dflipflop_0_node_24 = false;
bool aux_ic_input_dflipflop_0 = false;
bool aux_ic_input_dflipflop_1 = false;
bool aux_ic_input_dflipflop_2 = false;
bool aux_ic_input_dflipflop_3 = false;
// End IC: DFLIPFLOP
bool aux_node_1 = false;
bool aux_node_2 = false;
bool aux_node_3 = false;
bool aux_node_4 = false;
bool aux_node_5 = false;
bool aux_node_6 = false;
bool aux_node_7 = false;
bool aux_clock_8 = false;
unsigned long aux_clock_8_lastTime = 0;
const unsigned long aux_clock_8_interval = 1000;
bool aux_not_9 = false;
bool aux_and_10 = false;
bool aux_input_switch_11 = false;
bool aux_or_12 = false;
bool aux_input_switch_13 = false;
bool aux_input_switch_14 = false;
bool aux_input_switch_15 = false;
bool aux_node_16 = false;
bool aux_and_17 = false;

void setup() {
    pinMode(input_switch1__preset, INPUT);
    pinMode(input_switch2__clear, INPUT);
    pinMode(input_switch3_j, INPUT);
    pinMode(input_switch4_k, INPUT);
    pinMode(led19_q_0, OUTPUT);
    pinMode(led20_q_0, OUTPUT);
}

void loop() {
    // Read input data
    input_switch1__preset_val = digitalRead(input_switch1__preset);
    input_switch2__clear_val = digitalRead(input_switch2__clear);
    input_switch3_j_val = digitalRead(input_switch3_j);
    input_switch4_k_val = digitalRead(input_switch4_k);

    // Update clocks
    unsigned long now = millis();
    if (now - aux_clock_8_lastTime >= aux_clock_8_interval) {
        aux_clock_8_lastTime = now;
        aux_clock_8 = !aux_clock_8;
    }

    // Update logic variables
    // IC: DFLIPFLOP
    aux_ic_input_dflipflop_0 = aux_node_1;
    aux_ic_input_dflipflop_1 = aux_or_12;
    aux_ic_input_dflipflop_2 = aux_node_3;
    aux_ic_input_dflipflop_3 = aux_node_2;
    aux_dflipflop_0_node_18 = aux_ic_input_dflipflop_2;
    aux_dflipflop_0_not_19 = !aux_dflipflop_0_node_18;
    aux_dflipflop_0_not_7 = !aux_dflipflop_0_not_19;
    aux_dflipflop_0_node_20 = aux_dflipflop_0_not_19;
    aux_dflipflop_0_node_1 = aux_dflipflop_0_not_7;
    aux_dflipflop_0_node_2 = aux_dflipflop_0_node_20;
    aux_dflipflop_0_node_3 = aux_ic_input_dflipflop_1;
    aux_dflipflop_0_node_16 = aux_ic_input_dflipflop_0;
    aux_dflipflop_0_node_17 = aux_ic_input_dflipflop_3;
    aux_dflipflop_0_not_13 = !aux_dflipflop_0_node_3;
    aux_dflipflop_0_node_21 = aux_dflipflop_0_node_1;
    aux_dflipflop_0_nand_4 = !(aux_dflipflop_0_node_3 && aux_dflipflop_0_node_2);
    aux_dflipflop_0_nand_9 = !(aux_dflipflop_0_node_16 && aux_dflipflop_0_nand_4 && aux_dflipflop_0_nand_8);
    aux_dflipflop_0_node_22 = aux_dflipflop_0_node_17;
    aux_dflipflop_0_node_0 = aux_dflipflop_0_node_21;
    aux_dflipflop_0_nand_15 = !(aux_dflipflop_0_node_20 && aux_dflipflop_0_not_13);
    aux_dflipflop_0_node_24 = aux_dflipflop_0_node_16;
    aux_dflipflop_0_nand_10 = !(aux_dflipflop_0_nand_9 && aux_dflipflop_0_node_0);
    aux_dflipflop_0_nand_8 = !(aux_dflipflop_0_nand_9 && aux_dflipflop_0_nand_15 && aux_dflipflop_0_node_22);
    aux_dflipflop_0_nand_14 = !(aux_dflipflop_0_node_21 && aux_dflipflop_0_nand_8);
    aux_dflipflop_0_nand_6 = !(aux_dflipflop_0_node_24 && aux_dflipflop_0_nand_10 && aux_dflipflop_0_nand_11);
    aux_dflipflop_0_node_23 = aux_dflipflop_0_node_17;
    aux_dflipflop_0_nand_11 = !(aux_dflipflop_0_nand_6 && aux_dflipflop_0_nand_14 && aux_dflipflop_0_node_23);
    aux_dflipflop_0_node_12 = aux_dflipflop_0_nand_11;
    aux_dflipflop_0_node_5 = aux_dflipflop_0_nand_6;
    aux_dflipflop_0_led_0 = aux_dflipflop_0_node_5;
    aux_dflipflop_0_led_0_1 = aux_dflipflop_0_node_12;
    // End IC: DFLIPFLOP
    aux_node_1 = aux_input_switch_11;
    aux_node_2 = aux_input_switch_13;
    aux_node_3 = aux_clock_8;
    aux_node_4 = aux_node_7;
    aux_node_5 = aux_node_6;
    aux_node_6 = aux_dflipflop_0_led_0_1;
    aux_node_7 = aux_dflipflop_0_led_0;
    aux_not_9 = !aux_input_switch_14;
    aux_and_10 = aux_node_16 && aux_node_5;
    aux_input_switch_11 = input_switch1__preset_val;
    aux_or_12 = aux_and_17 || aux_and_10;
    aux_input_switch_13 = input_switch2__clear_val;
    aux_input_switch_14 = input_switch3_j_val;
    aux_input_switch_15 = input_switch4_k_val;
    aux_node_16 = aux_input_switch_15;
    aux_and_17 = aux_node_4 && aux_not_9;

    // Write output data
    digitalWrite(led19_q_0, aux_dflipflop_0_led_0);
    digitalWrite(led20_q_0, aux_dflipflop_0_led_0_1);
}
