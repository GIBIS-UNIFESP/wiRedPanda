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
bool aux_level4_binary_counter_4bit_0_and_17 = LOW;
bool aux_level4_binary_counter_4bit_0_and_18 = LOW;
bool aux_level4_binary_counter_4bit_0_or_19 = LOW;
bool aux_level4_binary_counter_4bit_0_or_20 = LOW;
bool aux_level4_binary_counter_4bit_0_or_21 = LOW;
bool aux_level4_binary_counter_4bit_0_or_22 = LOW;
// IC: FF0
bool aux_ff0_23_q = LOW;
bool aux_ff0_23_q_bar_1 = LOW;
bool aux_ff0_23_node_0 = LOW;
bool aux_ff0_23_node_1 = LOW;
bool aux_ff0_23_node_2 = LOW;
bool aux_ff0_23_node_3 = LOW;
bool aux_ff0_23_not_4 = LOW;
bool aux_ff0_23_not_5 = LOW;
bool aux_ff0_23_not_6 = LOW;
bool aux_ff0_23_or_7 = LOW;
bool aux_ff0_23_or_8 = LOW;
bool aux_ff0_23_not_9 = LOW;
bool aux_ff0_23_and_10 = LOW;
bool aux_ff0_23_and_11 = LOW;
bool aux_ff0_23_nor_12 = LOW;
bool aux_ff0_23_nor_13 = LOW;
bool aux_ff0_23_not_14 = LOW;
bool aux_ff0_23_and_15 = LOW;
bool aux_ff0_23_and_16 = LOW;
bool aux_ff0_23_nor_17 = LOW;
bool aux_ff0_23_nor_18 = LOW;
bool aux_ff0_23_node_19 = LOW;
bool aux_ff0_23_node_20 = LOW;
bool aux_ic_input_ff0_0 = LOW;
bool aux_ic_input_ff0_1 = LOW;
bool aux_ic_input_ff0_2 = LOW;
bool aux_ic_input_ff0_3 = LOW;
// End IC: FF0
// IC: FF1
bool aux_ff1_24_q = LOW;
bool aux_ff1_24_q_bar_1 = LOW;
bool aux_ff1_24_node_0 = LOW;
bool aux_ff1_24_node_1 = LOW;
bool aux_ff1_24_node_2 = LOW;
bool aux_ff1_24_node_3 = LOW;
bool aux_ff1_24_not_4 = LOW;
bool aux_ff1_24_not_5 = LOW;
bool aux_ff1_24_not_6 = LOW;
bool aux_ff1_24_or_7 = LOW;
bool aux_ff1_24_or_8 = LOW;
bool aux_ff1_24_not_9 = LOW;
bool aux_ff1_24_and_10 = LOW;
bool aux_ff1_24_and_11 = LOW;
bool aux_ff1_24_nor_12 = LOW;
bool aux_ff1_24_nor_13 = LOW;
bool aux_ff1_24_not_14 = LOW;
bool aux_ff1_24_and_15 = LOW;
bool aux_ff1_24_and_16 = LOW;
bool aux_ff1_24_nor_17 = LOW;
bool aux_ff1_24_nor_18 = LOW;
bool aux_ff1_24_node_19 = LOW;
bool aux_ff1_24_node_20 = LOW;
bool aux_ic_input_ff1_0 = LOW;
bool aux_ic_input_ff1_1 = LOW;
bool aux_ic_input_ff1_2 = LOW;
bool aux_ic_input_ff1_3 = LOW;
// End IC: FF1
// IC: FF2
bool aux_ff2_25_q = LOW;
bool aux_ff2_25_q_bar_1 = LOW;
bool aux_ff2_25_node_0 = LOW;
bool aux_ff2_25_node_1 = LOW;
bool aux_ff2_25_node_2 = LOW;
bool aux_ff2_25_node_3 = LOW;
bool aux_ff2_25_not_4 = LOW;
bool aux_ff2_25_not_5 = LOW;
bool aux_ff2_25_not_6 = LOW;
bool aux_ff2_25_or_7 = LOW;
bool aux_ff2_25_or_8 = LOW;
bool aux_ff2_25_not_9 = LOW;
bool aux_ff2_25_and_10 = LOW;
bool aux_ff2_25_and_11 = LOW;
bool aux_ff2_25_nor_12 = LOW;
bool aux_ff2_25_nor_13 = LOW;
bool aux_ff2_25_not_14 = LOW;
bool aux_ff2_25_and_15 = LOW;
bool aux_ff2_25_and_16 = LOW;
bool aux_ff2_25_nor_17 = LOW;
bool aux_ff2_25_nor_18 = LOW;
bool aux_ff2_25_node_19 = LOW;
bool aux_ff2_25_node_20 = LOW;
bool aux_ic_input_ff2_0 = LOW;
bool aux_ic_input_ff2_1 = LOW;
bool aux_ic_input_ff2_2 = LOW;
bool aux_ic_input_ff2_3 = LOW;
// End IC: FF2
// IC: FF3
bool aux_ff3_26_q = LOW;
bool aux_ff3_26_q_bar_1 = LOW;
bool aux_ff3_26_node_0 = LOW;
bool aux_ff3_26_node_1 = LOW;
bool aux_ff3_26_node_2 = LOW;
bool aux_ff3_26_node_3 = LOW;
bool aux_ff3_26_not_4 = LOW;
bool aux_ff3_26_not_5 = LOW;
bool aux_ff3_26_not_6 = LOW;
bool aux_ff3_26_or_7 = LOW;
bool aux_ff3_26_or_8 = LOW;
bool aux_ff3_26_not_9 = LOW;
bool aux_ff3_26_and_10 = LOW;
bool aux_ff3_26_and_11 = LOW;
bool aux_ff3_26_nor_12 = LOW;
bool aux_ff3_26_nor_13 = LOW;
bool aux_ff3_26_not_14 = LOW;
bool aux_ff3_26_and_15 = LOW;
bool aux_ff3_26_and_16 = LOW;
bool aux_ff3_26_nor_17 = LOW;
bool aux_ff3_26_nor_18 = LOW;
bool aux_ff3_26_node_19 = LOW;
bool aux_ff3_26_node_20 = LOW;
bool aux_ic_input_ff3_0 = LOW;
bool aux_ic_input_ff3_1 = LOW;
bool aux_ic_input_ff3_2 = LOW;
bool aux_ic_input_ff3_3 = LOW;
// End IC: FF3
bool aux_level4_binary_counter_4bit_0_node_27 = LOW;
bool aux_level4_binary_counter_4bit_0_node_28 = LOW;
bool aux_level4_binary_counter_4bit_0_node_29 = LOW;
bool aux_level4_binary_counter_4bit_0_node_30 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0 = LOW;
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
    aux_ic_input_level4_binary_counter_4bit_0 = input_switch1_val;
    aux_level4_binary_counter_4bit_0_node_0 = aux_ic_input_level4_binary_counter_4bit_0;
    // IC: FF0
    aux_ic_input_ff0_0 = aux_level4_binary_counter_4bit_0_not_2;
    aux_ic_input_ff0_1 = aux_level4_binary_counter_4bit_0_node_0;
    aux_ic_input_ff0_2 = HIGH;
    aux_ic_input_ff0_3 = HIGH;
    aux_ff0_23_node_1 = aux_ic_input_ff0_1;
    aux_ff0_23_node_3 = aux_ic_input_ff0_3;
    aux_ff0_23_not_14 = !aux_ff0_23_nor_12;
    aux_ff0_23_node_0 = aux_ic_input_ff0_0;
    aux_ff0_23_node_2 = aux_ic_input_ff0_2;
    aux_ff0_23_not_4 = !aux_ff0_23_node_1;
    aux_ff0_23_not_6 = !aux_ff0_23_node_3;
    aux_ff0_23_and_16 = aux_ff0_23_not_14 && aux_ff0_23_node_1;
    aux_ff0_23_not_5 = !aux_ff0_23_node_2;
    aux_ff0_23_or_7 = aux_ff0_23_and_16 || aux_ff0_23_not_6;
    aux_ff0_23_not_9 = !aux_ff0_23_node_0;
    aux_ff0_23_and_10 = aux_ff0_23_node_0 && aux_ff0_23_not_4;
    aux_ff0_23_and_15 = aux_ff0_23_nor_12 && aux_ff0_23_node_1;
    aux_ff0_23_or_8 = aux_ff0_23_and_15 || aux_ff0_23_not_5;
    aux_ff0_23_and_11 = aux_ff0_23_not_9 && aux_ff0_23_not_4;
    aux_ff0_23_nor_13 = !(aux_ff0_23_and_10 || aux_ff0_23_nor_12);
    aux_ff0_23_nor_17 = !(aux_ff0_23_or_7 || aux_ff0_23_nor_18);
    aux_ff0_23_nor_12 = !(aux_ff0_23_and_11 || aux_ff0_23_nor_13);
    aux_ff0_23_nor_18 = !(aux_ff0_23_or_8 || aux_ff0_23_nor_17);
    aux_ff0_23_node_19 = aux_ff0_23_nor_17;
    aux_ff0_23_node_20 = aux_ff0_23_nor_18;
    aux_ff0_23_q = aux_ff0_23_node_19;
    aux_ff0_23_q_bar_1 = aux_ff0_23_node_20;
    // End IC: FF0
    // IC: FF1
    aux_ic_input_ff1_0 = aux_level4_binary_counter_4bit_0_or_20;
    aux_ic_input_ff1_1 = aux_level4_binary_counter_4bit_0_node_0;
    aux_ic_input_ff1_2 = HIGH;
    aux_ic_input_ff1_3 = HIGH;
    aux_ff1_24_node_1 = aux_ic_input_ff1_1;
    aux_ff1_24_node_3 = aux_ic_input_ff1_3;
    aux_ff1_24_not_14 = !aux_ff1_24_nor_12;
    aux_ff1_24_node_0 = aux_ic_input_ff1_0;
    aux_ff1_24_node_2 = aux_ic_input_ff1_2;
    aux_ff1_24_not_4 = !aux_ff1_24_node_1;
    aux_ff1_24_not_6 = !aux_ff1_24_node_3;
    aux_ff1_24_and_16 = aux_ff1_24_not_14 && aux_ff1_24_node_1;
    aux_ff1_24_not_5 = !aux_ff1_24_node_2;
    aux_ff1_24_or_7 = aux_ff1_24_and_16 || aux_ff1_24_not_6;
    aux_ff1_24_not_9 = !aux_ff1_24_node_0;
    aux_ff1_24_and_10 = aux_ff1_24_node_0 && aux_ff1_24_not_4;
    aux_ff1_24_and_15 = aux_ff1_24_nor_12 && aux_ff1_24_node_1;
    aux_ff1_24_or_8 = aux_ff1_24_and_15 || aux_ff1_24_not_5;
    aux_ff1_24_and_11 = aux_ff1_24_not_9 && aux_ff1_24_not_4;
    aux_ff1_24_nor_13 = !(aux_ff1_24_and_10 || aux_ff1_24_nor_12);
    aux_ff1_24_nor_17 = !(aux_ff1_24_or_7 || aux_ff1_24_nor_18);
    aux_ff1_24_nor_12 = !(aux_ff1_24_and_11 || aux_ff1_24_nor_13);
    aux_ff1_24_nor_18 = !(aux_ff1_24_or_8 || aux_ff1_24_nor_17);
    aux_ff1_24_node_19 = aux_ff1_24_nor_17;
    aux_ff1_24_node_20 = aux_ff1_24_nor_18;
    aux_ff1_24_q = aux_ff1_24_node_19;
    aux_ff1_24_q_bar_1 = aux_ff1_24_node_20;
    // End IC: FF1
    aux_level4_binary_counter_4bit_0_and_6 = aux_ff0_23_q && aux_ff1_24_q;
    // IC: FF2
    aux_ic_input_ff2_0 = aux_level4_binary_counter_4bit_0_or_21;
    aux_ic_input_ff2_1 = aux_level4_binary_counter_4bit_0_node_0;
    aux_ic_input_ff2_2 = HIGH;
    aux_ic_input_ff2_3 = HIGH;
    aux_ff2_25_node_1 = aux_ic_input_ff2_1;
    aux_ff2_25_node_3 = aux_ic_input_ff2_3;
    aux_ff2_25_not_14 = !aux_ff2_25_nor_12;
    aux_ff2_25_node_0 = aux_ic_input_ff2_0;
    aux_ff2_25_node_2 = aux_ic_input_ff2_2;
    aux_ff2_25_not_4 = !aux_ff2_25_node_1;
    aux_ff2_25_not_6 = !aux_ff2_25_node_3;
    aux_ff2_25_and_16 = aux_ff2_25_not_14 && aux_ff2_25_node_1;
    aux_ff2_25_not_5 = !aux_ff2_25_node_2;
    aux_ff2_25_or_7 = aux_ff2_25_and_16 || aux_ff2_25_not_6;
    aux_ff2_25_not_9 = !aux_ff2_25_node_0;
    aux_ff2_25_and_10 = aux_ff2_25_node_0 && aux_ff2_25_not_4;
    aux_ff2_25_and_15 = aux_ff2_25_nor_12 && aux_ff2_25_node_1;
    aux_ff2_25_or_8 = aux_ff2_25_and_15 || aux_ff2_25_not_5;
    aux_ff2_25_and_11 = aux_ff2_25_not_9 && aux_ff2_25_not_4;
    aux_ff2_25_nor_13 = !(aux_ff2_25_and_10 || aux_ff2_25_nor_12);
    aux_ff2_25_nor_17 = !(aux_ff2_25_or_7 || aux_ff2_25_nor_18);
    aux_ff2_25_nor_12 = !(aux_ff2_25_and_11 || aux_ff2_25_nor_13);
    aux_ff2_25_nor_18 = !(aux_ff2_25_or_8 || aux_ff2_25_nor_17);
    aux_ff2_25_node_19 = aux_ff2_25_nor_17;
    aux_ff2_25_node_20 = aux_ff2_25_nor_18;
    aux_ff2_25_q = aux_ff2_25_node_19;
    aux_ff2_25_q_bar_1 = aux_ff2_25_node_20;
    // End IC: FF2
    aux_level4_binary_counter_4bit_0_and_7 = aux_level4_binary_counter_4bit_0_and_6 && aux_ff2_25_q;
    // IC: FF3
    aux_ic_input_ff3_0 = aux_level4_binary_counter_4bit_0_or_22;
    aux_ic_input_ff3_1 = aux_level4_binary_counter_4bit_0_node_0;
    aux_ic_input_ff3_2 = HIGH;
    aux_ic_input_ff3_3 = HIGH;
    aux_ff3_26_node_1 = aux_ic_input_ff3_1;
    aux_ff3_26_node_3 = aux_ic_input_ff3_3;
    aux_ff3_26_not_14 = !aux_ff3_26_nor_12;
    aux_ff3_26_node_0 = aux_ic_input_ff3_0;
    aux_ff3_26_node_2 = aux_ic_input_ff3_2;
    aux_ff3_26_not_4 = !aux_ff3_26_node_1;
    aux_ff3_26_not_6 = !aux_ff3_26_node_3;
    aux_ff3_26_and_16 = aux_ff3_26_not_14 && aux_ff3_26_node_1;
    aux_ff3_26_not_5 = !aux_ff3_26_node_2;
    aux_ff3_26_or_7 = aux_ff3_26_and_16 || aux_ff3_26_not_6;
    aux_ff3_26_not_9 = !aux_ff3_26_node_0;
    aux_ff3_26_and_10 = aux_ff3_26_node_0 && aux_ff3_26_not_4;
    aux_ff3_26_and_15 = aux_ff3_26_nor_12 && aux_ff3_26_node_1;
    aux_ff3_26_or_8 = aux_ff3_26_and_15 || aux_ff3_26_not_5;
    aux_ff3_26_and_11 = aux_ff3_26_not_9 && aux_ff3_26_not_4;
    aux_ff3_26_nor_13 = !(aux_ff3_26_and_10 || aux_ff3_26_nor_12);
    aux_ff3_26_nor_17 = !(aux_ff3_26_or_7 || aux_ff3_26_nor_18);
    aux_ff3_26_nor_12 = !(aux_ff3_26_and_11 || aux_ff3_26_nor_13);
    aux_ff3_26_nor_18 = !(aux_ff3_26_or_8 || aux_ff3_26_nor_17);
    aux_ff3_26_node_19 = aux_ff3_26_nor_17;
    aux_ff3_26_node_20 = aux_ff3_26_nor_18;
    aux_ff3_26_q = aux_ff3_26_node_19;
    aux_ff3_26_q_bar_1 = aux_ff3_26_node_20;
    // End IC: FF3
    aux_level4_binary_counter_4bit_0_not_3 = !aux_ff1_24_q;
    aux_level4_binary_counter_4bit_0_not_4 = !aux_ff2_25_q;
    aux_level4_binary_counter_4bit_0_not_5 = !aux_ff3_26_q;
    aux_level4_binary_counter_4bit_0_not_8 = !aux_ff0_23_q;
    aux_level4_binary_counter_4bit_0_not_9 = !aux_level4_binary_counter_4bit_0_and_6;
    aux_level4_binary_counter_4bit_0_not_10 = !aux_level4_binary_counter_4bit_0_and_7;
    aux_level4_binary_counter_4bit_0_and_13 = aux_level4_binary_counter_4bit_0_not_3 && aux_ff0_23_q;
    aux_level4_binary_counter_4bit_0_and_14 = aux_ff1_24_q && aux_level4_binary_counter_4bit_0_not_8;
    aux_level4_binary_counter_4bit_0_and_15 = aux_level4_binary_counter_4bit_0_not_4 && aux_level4_binary_counter_4bit_0_and_6;
    aux_level4_binary_counter_4bit_0_and_16 = aux_ff2_25_q && aux_level4_binary_counter_4bit_0_not_9;
    aux_level4_binary_counter_4bit_0_and_17 = aux_level4_binary_counter_4bit_0_not_5 && aux_level4_binary_counter_4bit_0_and_7;
    aux_level4_binary_counter_4bit_0_and_18 = aux_ff3_26_q && aux_level4_binary_counter_4bit_0_not_10;
    aux_level4_binary_counter_4bit_0_not_2 = !aux_ff0_23_q;
    aux_level4_binary_counter_4bit_0_and_11 = LOW && LOW;
    aux_level4_binary_counter_4bit_0_and_12 = LOW && LOW;
    aux_level4_binary_counter_4bit_0_or_19 = LOW || LOW;
    aux_level4_binary_counter_4bit_0_or_20 = aux_level4_binary_counter_4bit_0_and_13 || aux_level4_binary_counter_4bit_0_and_14;
    aux_level4_binary_counter_4bit_0_or_21 = aux_level4_binary_counter_4bit_0_and_15 || aux_level4_binary_counter_4bit_0_and_16;
    aux_level4_binary_counter_4bit_0_or_22 = aux_level4_binary_counter_4bit_0_and_17 || aux_level4_binary_counter_4bit_0_and_18;
    aux_level4_binary_counter_4bit_0_node_27 = aux_ff0_23_q;
    aux_level4_binary_counter_4bit_0_node_28 = aux_ff1_24_q;
    aux_level4_binary_counter_4bit_0_node_29 = aux_ff2_25_q;
    aux_level4_binary_counter_4bit_0_node_30 = aux_ff3_26_q;
    aux_level4_binary_counter_4bit_0_q0 = aux_level4_binary_counter_4bit_0_node_27;
    aux_level4_binary_counter_4bit_0_q1_1 = aux_level4_binary_counter_4bit_0_node_28;
    aux_level4_binary_counter_4bit_0_q2_2 = aux_level4_binary_counter_4bit_0_node_29;
    aux_level4_binary_counter_4bit_0_q3_3 = aux_level4_binary_counter_4bit_0_node_30;
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
