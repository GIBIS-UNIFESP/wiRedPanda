// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


/* ========= Inputs ========== */
const int push_button1_t = A0;
const int input_switch2__preset = A1;
const int input_switch3__clear = A2;

/* ========= Outputs ========== */
const int led12_q_0 = A3;
const int led13_q_0 = A4;

/* ====== Aux. Variables ====== */
bool push_button1_t_val = false;
bool input_switch2__preset_val = false;
bool input_switch3__clear_val = false;
bool aux_node_0 = false;
bool aux_push_button_1 = false;
bool aux_node_2 = false;
bool aux_not_3 = false;
bool aux_and_4 = false;
bool aux_clock_5 = false;
unsigned long aux_clock_5_lastTime = 0;
const unsigned long aux_clock_5_interval = 1000;
bool aux_input_switch_6 = false;
bool aux_input_switch_7 = false;
bool aux_or_8 = false;
// IC: DFLIPFLOP
bool aux_ic_9_led_0 = false;
bool aux_ic_9_led_0_1 = false;
bool aux_ic_9_node_0 = false;
bool aux_ic_9_node_1 = false;
bool aux_ic_9_node_2 = false;
bool aux_ic_9_node_3 = false;
bool aux_ic_9_nand_4 = false;
bool aux_ic_9_node_5 = false;
bool aux_ic_9_nand_6 = false;
bool aux_ic_9_not_7 = false;
bool aux_ic_9_nand_8 = false;
bool aux_ic_9_nand_9 = false;
bool aux_ic_9_nand_10 = false;
bool aux_ic_9_nand_11 = false;
bool aux_ic_9_node_12 = false;
bool aux_ic_9_not_13 = false;
bool aux_ic_9_nand_14 = false;
bool aux_ic_9_nand_15 = false;
bool aux_ic_9_node_16 = false;
bool aux_ic_9_node_17 = false;
bool aux_ic_9_node_18 = false;
bool aux_ic_9_not_19 = false;
bool aux_ic_9_node_20 = false;
bool aux_ic_9_node_21 = false;
bool aux_ic_9_node_22 = false;
bool aux_ic_9_node_23 = false;
bool aux_ic_9_node_24 = false;
bool aux_ic_input_ic_0 = false;
bool aux_ic_input_ic_1 = false;
bool aux_ic_input_ic_2 = false;
bool aux_ic_input_ic_3 = false;
// End IC: DFLIPFLOP
bool aux_node_10 = false;
bool aux_and_13 = false;

void setup() {
    pinMode(push_button1_t, INPUT);
    pinMode(input_switch2__preset, INPUT);
    pinMode(input_switch3__clear, INPUT);
    pinMode(led12_q_0, OUTPUT);
    pinMode(led13_q_0, OUTPUT);
}

void loop() {
    // Read input data
    push_button1_t_val = digitalRead(push_button1_t);
    input_switch2__preset_val = digitalRead(input_switch2__preset);
    input_switch3__clear_val = digitalRead(input_switch3__clear);

    // Update clocks
    unsigned long now = millis();
    if (now - aux_clock_5_lastTime >= aux_clock_5_interval) {
        aux_clock_5_lastTime = now;
        aux_clock_5 = !aux_clock_5;
    }

    // Update logic variables
    aux_node_0 = aux_ic_9_led_0;
    aux_push_button_1 = push_button1_t_val;
    aux_node_2 = aux_node_0;
    aux_not_3 = !aux_push_button_1;
    aux_and_4 = aux_node_2 && aux_not_3;
    aux_input_switch_6 = input_switch2__preset_val;
    aux_input_switch_7 = input_switch3__clear_val;
    aux_or_8 = aux_and_4 || aux_and_13;
    // IC: DFLIPFLOP
    aux_ic_input_ic_0 = aux_input_switch_6;
    aux_ic_input_ic_1 = aux_or_8;
    aux_ic_input_ic_2 = aux_clock_5;
    aux_ic_input_ic_3 = aux_input_switch_7;
    aux_ic_9_node_18 = aux_ic_input_ic_2;
    aux_ic_9_not_19 = !aux_ic_9_node_18;
    aux_ic_9_not_7 = !aux_ic_9_not_19;
    aux_ic_9_node_20 = aux_ic_9_not_19;
    aux_ic_9_node_1 = aux_ic_9_not_7;
    aux_ic_9_node_2 = aux_ic_9_node_20;
    aux_ic_9_node_3 = aux_ic_input_ic_1;
    aux_ic_9_node_16 = aux_ic_input_ic_0;
    aux_ic_9_node_17 = aux_ic_input_ic_3;
    aux_ic_9_not_13 = !aux_ic_9_node_3;
    aux_ic_9_node_21 = aux_ic_9_node_1;
    aux_ic_9_nand_4 = !(aux_ic_9_node_3 && aux_ic_9_node_2);
    aux_ic_9_nand_9 = !(aux_ic_9_node_16 && aux_ic_9_nand_4 && aux_ic_9_nand_8);
    aux_ic_9_node_22 = aux_ic_9_node_17;
    aux_ic_9_node_0 = aux_ic_9_node_21;
    aux_ic_9_nand_15 = !(aux_ic_9_node_20 && aux_ic_9_not_13);
    aux_ic_9_node_24 = aux_ic_9_node_16;
    aux_ic_9_nand_10 = !(aux_ic_9_nand_9 && aux_ic_9_node_0);
    aux_ic_9_nand_8 = !(aux_ic_9_nand_9 && aux_ic_9_nand_15 && aux_ic_9_node_22);
    aux_ic_9_nand_14 = !(aux_ic_9_node_21 && aux_ic_9_nand_8);
    aux_ic_9_nand_6 = !(aux_ic_9_node_24 && aux_ic_9_nand_10 && aux_ic_9_nand_11);
    aux_ic_9_node_23 = aux_ic_9_node_17;
    aux_ic_9_nand_11 = !(aux_ic_9_nand_6 && aux_ic_9_nand_14 && aux_ic_9_node_23);
    aux_ic_9_node_12 = aux_ic_9_nand_11;
    aux_ic_9_node_5 = aux_ic_9_nand_6;
    aux_ic_9_led_0 = aux_ic_9_node_5;
    aux_ic_9_led_0_1 = aux_ic_9_node_12;
    // End IC: DFLIPFLOP
    aux_node_10 = aux_push_button_1;
    aux_and_13 = aux_node_10 && aux_ic_9_led_0_1;

    // Write output data
    digitalWrite(led12_q_0, aux_ic_9_led_0);
    digitalWrite(led13_q_0, aux_ic_9_led_0_1);
}
