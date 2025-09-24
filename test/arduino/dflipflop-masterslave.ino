// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //
//
// Target Board: Arduino UNO R3/R4
// Pin Usage: 3/18 pins
// Standard Arduino board with 20 I/O pins (14 digital + 6 analog)
//


/* ========= Inputs ========== */
const int push_button1_d = A0;

/* ========= Outputs ========== */
const int led8_q_0 = A1;
const int led9_q_0 = A2;

/* ====== Aux. Variables ====== */
bool push_button1_d_val = false;
bool aux_clock_0 = false;
unsigned long aux_clock_0_lastTime = 0;
const unsigned long aux_clock_0_interval = 1000;
bool aux_not_1 = false;
bool aux_push_button_2 = false;
bool aux_node_3 = false;
// IC: DLATCH
bool aux_dlatch_4_q_0 = false;
bool aux_dlatch_4_q_0_1 = false;
bool aux_dlatch_4_node_0 = false;
bool aux_dlatch_4_node_1 = false;
bool aux_dlatch_4_nand_2 = false;
bool aux_dlatch_4_nand_3 = false;
bool aux_dlatch_4_not_4 = false;
bool aux_dlatch_4_nand_5 = false;
bool aux_dlatch_4_nand_6 = false;
bool aux_dlatch_4_node_7 = false;
bool aux_dlatch_4_node_8 = false;
bool aux_ic_input_dlatch_0 = false;
bool aux_ic_input_dlatch_1 = false;
// End IC: DLATCH
bool aux_node_5 = false;
// IC: DLATCH
bool aux_dlatch_6_q_0 = false;
bool aux_dlatch_6_q_0_1 = false;
bool aux_dlatch_6_node_0 = false;
bool aux_dlatch_6_node_1 = false;
bool aux_dlatch_6_nand_2 = false;
bool aux_dlatch_6_nand_3 = false;
bool aux_dlatch_6_not_4 = false;
bool aux_dlatch_6_nand_5 = false;
bool aux_dlatch_6_nand_6 = false;
bool aux_dlatch_6_node_7 = false;
bool aux_dlatch_6_node_8 = false;
// End IC: DLATCH

void setup() {
    pinMode(push_button1_d, INPUT);
    pinMode(led8_q_0, OUTPUT);
    pinMode(led9_q_0, OUTPUT);
}

void loop() {
    // Read input data
    push_button1_d_val = digitalRead(push_button1_d);

    // Update clocks
    unsigned long now = millis();
    if (now - aux_clock_0_lastTime >= aux_clock_0_interval) {
        aux_clock_0_lastTime = now;
        aux_clock_0 = !aux_clock_0;
    }

    // Update logic variables
    aux_not_1 = !aux_clock_0;
    aux_push_button_2 = push_button1_d_val;
    aux_node_3 = aux_clock_0;
    // IC: DLATCH
    aux_ic_input_dlatch_0 = aux_push_button_2;
    aux_ic_input_dlatch_1 = aux_node_3;
    aux_dlatch_4_node_8 = aux_ic_input_dlatch_0;
    aux_dlatch_4_not_4 = !aux_dlatch_4_node_8;
    aux_dlatch_4_node_7 = aux_ic_input_dlatch_1;
    aux_dlatch_4_nand_5 = !(aux_dlatch_4_node_7 && aux_dlatch_4_not_4);
    aux_dlatch_4_nand_2 = !(aux_dlatch_4_nand_3 && aux_dlatch_4_nand_5);
    aux_dlatch_4_nand_6 = !(aux_dlatch_4_node_8 && aux_dlatch_4_node_7);
    aux_dlatch_4_nand_3 = !(aux_dlatch_4_nand_6 && aux_dlatch_4_nand_2);
    aux_dlatch_4_node_0 = aux_dlatch_4_nand_2;
    aux_dlatch_4_node_1 = aux_dlatch_4_nand_3;
    aux_dlatch_4_q_0 = aux_dlatch_4_node_1;
    aux_dlatch_4_q_0_1 = aux_dlatch_4_node_0;
    // End IC: DLATCH
    aux_node_5 = aux_not_1;
    // IC: DLATCH
    aux_ic_input_dlatch_0 = aux_dlatch_4_q_0;
    aux_ic_input_dlatch_1 = aux_node_5;
    aux_dlatch_6_node_8 = aux_ic_input_dlatch_0;
    aux_dlatch_6_not_4 = !aux_dlatch_6_node_8;
    aux_dlatch_6_node_7 = aux_ic_input_dlatch_1;
    aux_dlatch_6_nand_5 = !(aux_dlatch_6_node_7 && aux_dlatch_6_not_4);
    aux_dlatch_6_nand_2 = !(aux_dlatch_6_nand_3 && aux_dlatch_6_nand_5);
    aux_dlatch_6_nand_6 = !(aux_dlatch_6_node_8 && aux_dlatch_6_node_7);
    aux_dlatch_6_nand_3 = !(aux_dlatch_6_nand_6 && aux_dlatch_6_nand_2);
    aux_dlatch_6_node_0 = aux_dlatch_6_nand_2;
    aux_dlatch_6_node_1 = aux_dlatch_6_nand_3;
    aux_dlatch_6_q_0 = aux_dlatch_6_node_1;
    aux_dlatch_6_q_0_1 = aux_dlatch_6_node_0;
    // End IC: DLATCH

    // Write output data
    digitalWrite(led8_q_0, aux_dlatch_6_q_0_1);
    digitalWrite(led9_q_0, aux_dlatch_6_q_0);
}
