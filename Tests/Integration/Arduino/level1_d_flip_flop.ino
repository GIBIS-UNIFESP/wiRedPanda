// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //
//
// Target Board: Arduino UNO R3/R4
// Pin Usage: 6/18 pins
//


#include <elapsedMillis.h>
/* ========= Inputs ========== */
const int input_switch1 = A0;
const int input_switch2 = A1;
const int input_switch3 = A2;
const int input_switch4 = A3;

/* ========= Outputs ========== */
const int led1_0 = A4;
const int led2_0 = A5;

/* ====== Aux. Variables ====== */
bool input_switch1_val = LOW;
bool input_switch2_val = LOW;
bool input_switch3_val = LOW;
bool input_switch4_val = LOW;
// IC: LEVEL1_D_FLIP_FLOP
bool aux_level1_d_flip_flop_0_q = LOW;
bool aux_level1_d_flip_flop_0_q_bar_1 = LOW;
bool aux_level1_d_flip_flop_0_node_0 = LOW;
bool aux_level1_d_flip_flop_0_node_1 = LOW;
bool aux_level1_d_flip_flop_0_node_2 = LOW;
bool aux_level1_d_flip_flop_0_node_3 = LOW;
bool aux_level1_d_flip_flop_0_not_4 = LOW;
bool aux_level1_d_flip_flop_0_not_5 = LOW;
bool aux_level1_d_flip_flop_0_not_6 = LOW;
bool aux_level1_d_flip_flop_0_or_7 = LOW;
bool aux_level1_d_flip_flop_0_or_8 = LOW;
bool aux_level1_d_flip_flop_0_not_9 = LOW;
bool aux_level1_d_flip_flop_0_and_10 = LOW;
bool aux_level1_d_flip_flop_0_and_11 = LOW;
bool aux_level1_d_flip_flop_0_nor_12 = LOW;
bool aux_level1_d_flip_flop_0_nor_13 = LOW;
bool aux_level1_d_flip_flop_0_not_14 = LOW;
bool aux_level1_d_flip_flop_0_and_15 = LOW;
bool aux_level1_d_flip_flop_0_and_16 = LOW;
bool aux_level1_d_flip_flop_0_nor_17 = LOW;
bool aux_level1_d_flip_flop_0_nor_18 = LOW;
bool aux_level1_d_flip_flop_0_node_19 = LOW;
bool aux_level1_d_flip_flop_0_node_20 = LOW;
bool aux_ic_input_level1_d_flip_flop_0 = LOW;
bool aux_ic_input_level1_d_flip_flop_1 = LOW;
bool aux_ic_input_level1_d_flip_flop_2 = LOW;
bool aux_ic_input_level1_d_flip_flop_3 = LOW;
// End IC: LEVEL1_D_FLIP_FLOP

void setup() {
    pinMode(input_switch1, INPUT);
    pinMode(input_switch2, INPUT);
    pinMode(input_switch3, INPUT);
    pinMode(input_switch4, INPUT);
    pinMode(led1_0, OUTPUT);
    pinMode(led2_0, OUTPUT);
}

void computeLogic() {
    // Assigning aux variables. //
    // IC: LEVEL1_D_FLIP_FLOP
    aux_ic_input_level1_d_flip_flop_0 = input_switch1_val;
    aux_ic_input_level1_d_flip_flop_1 = input_switch2_val;
    aux_ic_input_level1_d_flip_flop_2 = input_switch3_val;
    aux_ic_input_level1_d_flip_flop_3 = input_switch4_val;
    aux_level1_d_flip_flop_0_node_0 = aux_ic_input_level1_d_flip_flop_0;
    aux_level1_d_flip_flop_0_node_1 = aux_ic_input_level1_d_flip_flop_1;
    aux_level1_d_flip_flop_0_not_4 = !aux_level1_d_flip_flop_0_node_1;
    aux_level1_d_flip_flop_0_not_9 = !aux_level1_d_flip_flop_0_node_0;
    aux_level1_d_flip_flop_0_and_11 = aux_level1_d_flip_flop_0_not_9 && aux_level1_d_flip_flop_0_not_4;
    aux_level1_d_flip_flop_0_nor_12 = !(aux_level1_d_flip_flop_0_and_11 || aux_level1_d_flip_flop_0_nor_13);
    aux_level1_d_flip_flop_0_node_3 = aux_ic_input_level1_d_flip_flop_3;
    aux_level1_d_flip_flop_0_not_14 = !aux_level1_d_flip_flop_0_nor_12;
    aux_level1_d_flip_flop_0_node_2 = aux_ic_input_level1_d_flip_flop_2;
    aux_level1_d_flip_flop_0_not_6 = !aux_level1_d_flip_flop_0_node_3;
    aux_level1_d_flip_flop_0_and_16 = aux_level1_d_flip_flop_0_not_14 && aux_level1_d_flip_flop_0_node_1;
    aux_level1_d_flip_flop_0_not_5 = !aux_level1_d_flip_flop_0_node_2;
    aux_level1_d_flip_flop_0_or_7 = aux_level1_d_flip_flop_0_and_16 || aux_level1_d_flip_flop_0_not_6;
    aux_level1_d_flip_flop_0_and_15 = aux_level1_d_flip_flop_0_nor_12 && aux_level1_d_flip_flop_0_node_1;
    aux_level1_d_flip_flop_0_or_8 = aux_level1_d_flip_flop_0_and_15 || aux_level1_d_flip_flop_0_not_5;
    aux_level1_d_flip_flop_0_nor_17 = !(aux_level1_d_flip_flop_0_or_7 || aux_level1_d_flip_flop_0_nor_18);
    aux_level1_d_flip_flop_0_and_10 = aux_level1_d_flip_flop_0_node_0 && aux_level1_d_flip_flop_0_not_4;
    aux_level1_d_flip_flop_0_nor_18 = !(aux_level1_d_flip_flop_0_or_8 || aux_level1_d_flip_flop_0_nor_17);
    aux_level1_d_flip_flop_0_nor_13 = !(aux_level1_d_flip_flop_0_and_10 || aux_level1_d_flip_flop_0_nor_12);
    aux_level1_d_flip_flop_0_node_19 = aux_level1_d_flip_flop_0_nor_17;
    aux_level1_d_flip_flop_0_node_20 = aux_level1_d_flip_flop_0_nor_18;
    aux_level1_d_flip_flop_0_q = aux_level1_d_flip_flop_0_node_19;
    aux_level1_d_flip_flop_0_q_bar_1 = aux_level1_d_flip_flop_0_node_20;
    // End IC: LEVEL1_D_FLIP_FLOP
}

void loop() {
    // Reading input data //.
    input_switch1_val = digitalRead(input_switch1);
    input_switch2_val = digitalRead(input_switch2);
    input_switch3_val = digitalRead(input_switch3);
    input_switch4_val = digitalRead(input_switch4);

    // Updating clocks. //

    computeLogic();

    // Writing output data. //
    digitalWrite(led1_0, aux_level1_d_flip_flop_0_q);
    digitalWrite(led2_0, aux_level1_d_flip_flop_0_q_bar_1);
}
