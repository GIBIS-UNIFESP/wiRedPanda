// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //
//
// Target Board: Arduino UNO R3/R4
// Pin Usage: 5/18 pins
// Standard Arduino board with 20 I/O pins (14 digital + 6 analog)
//


/* ========= Inputs ========== */
const int push_button1_d = A0;
const int input_switch2__preset = A1;
const int input_switch3__clear = A2;

/* ========= Outputs ========== */
const int led6_0 = A3;
const int led13_0 = A4;

/* ====== Aux. Variables ====== */
bool push_button1_d_val = false;
bool input_switch2__preset_val = false;
bool input_switch3__clear_val = false;
bool aux_node_0 = false;
bool aux_node_1 = false;
bool aux_node_2 = false;
bool aux_push_button_3 = false;
bool aux_nand_4 = false;
bool aux_nand_6 = false;
bool aux_not_7 = false;
bool aux_nand_8 = false;
bool aux_nand_9 = false;
bool aux_nand_10 = false;
bool aux_nand_11 = false;
bool aux_not_13 = false;
bool aux_nand_14 = false;
bool aux_nand_15 = false;
bool aux_input_switch_16 = false;
bool aux_input_switch_17 = false;
bool aux_clock_18 = false;
unsigned long aux_clock_18_lastTime = 0;
const unsigned long aux_clock_18_interval = 1000;
bool aux_not_19 = false;
bool aux_node_20 = false;
bool aux_node_21 = false;
bool aux_node_22 = false;
bool aux_node_23 = false;
bool aux_node_24 = false;

void setup() {
    pinMode(push_button1_d, INPUT);
    pinMode(input_switch2__preset, INPUT);
    pinMode(input_switch3__clear, INPUT);
    pinMode(led6_0, OUTPUT);
    pinMode(led13_0, OUTPUT);
}

void loop() {
    // Read input data
    push_button1_d_val = digitalRead(push_button1_d);
    input_switch2__preset_val = digitalRead(input_switch2__preset);
    input_switch3__clear_val = digitalRead(input_switch3__clear);

    // Update clocks
    unsigned long now = millis();
    if (now - aux_clock_18_lastTime >= aux_clock_18_interval) {
        aux_clock_18_lastTime = now;
        aux_clock_18 = !aux_clock_18;
    }

    // Update logic variables
    aux_node_0 = aux_node_21;
    aux_node_1 = aux_not_7;
    aux_node_2 = aux_node_20;
    aux_push_button_3 = push_button1_d_val;
    aux_nand_4 = !(aux_push_button_3 && aux_node_2);
    aux_nand_6 = !(aux_node_24 && aux_nand_10 && aux_nand_11);
    aux_not_7 = !aux_not_19;
    aux_nand_8 = !(aux_nand_9 && aux_nand_15 && aux_node_22);
    aux_nand_9 = !(aux_input_switch_16 && aux_nand_4 && aux_nand_8);
    aux_nand_10 = !(aux_nand_9 && aux_node_0);
    aux_nand_11 = !(aux_nand_6 && aux_nand_14 && aux_node_23);
    aux_not_13 = !aux_push_button_3;
    aux_nand_14 = !(aux_node_21 && aux_nand_8);
    aux_nand_15 = !(aux_node_20 && aux_not_13);
    aux_input_switch_16 = input_switch2__preset_val;
    aux_input_switch_17 = input_switch3__clear_val;
    aux_not_19 = !aux_clock_18;
    aux_node_20 = aux_not_19;
    aux_node_21 = aux_node_1;
    aux_node_22 = aux_input_switch_17;
    aux_node_23 = aux_input_switch_17;
    aux_node_24 = aux_input_switch_16;

    // Write output data
    digitalWrite(led6_0, aux_nand_6);
    digitalWrite(led13_0, aux_nand_11);
}
