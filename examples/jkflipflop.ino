// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


/* ========= Inputs ========== */
const int input_switch1__clear = A0;
const int input_switch2__preset = A1;
const int input_switch3_j = A2;
const int input_switch4_k = A3;

/* ========= Outputs ========== */
const int led19_q_0 = A4;
const int led20_q_0 = A5;

/* ====== Aux. Variables ====== */
bool input_switch1__clear_val = false;
bool input_switch2__preset_val = false;
bool input_switch3_j_val = false;
bool input_switch4_k_val = false;
bool aux_input_switch_0 = false;
bool aux_input_switch_1 = false;
bool aux_clock_2 = false;
unsigned long aux_clock_2_lastTime = 0;
const unsigned long aux_clock_2_interval = 1000;
bool aux_node_3 = false;
bool aux_node_4 = false;
bool aux_node_5 = false;
// IC: DFLIPFLOP
bool aux_ic_6_led_0 = false;
bool aux_ic_6_led_0_1 = false;
bool aux_ic_6_node_0 = false;
bool aux_ic_6_node_1 = false;
bool aux_ic_6_node_2 = false;
bool aux_ic_6_node_3 = false;
bool aux_ic_6_nand_4 = false;
bool aux_ic_6_node_5 = false;
bool aux_ic_6_nand_6 = false;
bool aux_ic_6_not_7 = false;
bool aux_ic_6_nand_8 = false;
bool aux_ic_6_nand_9 = false;
bool aux_ic_6_nand_10 = false;
bool aux_ic_6_nand_11 = false;
bool aux_ic_6_node_12 = false;
bool aux_ic_6_not_13 = false;
bool aux_ic_6_nand_14 = false;
bool aux_ic_6_nand_15 = false;
bool aux_ic_6_node_16 = false;
bool aux_ic_6_node_17 = false;
bool aux_ic_6_node_18 = false;
bool aux_ic_6_not_19 = false;
bool aux_ic_6_node_20 = false;
bool aux_ic_6_node_21 = false;
bool aux_ic_6_node_22 = false;
bool aux_ic_6_node_23 = false;
bool aux_ic_6_node_24 = false;
bool aux_ic_input_ic_0 = false;
bool aux_ic_input_ic_1 = false;
bool aux_ic_input_ic_2 = false;
bool aux_ic_input_ic_3 = false;
// End IC: DFLIPFLOP
bool aux_node_7 = false;
bool aux_node_8 = false;
bool aux_input_switch_9 = false;
bool aux_input_switch_10 = false;
bool aux_node_11 = false;
bool aux_node_12 = false;
bool aux_not_13 = false;
bool aux_node_14 = false;
bool aux_and_15 = false;
bool aux_and_16 = false;
bool aux_or_17 = false;

void setup() {
    pinMode(input_switch1__clear, INPUT);
    pinMode(input_switch2__preset, INPUT);
    pinMode(input_switch3_j, INPUT);
    pinMode(input_switch4_k, INPUT);
    pinMode(led19_q_0, OUTPUT);
    pinMode(led20_q_0, OUTPUT);
}

void loop() {
    // Read input data
    input_switch1__clear_val = digitalRead(input_switch1__clear);
    input_switch2__preset_val = digitalRead(input_switch2__preset);
    input_switch3_j_val = digitalRead(input_switch3_j);
    input_switch4_k_val = digitalRead(input_switch4_k);

    // Update clocks
    unsigned long now = millis();
    if (now - aux_clock_2_lastTime >= aux_clock_2_interval) {
        aux_clock_2_lastTime = now;
        aux_clock_2 = !aux_clock_2;
    }

    // Update logic variables
    aux_input_switch_0 = input_switch1__clear_val;
    aux_input_switch_1 = input_switch2__preset_val;
    aux_node_3 = aux_input_switch_1;
    aux_node_4 = aux_clock_2;
    aux_node_5 = aux_input_switch_0;
    // IC: DFLIPFLOP
    aux_ic_input_ic_0 = aux_node_3;
    aux_ic_input_ic_1 = aux_or_17;
    aux_ic_input_ic_2 = aux_node_4;
    aux_ic_input_ic_3 = aux_node_5;
    aux_ic_6_node_18 = aux_ic_input_ic_2;
    aux_ic_6_not_19 = !aux_ic_6_node_18;
    aux_ic_6_not_7 = !aux_ic_6_not_19;
    aux_ic_6_node_20 = aux_ic_6_not_19;
    aux_ic_6_node_1 = aux_ic_6_not_7;
    aux_ic_6_node_2 = aux_ic_6_node_20;
    aux_ic_6_node_3 = aux_ic_input_ic_1;
    aux_ic_6_node_16 = aux_ic_input_ic_0;
    aux_ic_6_node_17 = aux_ic_input_ic_3;
    aux_ic_6_not_13 = !aux_ic_6_node_3;
    aux_ic_6_node_21 = aux_ic_6_node_1;
    aux_ic_6_nand_4 = !(aux_ic_6_node_3 && aux_ic_6_node_2);
    aux_ic_6_nand_9 = !(aux_ic_6_node_16 && aux_ic_6_nand_4 && aux_ic_6_nand_8);
    aux_ic_6_node_22 = aux_ic_6_node_17;
    aux_ic_6_node_0 = aux_ic_6_node_21;
    aux_ic_6_nand_15 = !(aux_ic_6_node_20 && aux_ic_6_not_13);
    aux_ic_6_node_24 = aux_ic_6_node_16;
    aux_ic_6_nand_10 = !(aux_ic_6_nand_9 && aux_ic_6_node_0);
    aux_ic_6_nand_8 = !(aux_ic_6_nand_9 && aux_ic_6_nand_15 && aux_ic_6_node_22);
    aux_ic_6_nand_14 = !(aux_ic_6_node_21 && aux_ic_6_nand_8);
    aux_ic_6_nand_6 = !(aux_ic_6_node_24 && aux_ic_6_nand_10 && aux_ic_6_nand_11);
    aux_ic_6_node_23 = aux_ic_6_node_17;
    aux_ic_6_nand_11 = !(aux_ic_6_nand_6 && aux_ic_6_nand_14 && aux_ic_6_node_23);
    aux_ic_6_node_12 = aux_ic_6_nand_11;
    aux_ic_6_node_5 = aux_ic_6_nand_6;
    aux_ic_6_led_0 = aux_ic_6_node_5;
    aux_ic_6_led_0_1 = aux_ic_6_node_12;
    // End IC: DFLIPFLOP
    aux_node_7 = aux_ic_6_led_0_1;
    aux_node_8 = aux_ic_6_led_0;
    aux_input_switch_9 = input_switch3_j_val;
    aux_input_switch_10 = input_switch4_k_val;
    aux_node_11 = aux_node_8;
    aux_node_12 = aux_node_7;
    aux_not_13 = !aux_input_switch_9;
    aux_node_14 = aux_input_switch_10;
    aux_and_15 = aux_node_11 && aux_not_13;
    aux_and_16 = aux_node_14 && aux_node_12;
    aux_or_17 = aux_and_15 || aux_and_16;

    // Write output data
    digitalWrite(led19_q_0, aux_ic_6_led_0);
    digitalWrite(led20_q_0, aux_ic_6_led_0_1);
}
