// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //
//
// Target Board: Arduino UNO R3/R4
// Pin Usage: 5/18 pins
//


#include <elapsedMillis.h>
/* ========= Inputs ========== */
const int input_switch1 = A0;

/* ========= Outputs ========== */
const int led1_1 = A1;
const int led2_1 = A2;
const int led3_1 = A3;
const int led4_1 = A4;

/* ====== Aux. Variables ====== */
bool input_switch1_val = LOW;
// IC: LEVEL4_BINARY_COUNTER_4BIT
bool aux_level4_binary_counter_4bit_0_q0 = LOW;
bool aux_level4_binary_counter_4bit_0_q1_1 = LOW;
bool aux_level4_binary_counter_4bit_0_q2_2 = LOW;
bool aux_level4_binary_counter_4bit_0_q3_3 = LOW;
bool aux_level4_binary_counter_4bit_0_node_0 = LOW;
bool aux_level4_binary_counter_4bit_0_not_2 = LOW;
bool aux_level4_binary_counter_4bit_0_not_3 = LOW;
bool aux_level4_binary_counter_4bit_0_not_4 = LOW;
bool aux_level4_binary_counter_4bit_0_not_5 = LOW;
bool aux_level4_binary_counter_4bit_0_and_6 = LOW;
bool aux_level4_binary_counter_4bit_0_and_7 = LOW;
bool aux_level4_binary_counter_4bit_0_not_8 = LOW;
bool aux_level4_binary_counter_4bit_0_not_9 = LOW;
bool aux_level4_binary_counter_4bit_0_not_10 = LOW;
bool aux_level4_binary_counter_4bit_0_and_11 = LOW;
bool aux_level4_binary_counter_4bit_0_and_12 = LOW;
bool aux_level4_binary_counter_4bit_0_and_13 = LOW;
bool aux_level4_binary_counter_4bit_0_and_14 = LOW;
bool aux_level4_binary_counter_4bit_0_and_15 = LOW;
bool aux_level4_binary_counter_4bit_0_and_16 = LOW;
bool aux_level4_binary_counter_4bit_0_or_17 = LOW;
bool aux_level4_binary_counter_4bit_0_or_18 = LOW;
bool aux_level4_binary_counter_4bit_0_or_19 = LOW;
// IC: FF0
bool aux_ff0_20_q = LOW;
bool aux_ff0_20_q_bar_1 = LOW;
bool aux_ff0_20_node_0 = LOW;
bool aux_ff0_20_node_1 = LOW;
bool aux_ff0_20_node_2 = LOW;
bool aux_ff0_20_node_3 = LOW;
bool aux_ff0_20_not_4 = LOW;
bool aux_ff0_20_not_5 = LOW;
bool aux_ff0_20_not_6 = LOW;
bool aux_ff0_20_or_7 = LOW;
bool aux_ff0_20_or_8 = LOW;
bool aux_ff0_20_or_9 = LOW;
bool aux_ff0_20_or_10 = LOW;
bool aux_ff0_20_not_11 = LOW;
bool aux_ff0_20_and_12 = LOW;
bool aux_ff0_20_and_13 = LOW;
bool aux_ff0_20_nor_14 = LOW;
bool aux_ff0_20_nor_15 = LOW;
bool aux_ff0_20_not_16 = LOW;
bool aux_ff0_20_and_17 = LOW;
bool aux_ff0_20_and_18 = LOW;
bool aux_ff0_20_nor_19 = LOW;
bool aux_ff0_20_nor_20 = LOW;
bool aux_ff0_20_node_21 = LOW;
bool aux_ff0_20_node_22 = LOW;
bool aux_ic_input_ff0_20_0 = LOW;
bool aux_ic_input_ff0_20_1 = LOW;
bool aux_ic_input_ff0_20_2 = LOW;
bool aux_ic_input_ff0_20_3 = LOW;
// End IC: FF0
// IC: FF1
bool aux_ff1_21_q = LOW;
bool aux_ff1_21_q_bar_1 = LOW;
bool aux_ff1_21_node_0 = LOW;
bool aux_ff1_21_node_1 = LOW;
bool aux_ff1_21_node_2 = LOW;
bool aux_ff1_21_node_3 = LOW;
bool aux_ff1_21_not_4 = LOW;
bool aux_ff1_21_not_5 = LOW;
bool aux_ff1_21_not_6 = LOW;
bool aux_ff1_21_or_7 = LOW;
bool aux_ff1_21_or_8 = LOW;
bool aux_ff1_21_or_9 = LOW;
bool aux_ff1_21_or_10 = LOW;
bool aux_ff1_21_not_11 = LOW;
bool aux_ff1_21_and_12 = LOW;
bool aux_ff1_21_and_13 = LOW;
bool aux_ff1_21_nor_14 = LOW;
bool aux_ff1_21_nor_15 = LOW;
bool aux_ff1_21_not_16 = LOW;
bool aux_ff1_21_and_17 = LOW;
bool aux_ff1_21_and_18 = LOW;
bool aux_ff1_21_nor_19 = LOW;
bool aux_ff1_21_nor_20 = LOW;
bool aux_ff1_21_node_21 = LOW;
bool aux_ff1_21_node_22 = LOW;
bool aux_ic_input_ff1_21_0 = LOW;
bool aux_ic_input_ff1_21_1 = LOW;
bool aux_ic_input_ff1_21_2 = LOW;
bool aux_ic_input_ff1_21_3 = LOW;
// End IC: FF1
// IC: FF2
bool aux_ff2_22_q = LOW;
bool aux_ff2_22_q_bar_1 = LOW;
bool aux_ff2_22_node_0 = LOW;
bool aux_ff2_22_node_1 = LOW;
bool aux_ff2_22_node_2 = LOW;
bool aux_ff2_22_node_3 = LOW;
bool aux_ff2_22_not_4 = LOW;
bool aux_ff2_22_not_5 = LOW;
bool aux_ff2_22_not_6 = LOW;
bool aux_ff2_22_or_7 = LOW;
bool aux_ff2_22_or_8 = LOW;
bool aux_ff2_22_or_9 = LOW;
bool aux_ff2_22_or_10 = LOW;
bool aux_ff2_22_not_11 = LOW;
bool aux_ff2_22_and_12 = LOW;
bool aux_ff2_22_and_13 = LOW;
bool aux_ff2_22_nor_14 = LOW;
bool aux_ff2_22_nor_15 = LOW;
bool aux_ff2_22_not_16 = LOW;
bool aux_ff2_22_and_17 = LOW;
bool aux_ff2_22_and_18 = LOW;
bool aux_ff2_22_nor_19 = LOW;
bool aux_ff2_22_nor_20 = LOW;
bool aux_ff2_22_node_21 = LOW;
bool aux_ff2_22_node_22 = LOW;
bool aux_ic_input_ff2_22_0 = LOW;
bool aux_ic_input_ff2_22_1 = LOW;
bool aux_ic_input_ff2_22_2 = LOW;
bool aux_ic_input_ff2_22_3 = LOW;
// End IC: FF2
// IC: FF3
bool aux_ff3_23_q = LOW;
bool aux_ff3_23_q_bar_1 = LOW;
bool aux_ff3_23_node_0 = LOW;
bool aux_ff3_23_node_1 = LOW;
bool aux_ff3_23_node_2 = LOW;
bool aux_ff3_23_node_3 = LOW;
bool aux_ff3_23_not_4 = LOW;
bool aux_ff3_23_not_5 = LOW;
bool aux_ff3_23_not_6 = LOW;
bool aux_ff3_23_or_7 = LOW;
bool aux_ff3_23_or_8 = LOW;
bool aux_ff3_23_or_9 = LOW;
bool aux_ff3_23_or_10 = LOW;
bool aux_ff3_23_not_11 = LOW;
bool aux_ff3_23_and_12 = LOW;
bool aux_ff3_23_and_13 = LOW;
bool aux_ff3_23_nor_14 = LOW;
bool aux_ff3_23_nor_15 = LOW;
bool aux_ff3_23_not_16 = LOW;
bool aux_ff3_23_and_17 = LOW;
bool aux_ff3_23_and_18 = LOW;
bool aux_ff3_23_nor_19 = LOW;
bool aux_ff3_23_nor_20 = LOW;
bool aux_ff3_23_node_21 = LOW;
bool aux_ff3_23_node_22 = LOW;
bool aux_ic_input_ff3_23_0 = LOW;
bool aux_ic_input_ff3_23_1 = LOW;
bool aux_ic_input_ff3_23_2 = LOW;
bool aux_ic_input_ff3_23_3 = LOW;
// End IC: FF3
bool aux_level4_binary_counter_4bit_0_node_24 = LOW;
bool aux_level4_binary_counter_4bit_0_node_25 = LOW;
bool aux_level4_binary_counter_4bit_0_node_26 = LOW;
bool aux_level4_binary_counter_4bit_0_node_27 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_0 = LOW;
// End IC: LEVEL4_BINARY_COUNTER_4BIT

