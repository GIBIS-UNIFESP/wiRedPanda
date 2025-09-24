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
const int led4_q_0 = A1;
const int led6_q_0 = A2;

/* ====== Aux. Variables ====== */
bool push_button1_d_val = false;
// IC: DLATCH
bool aux_dlatch_0_q_0 = false;
bool aux_dlatch_0_q_0_1 = false;
bool aux_dlatch_0_node_0 = false;
bool aux_dlatch_0_node_1 = false;
bool aux_dlatch_0_nand_2 = false;
bool aux_dlatch_0_nand_3 = false;
bool aux_dlatch_0_not_4 = false;
bool aux_dlatch_0_nand_5 = false;
bool aux_dlatch_0_nand_6 = false;
bool aux_dlatch_0_node_7 = false;
bool aux_dlatch_0_node_8 = false;
bool aux_ic_input_dlatch_0 = false;
bool aux_ic_input_dlatch_1 = false;
// End IC: DLATCH
bool aux_clock_1 = false;
unsigned long aux_clock_1_lastTime = 0;
const unsigned long aux_clock_1_interval = 1000;
// IC: DLATCH
bool aux_dlatch_2_q_0 = false;
bool aux_dlatch_2_q_0_1 = false;
bool aux_dlatch_2_node_0 = false;
bool aux_dlatch_2_node_1 = false;
bool aux_dlatch_2_nand_2 = false;
bool aux_dlatch_2_nand_3 = false;
bool aux_dlatch_2_not_4 = false;
bool aux_dlatch_2_nand_5 = false;
bool aux_dlatch_2_nand_6 = false;
bool aux_dlatch_2_node_7 = false;
bool aux_dlatch_2_node_8 = false;
// End IC: DLATCH
bool aux_not_4 = false;
bool aux_push_button_6 = false;
bool aux_node_7 = false;
bool aux_node_8 = false;

void setup() {
    pinMode(push_button1_d, INPUT);
    pinMode(led4_q_0, OUTPUT);
    pinMode(led6_q_0, OUTPUT);
}

void loop() {
    // Read input data
    push_button1_d_val = digitalRead(push_button1_d);

    // Update clocks
    unsigned long now = millis();
    if (now - aux_clock_1_lastTime >= aux_clock_1_interval) {
        aux_clock_1_lastTime = now;
        aux_clock_1 = !aux_clock_1;
    }

    // Update logic variables
    // IC: DLATCH
    aux_ic_input_dlatch_0 = aux_dlatch_2_q_0;
    aux_ic_input_dlatch_1 = aux_node_7;
    aux_dlatch_0_node_8 = aux_ic_input_dlatch_0;
    aux_dlatch_0_not_4 = !aux_dlatch_0_node_8;
    aux_dlatch_0_node_7 = aux_ic_input_dlatch_1;
    aux_dlatch_0_nand_5 = !(aux_dlatch_0_node_7 && aux_dlatch_0_not_4);
    aux_dlatch_0_nand_2 = !(aux_dlatch_0_nand_3 && aux_dlatch_0_nand_5);
    aux_dlatch_0_nand_6 = !(aux_dlatch_0_node_8 && aux_dlatch_0_node_7);
    aux_dlatch_0_nand_3 = !(aux_dlatch_0_nand_6 && aux_dlatch_0_nand_2);
    aux_dlatch_0_node_0 = aux_dlatch_0_nand_2;
    aux_dlatch_0_node_1 = aux_dlatch_0_nand_3;
    aux_dlatch_0_q_0 = aux_dlatch_0_node_1;
    aux_dlatch_0_q_0_1 = aux_dlatch_0_node_0;
    // End IC: DLATCH
    // IC: DLATCH
    aux_ic_input_dlatch_0 = aux_push_button_6;
    aux_ic_input_dlatch_1 = aux_node_8;
    aux_dlatch_2_node_8 = aux_ic_input_dlatch_0;
    aux_dlatch_2_not_4 = !aux_dlatch_2_node_8;
    aux_dlatch_2_node_7 = aux_ic_input_dlatch_1;
    aux_dlatch_2_nand_5 = !(aux_dlatch_2_node_7 && aux_dlatch_2_not_4);
    aux_dlatch_2_nand_2 = !(aux_dlatch_2_nand_3 && aux_dlatch_2_nand_5);
    aux_dlatch_2_nand_6 = !(aux_dlatch_2_node_8 && aux_dlatch_2_node_7);
    aux_dlatch_2_nand_3 = !(aux_dlatch_2_nand_6 && aux_dlatch_2_nand_2);
    aux_dlatch_2_node_0 = aux_dlatch_2_nand_2;
    aux_dlatch_2_node_1 = aux_dlatch_2_nand_3;
    aux_dlatch_2_q_0 = aux_dlatch_2_node_1;
    aux_dlatch_2_q_0_1 = aux_dlatch_2_node_0;
    // End IC: DLATCH
    aux_not_4 = !aux_clock_1;
    aux_push_button_6 = push_button1_d_val;
    aux_node_7 = aux_not_4;
    aux_node_8 = aux_clock_1;

    // Write output data
    digitalWrite(led4_q_0, aux_dlatch_0_q_0_1);
    digitalWrite(led6_q_0, aux_dlatch_0_q_0);
}
