// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //
//
// Target Board: Arduino UNO R3/R4
// Pin Usage: 5/18 pins
// Standard Arduino board with 20 I/O pins (14 digital + 6 analog)
//


/* ========= Inputs ========== */
const int push_button1_t = A0;
const int input_switch2__preset = A1;
const int input_switch3__clear = A2;

/* ========= Outputs ========== */
const int led6_q_0 = A3;
const int led7_q_0 = A4;

/* ====== Aux. Variables ====== */
bool push_button1_t_val = false;
bool input_switch2__preset_val = false;
bool input_switch3__clear_val = false;
bool aux_node_0 = false;
bool aux_node_1 = false;
bool aux_push_button_2 = false;
bool aux_clock_3 = false;
unsigned long aux_clock_3_lastTime = 0;
const unsigned long aux_clock_3_interval = 1000;
// IC: DFLIPFLOP
bool aux_dflipflop_4_led_0 = false;
bool aux_dflipflop_4_led_0_1 = false;
bool aux_dflipflop_4_node_0 = false;
bool aux_dflipflop_4_node_1 = false;
bool aux_dflipflop_4_node_2 = false;
bool aux_dflipflop_4_node_3 = false;
bool aux_dflipflop_4_nand_4 = false;
bool aux_dflipflop_4_node_5 = false;
bool aux_dflipflop_4_nand_6 = false;
bool aux_dflipflop_4_not_7 = false;
bool aux_dflipflop_4_nand_8 = false;
bool aux_dflipflop_4_nand_9 = false;
bool aux_dflipflop_4_nand_10 = false;
bool aux_dflipflop_4_nand_11 = false;
bool aux_dflipflop_4_node_12 = false;
bool aux_dflipflop_4_not_13 = false;
bool aux_dflipflop_4_nand_14 = false;
bool aux_dflipflop_4_nand_15 = false;
bool aux_dflipflop_4_node_16 = false;
bool aux_dflipflop_4_node_17 = false;
bool aux_dflipflop_4_node_18 = false;
bool aux_dflipflop_4_not_19 = false;
bool aux_dflipflop_4_node_20 = false;
bool aux_dflipflop_4_node_21 = false;
bool aux_dflipflop_4_node_22 = false;
bool aux_dflipflop_4_node_23 = false;
bool aux_dflipflop_4_node_24 = false;
bool aux_ic_input_dflipflop_0 = false;
bool aux_ic_input_dflipflop_1 = false;
bool aux_ic_input_dflipflop_2 = false;
bool aux_ic_input_dflipflop_3 = false;
// End IC: DFLIPFLOP
bool aux_input_switch_7 = false;
bool aux_input_switch_8 = false;
bool aux_or_9 = false;
bool aux_not_10 = false;
bool aux_and_11 = false;
bool aux_and_12 = false;
bool aux_node_13 = false;

void setup() {
    pinMode(push_button1_t, INPUT);
    pinMode(input_switch2__preset, INPUT);
    pinMode(input_switch3__clear, INPUT);
    pinMode(led6_q_0, OUTPUT);
    pinMode(led7_q_0, OUTPUT);
}

void loop() {
    // Read input data
    push_button1_t_val = digitalRead(push_button1_t);
    input_switch2__preset_val = digitalRead(input_switch2__preset);
    input_switch3__clear_val = digitalRead(input_switch3__clear);

    // Update clocks
    unsigned long now = millis();
    if (now - aux_clock_3_lastTime >= aux_clock_3_interval) {
        aux_clock_3_lastTime = now;
        aux_clock_3 = !aux_clock_3;
    }

    // Update logic variables
    aux_node_0 = aux_dflipflop_4_led_0;
    aux_node_1 = aux_node_0;
    aux_push_button_2 = push_button1_t_val;
    // IC: DFLIPFLOP
    aux_ic_input_dflipflop_0 = aux_input_switch_7;
    aux_ic_input_dflipflop_1 = aux_or_9;
    aux_ic_input_dflipflop_2 = aux_clock_3;
    aux_ic_input_dflipflop_3 = aux_input_switch_8;
    aux_dflipflop_4_node_18 = aux_ic_input_dflipflop_2;
    aux_dflipflop_4_not_19 = !aux_dflipflop_4_node_18;
    aux_dflipflop_4_not_7 = !aux_dflipflop_4_not_19;
    aux_dflipflop_4_node_20 = aux_dflipflop_4_not_19;
    aux_dflipflop_4_node_1 = aux_dflipflop_4_not_7;
    aux_dflipflop_4_node_2 = aux_dflipflop_4_node_20;
    aux_dflipflop_4_node_3 = aux_ic_input_dflipflop_1;
    aux_dflipflop_4_node_16 = aux_ic_input_dflipflop_0;
    aux_dflipflop_4_node_17 = aux_ic_input_dflipflop_3;
    aux_dflipflop_4_not_13 = !aux_dflipflop_4_node_3;
    aux_dflipflop_4_node_21 = aux_dflipflop_4_node_1;
    aux_dflipflop_4_nand_4 = !(aux_dflipflop_4_node_3 && aux_dflipflop_4_node_2);
    aux_dflipflop_4_nand_9 = !(aux_dflipflop_4_node_16 && aux_dflipflop_4_nand_4 && aux_dflipflop_4_nand_8);
    aux_dflipflop_4_node_22 = aux_dflipflop_4_node_17;
    aux_dflipflop_4_node_0 = aux_dflipflop_4_node_21;
    aux_dflipflop_4_nand_15 = !(aux_dflipflop_4_node_20 && aux_dflipflop_4_not_13);
    aux_dflipflop_4_node_24 = aux_dflipflop_4_node_16;
    aux_dflipflop_4_nand_10 = !(aux_dflipflop_4_nand_9 && aux_dflipflop_4_node_0);
    aux_dflipflop_4_nand_8 = !(aux_dflipflop_4_nand_9 && aux_dflipflop_4_nand_15 && aux_dflipflop_4_node_22);
    aux_dflipflop_4_nand_14 = !(aux_dflipflop_4_node_21 && aux_dflipflop_4_nand_8);
    aux_dflipflop_4_nand_6 = !(aux_dflipflop_4_node_24 && aux_dflipflop_4_nand_10 && aux_dflipflop_4_nand_11);
    aux_dflipflop_4_node_23 = aux_dflipflop_4_node_17;
    aux_dflipflop_4_nand_11 = !(aux_dflipflop_4_nand_6 && aux_dflipflop_4_nand_14 && aux_dflipflop_4_node_23);
    aux_dflipflop_4_node_12 = aux_dflipflop_4_nand_11;
    aux_dflipflop_4_node_5 = aux_dflipflop_4_nand_6;
    aux_dflipflop_4_led_0 = aux_dflipflop_4_node_5;
    aux_dflipflop_4_led_0_1 = aux_dflipflop_4_node_12;
    // End IC: DFLIPFLOP
    aux_input_switch_7 = input_switch2__preset_val;
    aux_input_switch_8 = input_switch3__clear_val;
    aux_or_9 = aux_and_11 || aux_and_12;
    aux_not_10 = !aux_push_button_2;
    aux_and_11 = aux_node_1 && aux_not_10;
    aux_and_12 = aux_node_13 && aux_dflipflop_4_led_0_1;
    aux_node_13 = aux_push_button_2;

    // Write output data
    digitalWrite(led6_q_0, aux_dflipflop_4_led_0);
    digitalWrite(led7_q_0, aux_dflipflop_4_led_0_1);
}