void setup() {
    pinMode(input_switch1, INPUT);
    pinMode(led1_1, OUTPUT);
    pinMode(led2_1, OUTPUT);
    pinMode(led3_1, OUTPUT);
    pinMode(led4_1, OUTPUT);
}

void computeLogic() {
    // Assigning aux variables. //
    // IC: LEVEL4_BINARY_COUNTER_4BIT
    aux_ic_input_level4_binary_counter_4bit_0_0 = input_switch1_val;
    aux_level4_binary_counter_4bit_0_node_0 = aux_ic_input_level4_binary_counter_4bit_0_0;
    // IC: FF0
    aux_ic_input_ff0_20_0 = aux_level4_binary_counter_4bit_0_not_2;
    aux_ic_input_ff0_20_1 = aux_level4_binary_counter_4bit_0_node_0;
    aux_ic_input_ff0_20_2 = HIGH;
    aux_ic_input_ff0_20_3 = HIGH;
    aux_ff0_20_node_0 = aux_ic_input_ff0_20_0;
    aux_ff0_20_node_1 = aux_ic_input_ff0_20_1;
    aux_ff0_20_node_2 = aux_ic_input_ff0_20_2;
    aux_ff0_20_node_3 = aux_ic_input_ff0_20_3;
    aux_ff0_20_not_4 = !aux_ff0_20_node_1;
    aux_ff0_20_not_16 = !aux_ff0_20_nor_14;
    aux_ff0_20_not_5 = !aux_ff0_20_node_2;
    aux_ff0_20_not_6 = !aux_ff0_20_node_3;
    aux_ff0_20_not_11 = !aux_ff0_20_node_0;
    aux_ff0_20_and_12 = aux_ff0_20_node_0 && aux_ff0_20_not_4;
    aux_ff0_20_and_18 = aux_ff0_20_not_16 && aux_ff0_20_node_1;
    aux_ff0_20_or_7 = aux_ff0_20_and_18 || aux_ff0_20_not_6;
    aux_ff0_20_or_10 = aux_ff0_20_and_12 || aux_ff0_20_not_5;
    aux_ff0_20_and_13 = aux_ff0_20_not_11 && aux_ff0_20_not_4;
    aux_ff0_20_and_17 = aux_ff0_20_nor_14 && aux_ff0_20_node_1;
    aux_ff0_20_or_8 = aux_ff0_20_and_17 || aux_ff0_20_not_5;
    aux_ff0_20_or_9 = aux_ff0_20_and_13 || aux_ff0_20_not_6;
    aux_ff0_20_nor_15 = !(aux_ff0_20_or_10 || aux_ff0_20_nor_14);
    aux_ff0_20_nor_19 = !(aux_ff0_20_or_7 || aux_ff0_20_nor_20);
    aux_ff0_20_nor_14 = !(aux_ff0_20_or_9 || aux_ff0_20_nor_15);
    aux_ff0_20_nor_20 = !(aux_ff0_20_or_8 || aux_ff0_20_nor_19);
    aux_ff0_20_node_21 = aux_ff0_20_nor_19;
    aux_ff0_20_node_22 = aux_ff0_20_nor_20;
    aux_ff0_20_q = aux_ff0_20_node_21;
    aux_ff0_20_q_bar_1 = aux_ff0_20_node_22;
    // End IC: FF0
    // IC: FF1
    aux_ic_input_ff1_21_0 = aux_level4_binary_counter_4bit_0_or_17;
    aux_ic_input_ff1_21_1 = aux_level4_binary_counter_4bit_0_node_0;
    aux_ic_input_ff1_21_2 = HIGH;
    aux_ic_input_ff1_21_3 = HIGH;
    aux_ff1_21_node_0 = aux_ic_input_ff1_21_0;
    aux_ff1_21_node_1 = aux_ic_input_ff1_21_1;
    aux_ff1_21_node_2 = aux_ic_input_ff1_21_2;
    aux_ff1_21_node_3 = aux_ic_input_ff1_21_3;
    aux_ff1_21_not_4 = !aux_ff1_21_node_1;
    aux_ff1_21_not_16 = !aux_ff1_21_nor_14;
    aux_ff1_21_not_5 = !aux_ff1_21_node_2;
    aux_ff1_21_not_6 = !aux_ff1_21_node_3;
    aux_ff1_21_not_11 = !aux_ff1_21_node_0;
    aux_ff1_21_and_12 = aux_ff1_21_node_0 && aux_ff1_21_not_4;
    aux_ff1_21_and_18 = aux_ff1_21_not_16 && aux_ff1_21_node_1;
    aux_ff1_21_or_7 = aux_ff1_21_and_18 || aux_ff1_21_not_6;
    aux_ff1_21_or_10 = aux_ff1_21_and_12 || aux_ff1_21_not_5;
    aux_ff1_21_and_13 = aux_ff1_21_not_11 && aux_ff1_21_not_4;
    aux_ff1_21_and_17 = aux_ff1_21_nor_14 && aux_ff1_21_node_1;
    aux_ff1_21_or_8 = aux_ff1_21_and_17 || aux_ff1_21_not_5;
    aux_ff1_21_or_9 = aux_ff1_21_and_13 || aux_ff1_21_not_6;
    aux_ff1_21_nor_15 = !(aux_ff1_21_or_10 || aux_ff1_21_nor_14);
    aux_ff1_21_nor_19 = !(aux_ff1_21_or_7 || aux_ff1_21_nor_20);
    aux_ff1_21_nor_14 = !(aux_ff1_21_or_9 || aux_ff1_21_nor_15);
    aux_ff1_21_nor_20 = !(aux_ff1_21_or_8 || aux_ff1_21_nor_19);
    aux_ff1_21_node_21 = aux_ff1_21_nor_19;
    aux_ff1_21_node_22 = aux_ff1_21_nor_20;
    aux_ff1_21_q = aux_ff1_21_node_21;
    aux_ff1_21_q_bar_1 = aux_ff1_21_node_22;
    // End IC: FF1
    aux_level4_binary_counter_4bit_0_and_6 = aux_ff0_20_q && aux_ff1_21_q;
    // IC: FF2
    aux_ic_input_ff2_22_0 = aux_level4_binary_counter_4bit_0_or_18;
    aux_ic_input_ff2_22_1 = aux_level4_binary_counter_4bit_0_node_0;
    aux_ic_input_ff2_22_2 = HIGH;
    aux_ic_input_ff2_22_3 = HIGH;
    aux_ff2_22_node_0 = aux_ic_input_ff2_22_0;
    aux_ff2_22_node_1 = aux_ic_input_ff2_22_1;
    aux_ff2_22_node_2 = aux_ic_input_ff2_22_2;
    aux_ff2_22_node_3 = aux_ic_input_ff2_22_3;
    aux_ff2_22_not_4 = !aux_ff2_22_node_1;
    aux_ff2_22_not_16 = !aux_ff2_22_nor_14;
    aux_ff2_22_not_5 = !aux_ff2_22_node_2;
    aux_ff2_22_not_6 = !aux_ff2_22_node_3;
    aux_ff2_22_not_11 = !aux_ff2_22_node_0;
    aux_ff2_22_and_12 = aux_ff2_22_node_0 && aux_ff2_22_not_4;
    aux_ff2_22_and_18 = aux_ff2_22_not_16 && aux_ff2_22_node_1;
    aux_ff2_22_or_7 = aux_ff2_22_and_18 || aux_ff2_22_not_6;
    aux_ff2_22_or_10 = aux_ff2_22_and_12 || aux_ff2_22_not_5;
    aux_ff2_22_and_13 = aux_ff2_22_not_11 && aux_ff2_22_not_4;
    aux_ff2_22_and_17 = aux_ff2_22_nor_14 && aux_ff2_22_node_1;
    aux_ff2_22_or_8 = aux_ff2_22_and_17 || aux_ff2_22_not_5;
    aux_ff2_22_or_9 = aux_ff2_22_and_13 || aux_ff2_22_not_6;
    aux_ff2_22_nor_15 = !(aux_ff2_22_or_10 || aux_ff2_22_nor_14);
    aux_ff2_22_nor_19 = !(aux_ff2_22_or_7 || aux_ff2_22_nor_20);
    aux_ff2_22_nor_14 = !(aux_ff2_22_or_9 || aux_ff2_22_nor_15);
    aux_ff2_22_nor_20 = !(aux_ff2_22_or_8 || aux_ff2_22_nor_19);
    aux_ff2_22_node_21 = aux_ff2_22_nor_19;
    aux_ff2_22_node_22 = aux_ff2_22_nor_20;
    aux_ff2_22_q = aux_ff2_22_node_21;
    aux_ff2_22_q_bar_1 = aux_ff2_22_node_22;
    // End IC: FF2
    aux_level4_binary_counter_4bit_0_and_7 = aux_level4_binary_counter_4bit_0_and_6 && aux_ff2_22_q;
    // IC: FF3
    aux_ic_input_ff3_23_0 = aux_level4_binary_counter_4bit_0_or_19;
    aux_ic_input_ff3_23_1 = aux_level4_binary_counter_4bit_0_node_0;
    aux_ic_input_ff3_23_2 = HIGH;
    aux_ic_input_ff3_23_3 = HIGH;
    aux_ff3_23_node_0 = aux_ic_input_ff3_23_0;
    aux_ff3_23_node_1 = aux_ic_input_ff3_23_1;
    aux_ff3_23_node_2 = aux_ic_input_ff3_23_2;
    aux_ff3_23_node_3 = aux_ic_input_ff3_23_3;
    aux_ff3_23_not_4 = !aux_ff3_23_node_1;
    aux_ff3_23_not_16 = !aux_ff3_23_nor_14;
    aux_ff3_23_not_5 = !aux_ff3_23_node_2;
    aux_ff3_23_not_6 = !aux_ff3_23_node_3;
    aux_ff3_23_not_11 = !aux_ff3_23_node_0;
    aux_ff3_23_and_12 = aux_ff3_23_node_0 && aux_ff3_23_not_4;
    aux_ff3_23_and_18 = aux_ff3_23_not_16 && aux_ff3_23_node_1;
    aux_ff3_23_or_7 = aux_ff3_23_and_18 || aux_ff3_23_not_6;
    aux_ff3_23_or_10 = aux_ff3_23_and_12 || aux_ff3_23_not_5;
    aux_ff3_23_and_13 = aux_ff3_23_not_11 && aux_ff3_23_not_4;
    aux_ff3_23_and_17 = aux_ff3_23_nor_14 && aux_ff3_23_node_1;
    aux_ff3_23_or_8 = aux_ff3_23_and_17 || aux_ff3_23_not_5;
    aux_ff3_23_or_9 = aux_ff3_23_and_13 || aux_ff3_23_not_6;
    aux_ff3_23_nor_15 = !(aux_ff3_23_or_10 || aux_ff3_23_nor_14);
    aux_ff3_23_nor_19 = !(aux_ff3_23_or_7 || aux_ff3_23_nor_20);
    aux_ff3_23_nor_14 = !(aux_ff3_23_or_9 || aux_ff3_23_nor_15);
    aux_ff3_23_nor_20 = !(aux_ff3_23_or_8 || aux_ff3_23_nor_19);
    aux_ff3_23_node_21 = aux_ff3_23_nor_19;
    aux_ff3_23_node_22 = aux_ff3_23_nor_20;
    aux_ff3_23_q = aux_ff3_23_node_21;
    aux_ff3_23_q_bar_1 = aux_ff3_23_node_22;
    // End IC: FF3
    aux_level4_binary_counter_4bit_0_not_3 = !aux_ff1_21_q;
    aux_level4_binary_counter_4bit_0_not_4 = !aux_ff2_22_q;
    aux_level4_binary_counter_4bit_0_not_5 = !aux_ff3_23_q;
    aux_level4_binary_counter_4bit_0_not_8 = !aux_ff0_20_q;
    aux_level4_binary_counter_4bit_0_not_9 = !aux_level4_binary_counter_4bit_0_and_6;
    aux_level4_binary_counter_4bit_0_not_10 = !aux_level4_binary_counter_4bit_0_and_7;
    aux_level4_binary_counter_4bit_0_and_11 = aux_level4_binary_counter_4bit_0_not_3 && aux_ff0_20_q;
    aux_level4_binary_counter_4bit_0_and_12 = aux_ff1_21_q && aux_level4_binary_counter_4bit_0_not_8;
    aux_level4_binary_counter_4bit_0_and_13 = aux_level4_binary_counter_4bit_0_not_4 && aux_level4_binary_counter_4bit_0_and_6;
    aux_level4_binary_counter_4bit_0_and_14 = aux_ff2_22_q && aux_level4_binary_counter_4bit_0_not_9;
    aux_level4_binary_counter_4bit_0_and_15 = aux_level4_binary_counter_4bit_0_not_5 && aux_level4_binary_counter_4bit_0_and_7;
    aux_level4_binary_counter_4bit_0_and_16 = aux_ff3_23_q && aux_level4_binary_counter_4bit_0_not_10;
    aux_level4_binary_counter_4bit_0_not_2 = !aux_ff0_20_q;
    aux_level4_binary_counter_4bit_0_or_17 = aux_level4_binary_counter_4bit_0_and_11 || aux_level4_binary_counter_4bit_0_and_12;
    aux_level4_binary_counter_4bit_0_or_18 = aux_level4_binary_counter_4bit_0_and_13 || aux_level4_binary_counter_4bit_0_and_14;
    aux_level4_binary_counter_4bit_0_or_19 = aux_level4_binary_counter_4bit_0_and_15 || aux_level4_binary_counter_4bit_0_and_16;
    aux_level4_binary_counter_4bit_0_node_24 = aux_ff0_20_q;
    aux_level4_binary_counter_4bit_0_node_25 = aux_ff1_21_q;
    aux_level4_binary_counter_4bit_0_node_26 = aux_ff2_22_q;
    aux_level4_binary_counter_4bit_0_node_27 = aux_ff3_23_q;
    aux_level4_binary_counter_4bit_0_q0 = aux_level4_binary_counter_4bit_0_node_24;
    aux_level4_binary_counter_4bit_0_q1_1 = aux_level4_binary_counter_4bit_0_node_25;
    aux_level4_binary_counter_4bit_0_q2_2 = aux_level4_binary_counter_4bit_0_node_26;
    aux_level4_binary_counter_4bit_0_q3_3 = aux_level4_binary_counter_4bit_0_node_27;
    // End IC: LEVEL4_BINARY_COUNTER_4BIT
}

void loop() {
    // Reading input data //.
    input_switch1_val = digitalRead(input_switch1);

    // Updating clocks. //

    computeLogic();

    // Writing output data. //
    digitalWrite(led1_1, aux_level4_binary_counter_4bit_0_q0);
    digitalWrite(led2_1, aux_level4_binary_counter_4bit_0_q1_1);
    digitalWrite(led3_1, aux_level4_binary_counter_4bit_0_q2_2);
    digitalWrite(led4_1, aux_level4_binary_counter_4bit_0_q3_3);
}
