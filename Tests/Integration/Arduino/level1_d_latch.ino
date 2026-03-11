// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //
//
// Target Board: Arduino UNO R3/R4
// Pin Usage: 4/18 pins
//


#include <elapsedMillis.h>
/* ========= Inputs ========== */
const int input_switch1 = A0;
const int input_switch2 = A1;

/* ========= Outputs ========== */
const int led1_0 = A2;
const int led2_0 = A3;

/* ====== Aux. Variables ====== */
bool input_switch1_val = LOW;
bool input_switch2_val = LOW;
// IC: LEVEL1_D_LATCH
bool aux_level1_d_latch_0_q = LOW;
bool aux_level1_d_latch_0_q_bar_1 = LOW;
bool aux_level1_d_latch_0_node_0 = LOW;
bool aux_level1_d_latch_0_node_1 = LOW;
bool aux_level1_d_latch_0_not_2 = LOW;
bool aux_level1_d_latch_0_and_3 = LOW;
bool aux_level1_d_latch_0_and_4 = LOW;
bool aux_level1_d_latch_0_nor_5 = LOW;
bool aux_level1_d_latch_0_nor_6 = LOW;
bool aux_level1_d_latch_0_node_7 = LOW;
bool aux_level1_d_latch_0_node_8 = LOW;
bool aux_ic_input_level1_d_latch_0 = LOW;
bool aux_ic_input_level1_d_latch_1 = LOW;
// End IC: LEVEL1_D_LATCH

void setup() {
    pinMode(input_switch1, INPUT);
    pinMode(input_switch2, INPUT);
    pinMode(led1_0, OUTPUT);
    pinMode(led2_0, OUTPUT);
}

void computeLogic() {
    // Assigning aux variables. //
    // IC: LEVEL1_D_LATCH
    aux_ic_input_level1_d_latch_0 = input_switch1_val;
    aux_ic_input_level1_d_latch_1 = input_switch2_val;
    aux_level1_d_latch_0_node_0 = aux_ic_input_level1_d_latch_0;
    aux_level1_d_latch_0_node_1 = aux_ic_input_level1_d_latch_1;
    aux_level1_d_latch_0_not_2 = !aux_level1_d_latch_0_node_0;
    aux_level1_d_latch_0_and_4 = aux_level1_d_latch_0_not_2 && aux_level1_d_latch_0_node_1;
    aux_level1_d_latch_0_and_3 = aux_level1_d_latch_0_node_0 && aux_level1_d_latch_0_node_1;
    aux_level1_d_latch_0_nor_5 = !(aux_level1_d_latch_0_and_4 || aux_level1_d_latch_0_nor_6);
    aux_level1_d_latch_0_nor_6 = !(aux_level1_d_latch_0_and_3 || aux_level1_d_latch_0_nor_5);
    aux_level1_d_latch_0_node_7 = aux_level1_d_latch_0_nor_5;
    aux_level1_d_latch_0_node_8 = aux_level1_d_latch_0_nor_6;
    aux_level1_d_latch_0_q = aux_level1_d_latch_0_node_7;
    aux_level1_d_latch_0_q_bar_1 = aux_level1_d_latch_0_node_8;
    // End IC: LEVEL1_D_LATCH
}

void loop() {
    // Reading input data //.
    input_switch1_val = digitalRead(input_switch1);
    input_switch2_val = digitalRead(input_switch2);

    // Updating clocks. //

    computeLogic();

    // Writing output data. //
    digitalWrite(led1_0, aux_level1_d_latch_0_q);
    digitalWrite(led2_0, aux_level1_d_latch_0_q_bar_1);
}
