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
const int led24_0 = A3;
const int led25_0 = A4;

/* ====== Aux. Variables ====== */
bool push_button1_d_val = false;
bool input_switch2__preset_val = false;
bool input_switch3__clear_val = false;
bool aux_clock_0 = false;
unsigned long aux_clock_0_lastTime = 0;
const unsigned long aux_clock_0_interval = 1000;
bool aux_not_1 = false;
bool aux_not_2 = false;
bool aux_node_3 = false;
bool aux_node_4 = false;
bool aux_node_5 = false;
bool aux_push_button_6 = false;
bool aux_nand_7 = false;
bool aux_not_8 = false;
bool aux_input_switch_9 = false;
bool aux_input_switch_10 = false;
bool aux_node_11 = false;
bool aux_node_12 = false;
bool aux_nand_13 = false;
bool aux_nand_14 = false;
bool aux_node_15 = false;
bool aux_nand_16 = false;
bool aux_nand_17 = false;
bool aux_node_18 = false;
bool aux_nand_19 = false;
bool aux_nand_20 = false;
bool aux_node_21 = false;
bool aux_nand_22 = false;

void setup() {
    pinMode(push_button1_d, INPUT);
    pinMode(input_switch2__preset, INPUT);
    pinMode(input_switch3__clear, INPUT);
    pinMode(led24_0, OUTPUT);
    pinMode(led25_0, OUTPUT);
}

void loop() {
    // Read input data
    push_button1_d_val = digitalRead(push_button1_d);
    input_switch2__preset_val = digitalRead(input_switch2__preset);
    input_switch3__clear_val = digitalRead(input_switch3__clear);

    // Update clocks
    unsigned long now = millis();
    if (now - aux_clock_0_lastTime >= aux_clock_0_interval) {
        aux_clock_0_lastTime = now;
        aux_clock_0 = !aux_clock_0;
    }

    // Update logic variables
    aux_not_1 = !aux_clock_0;
    aux_not_2 = !aux_not_1;
    aux_node_3 = aux_not_1;
    aux_node_4 = aux_not_2;
    aux_node_5 = aux_node_3;
    aux_push_button_6 = push_button1_d_val;
    aux_nand_7 = !(aux_push_button_6 && aux_node_5);
    aux_not_8 = !aux_push_button_6;
    aux_input_switch_9 = input_switch2__preset_val;
    aux_input_switch_10 = input_switch3__clear_val;
    aux_node_11 = aux_node_4;
    aux_node_12 = aux_node_11;
    aux_nand_13 = !(aux_input_switch_9 && aux_nand_7 && aux_nand_16);
    aux_nand_14 = !(aux_node_3 && aux_not_8);
    aux_node_15 = aux_input_switch_10;
    aux_nand_16 = !(aux_nand_13 && aux_nand_14 && aux_node_15);
    aux_nand_17 = !(aux_nand_13 && aux_node_12);
    aux_node_18 = aux_input_switch_9;
    aux_nand_19 = !(aux_node_18 && aux_nand_17 && aux_nand_22);
    aux_nand_20 = !(aux_node_11 && aux_nand_16);
    aux_node_21 = aux_input_switch_10;
    aux_nand_22 = !(aux_nand_19 && aux_nand_20 && aux_node_21);

    // Write output data
    digitalWrite(led24_0, aux_nand_19);
    digitalWrite(led25_0, aux_nand_22);
}
