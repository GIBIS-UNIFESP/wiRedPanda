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

/* ========= Outputs ========== */
const int led1_0 = A2;
const int led2_0 = A3;
const int led3_0 = A4;
const int led4_0 = A5;

/* ====== Aux. Variables ====== */
bool input_switch1_val = LOW;
bool input_switch2_val = LOW;
// IC: LEVEL4_BINARY_COUNTER_4BIT
bool aux_level4_binary_counter_4bit_0_q0 = LOW;
bool aux_level4_binary_counter_4bit_0_q1_1 = LOW;
bool aux_level4_binary_counter_4bit_0_q2_2 = LOW;
bool aux_level4_binary_counter_4bit_0_q3_3 = LOW;
bool aux_level4_binary_counter_4bit_0_node_0 = LOW;
bool aux_level4_binary_counter_4bit_0_node_1 = LOW;
bool aux_level4_binary_counter_4bit_0_not_3 = LOW;
bool aux_level4_binary_counter_4bit_0_not_4 = LOW;
bool aux_level4_binary_counter_4bit_0_not_5 = LOW;
bool aux_level4_binary_counter_4bit_0_not_6 = LOW;
bool aux_level4_binary_counter_4bit_0_and_7 = LOW;
bool aux_level4_binary_counter_4bit_0_and_8 = LOW;
bool aux_level4_binary_counter_4bit_0_not_9 = LOW;
bool aux_level4_binary_counter_4bit_0_not_10 = LOW;
bool aux_level4_binary_counter_4bit_0_not_11 = LOW;
bool aux_level4_binary_counter_4bit_0_and_12 = LOW;
bool aux_level4_binary_counter_4bit_0_and_13 = LOW;
bool aux_level4_binary_counter_4bit_0_and_14 = LOW;
bool aux_level4_binary_counter_4bit_0_and_15 = LOW;
bool aux_level4_binary_counter_4bit_0_and_16 = LOW;
bool aux_level4_binary_counter_4bit_0_and_17 = LOW;
bool aux_level4_binary_counter_4bit_0_and_18 = LOW;
bool aux_level4_binary_counter_4bit_0_and_19 = LOW;
bool aux_level4_binary_counter_4bit_0_or_20 = LOW;
bool aux_level4_binary_counter_4bit_0_or_21 = LOW;
bool aux_level4_binary_counter_4bit_0_or_22 = LOW;
bool aux_level4_binary_counter_4bit_0_or_23 = LOW;
// IC: FF0
bool aux_ff0_24_q = LOW;
bool aux_ff0_24_q_bar_1 = LOW;
bool aux_ff0_24_node_0 = LOW;
bool aux_ff0_24_node_1 = LOW;
bool aux_ff0_24_node_2 = LOW;
bool aux_ff0_24_node_3 = LOW;
bool aux_ff0_24_not_4 = LOW;
bool aux_ff0_24_not_5 = LOW;
bool aux_ff0_24_not_6 = LOW;
bool aux_ff0_24_not_7 = LOW;
bool aux_ff0_24_and_8 = LOW;
bool aux_ff0_24_and_9 = LOW;
bool aux_ff0_24_nor_10 = LOW;
bool aux_ff0_24_nor_11 = LOW;
bool aux_ff0_24_and_12 = LOW;
bool aux_ff0_24_and_13 = LOW;
bool aux_ff0_24_or_14 = LOW;
bool aux_ff0_24_or_15 = LOW;
bool aux_ff0_24_nor_16 = LOW;
bool aux_ff0_24_nor_17 = LOW;
bool aux_ff0_24_node_18 = LOW;
bool aux_ff0_24_node_19 = LOW;
bool aux_ic_input_ff0_0 = LOW;
bool aux_ic_input_ff0_1 = LOW;
bool aux_ic_input_ff0_2 = LOW;
bool aux_ic_input_ff0_3 = LOW;
// End IC: FF0
// IC: FF1
bool aux_ff1_25_q = LOW;
bool aux_ff1_25_q_bar_1 = LOW;
bool aux_ff1_25_node_0 = LOW;
bool aux_ff1_25_node_1 = LOW;
bool aux_ff1_25_node_2 = LOW;
bool aux_ff1_25_node_3 = LOW;
bool aux_ff1_25_not_4 = LOW;
bool aux_ff1_25_not_5 = LOW;
bool aux_ff1_25_not_6 = LOW;
bool aux_ff1_25_not_7 = LOW;
bool aux_ff1_25_and_8 = LOW;
bool aux_ff1_25_and_9 = LOW;
bool aux_ff1_25_nor_10 = LOW;
bool aux_ff1_25_nor_11 = LOW;
bool aux_ff1_25_and_12 = LOW;
bool aux_ff1_25_and_13 = LOW;
bool aux_ff1_25_or_14 = LOW;
bool aux_ff1_25_or_15 = LOW;
bool aux_ff1_25_nor_16 = LOW;
bool aux_ff1_25_nor_17 = LOW;
bool aux_ff1_25_node_18 = LOW;
bool aux_ff1_25_node_19 = LOW;
bool aux_ic_input_ff1_0 = LOW;
bool aux_ic_input_ff1_1 = LOW;
bool aux_ic_input_ff1_2 = LOW;
bool aux_ic_input_ff1_3 = LOW;
// End IC: FF1
// IC: FF2
bool aux_ff2_26_q = LOW;
bool aux_ff2_26_q_bar_1 = LOW;
bool aux_ff2_26_node_0 = LOW;
bool aux_ff2_26_node_1 = LOW;
bool aux_ff2_26_node_2 = LOW;
bool aux_ff2_26_node_3 = LOW;
bool aux_ff2_26_not_4 = LOW;
bool aux_ff2_26_not_5 = LOW;
bool aux_ff2_26_not_6 = LOW;
bool aux_ff2_26_not_7 = LOW;
bool aux_ff2_26_and_8 = LOW;
bool aux_ff2_26_and_9 = LOW;
bool aux_ff2_26_nor_10 = LOW;
bool aux_ff2_26_nor_11 = LOW;
bool aux_ff2_26_and_12 = LOW;
bool aux_ff2_26_and_13 = LOW;
bool aux_ff2_26_or_14 = LOW;
bool aux_ff2_26_or_15 = LOW;
bool aux_ff2_26_nor_16 = LOW;
bool aux_ff2_26_nor_17 = LOW;
bool aux_ff2_26_node_18 = LOW;
bool aux_ff2_26_node_19 = LOW;
bool aux_ic_input_ff2_0 = LOW;
bool aux_ic_input_ff2_1 = LOW;
bool aux_ic_input_ff2_2 = LOW;
bool aux_ic_input_ff2_3 = LOW;
// End IC: FF2
// IC: FF3
bool aux_ff3_27_q = LOW;
bool aux_ff3_27_q_bar_1 = LOW;
bool aux_ff3_27_node_0 = LOW;
bool aux_ff3_27_node_1 = LOW;
bool aux_ff3_27_node_2 = LOW;
bool aux_ff3_27_node_3 = LOW;
bool aux_ff3_27_not_4 = LOW;
bool aux_ff3_27_not_5 = LOW;
bool aux_ff3_27_not_6 = LOW;
bool aux_ff3_27_not_7 = LOW;
bool aux_ff3_27_and_8 = LOW;
bool aux_ff3_27_and_9 = LOW;
bool aux_ff3_27_nor_10 = LOW;
bool aux_ff3_27_nor_11 = LOW;
bool aux_ff3_27_and_12 = LOW;
bool aux_ff3_27_and_13 = LOW;
bool aux_ff3_27_or_14 = LOW;
bool aux_ff3_27_or_15 = LOW;
bool aux_ff3_27_nor_16 = LOW;
bool aux_ff3_27_nor_17 = LOW;
bool aux_ff3_27_node_18 = LOW;
bool aux_ff3_27_node_19 = LOW;
bool aux_ic_input_ff3_0 = LOW;
bool aux_ic_input_ff3_1 = LOW;
bool aux_ic_input_ff3_2 = LOW;
bool aux_ic_input_ff3_3 = LOW;
// End IC: FF3
bool aux_level4_binary_counter_4bit_0_node_28 = LOW;
bool aux_level4_binary_counter_4bit_0_node_29 = LOW;
bool aux_level4_binary_counter_4bit_0_node_30 = LOW;
bool aux_level4_binary_counter_4bit_0_node_31 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_1 = LOW;
// End IC: LEVEL4_BINARY_COUNTER_4BIT

void setup() {
    pinMode(input_switch1, INPUT);
    pinMode(input_switch2, INPUT);
    pinMode(led1_0, OUTPUT);
    pinMode(led2_0, OUTPUT);
    pinMode(led3_0, OUTPUT);
    pinMode(led4_0, OUTPUT);
}

void computeLogic() {
    // Assigning aux variables. //
    // IC: LEVEL4_BINARY_COUNTER_4BIT
    aux_ic_input_level4_binary_counter_4bit_0 = input_switch1_val;
    aux_ic_input_level4_binary_counter_4bit_1 = input_switch2_val;
    aux_level4_binary_counter_4bit_0_node_0 = aux_ic_input_level4_binary_counter_4bit_0;
    aux_level4_binary_counter_4bit_0_node_1 = aux_ic_input_level4_binary_counter_4bit_1;
    // IC: FF0
    aux_ic_input_ff0_0 = aux_level4_binary_counter_4bit_0_not_3;
    aux_ic_input_ff0_1 = aux_level4_binary_counter_4bit_0_node_0;
    aux_ic_input_ff0_2 = HIGH;
    aux_ic_input_ff0_3 = aux_level4_binary_counter_4bit_0_node_1;
    aux_ff0_24_node_0 = aux_ic_input_ff0_0;
    aux_ff0_24_node_2 = aux_ic_input_ff0_2;
    aux_ff0_24_not_5 = !aux_ff0_24_node_1;
    aux_ff0_24_not_6 = !aux_ff0_24_node_2;
    aux_ff0_24_and_8 = aux_ff0_24_node_0 && aux_ff0_24_not_5;
    aux_ff0_24_node_1 = aux_ic_input_ff0_1;
    aux_ff0_24_node_3 = aux_ic_input_ff0_3;
    aux_ff0_24_nor_11 = !(aux_ff0_24_and_8 || aux_ff0_24_nor_10 || aux_ff0_24_not_6);
    aux_ff0_24_not_7 = !aux_ff0_24_node_3;
    aux_ff0_24_and_13 = aux_ff0_24_nor_11 && aux_ff0_24_node_1;
    aux_ff0_24_not_4 = !aux_ff0_24_node_0;
    aux_ff0_24_and_12 = aux_ff0_24_nor_10 && aux_ff0_24_node_1;
    aux_ff0_24_or_15 = aux_ff0_24_and_13 || aux_ff0_24_not_7;
    aux_ff0_24_and_9 = aux_ff0_24_not_4 && aux_ff0_24_not_5;
    aux_ff0_24_or_14 = aux_ff0_24_and_12 || aux_ff0_24_not_6;
    aux_ff0_24_nor_16 = !(aux_ff0_24_or_15 || aux_ff0_24_nor_17);
    aux_ff0_24_nor_10 = !(aux_ff0_24_and_9 || aux_ff0_24_nor_11 || aux_ff0_24_not_7);
    aux_ff0_24_nor_17 = !(aux_ff0_24_or_14 || aux_ff0_24_nor_16);
    aux_ff0_24_node_18 = aux_ff0_24_nor_16;
    aux_ff0_24_node_19 = aux_ff0_24_nor_17;
    aux_ff0_24_q = aux_ff0_24_node_18;
    aux_ff0_24_q_bar_1 = aux_ff0_24_node_19;
    // End IC: FF0
    // IC: FF1
    aux_ic_input_ff1_0 = aux_level4_binary_counter_4bit_0_or_21;
    aux_ic_input_ff1_1 = aux_level4_binary_counter_4bit_0_node_0;
    aux_ic_input_ff1_2 = HIGH;
    aux_ic_input_ff1_3 = aux_level4_binary_counter_4bit_0_node_1;
    aux_ff1_25_node_0 = aux_ic_input_ff1_0;
    aux_ff1_25_node_2 = aux_ic_input_ff1_2;
    aux_ff1_25_not_5 = !aux_ff1_25_node_1;
    aux_ff1_25_not_6 = !aux_ff1_25_node_2;
    aux_ff1_25_and_8 = aux_ff1_25_node_0 && aux_ff1_25_not_5;
    aux_ff1_25_node_1 = aux_ic_input_ff1_1;
    aux_ff1_25_node_3 = aux_ic_input_ff1_3;
    aux_ff1_25_nor_11 = !(aux_ff1_25_and_8 || aux_ff1_25_nor_10 || aux_ff1_25_not_6);
    aux_ff1_25_not_7 = !aux_ff1_25_node_3;
    aux_ff1_25_and_13 = aux_ff1_25_nor_11 && aux_ff1_25_node_1;
    aux_ff1_25_not_4 = !aux_ff1_25_node_0;
    aux_ff1_25_and_12 = aux_ff1_25_nor_10 && aux_ff1_25_node_1;
    aux_ff1_25_or_15 = aux_ff1_25_and_13 || aux_ff1_25_not_7;
    aux_ff1_25_and_9 = aux_ff1_25_not_4 && aux_ff1_25_not_5;
    aux_ff1_25_or_14 = aux_ff1_25_and_12 || aux_ff1_25_not_6;
    aux_ff1_25_nor_16 = !(aux_ff1_25_or_15 || aux_ff1_25_nor_17);
    aux_ff1_25_nor_10 = !(aux_ff1_25_and_9 || aux_ff1_25_nor_11 || aux_ff1_25_not_7);
    aux_ff1_25_nor_17 = !(aux_ff1_25_or_14 || aux_ff1_25_nor_16);
    aux_ff1_25_node_18 = aux_ff1_25_nor_16;
    aux_ff1_25_node_19 = aux_ff1_25_nor_17;
    aux_ff1_25_q = aux_ff1_25_node_18;
    aux_ff1_25_q_bar_1 = aux_ff1_25_node_19;
    // End IC: FF1
    aux_level4_binary_counter_4bit_0_and_7 = aux_ff0_24_q && aux_ff1_25_q;
    // IC: FF2
    aux_ic_input_ff2_0 = aux_level4_binary_counter_4bit_0_or_22;
    aux_ic_input_ff2_1 = aux_level4_binary_counter_4bit_0_node_0;
    aux_ic_input_ff2_2 = HIGH;
    aux_ic_input_ff2_3 = aux_level4_binary_counter_4bit_0_node_1;
    aux_ff2_26_node_0 = aux_ic_input_ff2_0;
    aux_ff2_26_node_2 = aux_ic_input_ff2_2;
    aux_ff2_26_not_5 = !aux_ff2_26_node_1;
    aux_ff2_26_not_6 = !aux_ff2_26_node_2;
    aux_ff2_26_and_8 = aux_ff2_26_node_0 && aux_ff2_26_not_5;
    aux_ff2_26_node_1 = aux_ic_input_ff2_1;
    aux_ff2_26_node_3 = aux_ic_input_ff2_3;
    aux_ff2_26_nor_11 = !(aux_ff2_26_and_8 || aux_ff2_26_nor_10 || aux_ff2_26_not_6);
    aux_ff2_26_not_7 = !aux_ff2_26_node_3;
    aux_ff2_26_and_13 = aux_ff2_26_nor_11 && aux_ff2_26_node_1;
    aux_ff2_26_not_4 = !aux_ff2_26_node_0;
    aux_ff2_26_and_12 = aux_ff2_26_nor_10 && aux_ff2_26_node_1;
    aux_ff2_26_or_15 = aux_ff2_26_and_13 || aux_ff2_26_not_7;
    aux_ff2_26_and_9 = aux_ff2_26_not_4 && aux_ff2_26_not_5;
    aux_ff2_26_or_14 = aux_ff2_26_and_12 || aux_ff2_26_not_6;
    aux_ff2_26_nor_16 = !(aux_ff2_26_or_15 || aux_ff2_26_nor_17);
    aux_ff2_26_nor_10 = !(aux_ff2_26_and_9 || aux_ff2_26_nor_11 || aux_ff2_26_not_7);
    aux_ff2_26_nor_17 = !(aux_ff2_26_or_14 || aux_ff2_26_nor_16);
    aux_ff2_26_node_18 = aux_ff2_26_nor_16;
    aux_ff2_26_node_19 = aux_ff2_26_nor_17;
    aux_ff2_26_q = aux_ff2_26_node_18;
    aux_ff2_26_q_bar_1 = aux_ff2_26_node_19;
    // End IC: FF2
    aux_level4_binary_counter_4bit_0_and_8 = aux_level4_binary_counter_4bit_0_and_7 && aux_ff2_26_q;
    // IC: FF3
    aux_ic_input_ff3_0 = aux_level4_binary_counter_4bit_0_or_23;
    aux_ic_input_ff3_1 = aux_level4_binary_counter_4bit_0_node_0;
    aux_ic_input_ff3_2 = HIGH;
    aux_ic_input_ff3_3 = aux_level4_binary_counter_4bit_0_node_1;
    aux_ff3_27_node_0 = aux_ic_input_ff3_0;
    aux_ff3_27_node_2 = aux_ic_input_ff3_2;
    aux_ff3_27_not_5 = !aux_ff3_27_node_1;
    aux_ff3_27_not_6 = !aux_ff3_27_node_2;
    aux_ff3_27_and_8 = aux_ff3_27_node_0 && aux_ff3_27_not_5;
    aux_ff3_27_node_1 = aux_ic_input_ff3_1;
    aux_ff3_27_node_3 = aux_ic_input_ff3_3;
    aux_ff3_27_nor_11 = !(aux_ff3_27_and_8 || aux_ff3_27_nor_10 || aux_ff3_27_not_6);
    aux_ff3_27_not_7 = !aux_ff3_27_node_3;
    aux_ff3_27_and_13 = aux_ff3_27_nor_11 && aux_ff3_27_node_1;
    aux_ff3_27_not_4 = !aux_ff3_27_node_0;
    aux_ff3_27_and_12 = aux_ff3_27_nor_10 && aux_ff3_27_node_1;
    aux_ff3_27_or_15 = aux_ff3_27_and_13 || aux_ff3_27_not_7;
    aux_ff3_27_and_9 = aux_ff3_27_not_4 && aux_ff3_27_not_5;
    aux_ff3_27_or_14 = aux_ff3_27_and_12 || aux_ff3_27_not_6;
    aux_ff3_27_nor_16 = !(aux_ff3_27_or_15 || aux_ff3_27_nor_17);
    aux_ff3_27_nor_10 = !(aux_ff3_27_and_9 || aux_ff3_27_nor_11 || aux_ff3_27_not_7);
    aux_ff3_27_nor_17 = !(aux_ff3_27_or_14 || aux_ff3_27_nor_16);
    aux_ff3_27_node_18 = aux_ff3_27_nor_16;
    aux_ff3_27_node_19 = aux_ff3_27_nor_17;
    aux_ff3_27_q = aux_ff3_27_node_18;
    aux_ff3_27_q_bar_1 = aux_ff3_27_node_19;
    // End IC: FF3
    aux_level4_binary_counter_4bit_0_not_4 = !aux_ff1_25_q;
    aux_level4_binary_counter_4bit_0_not_5 = !aux_ff2_26_q;
    aux_level4_binary_counter_4bit_0_not_6 = !aux_ff3_27_q;
    aux_level4_binary_counter_4bit_0_not_9 = !aux_ff0_24_q;
    aux_level4_binary_counter_4bit_0_not_10 = !aux_level4_binary_counter_4bit_0_and_7;
    aux_level4_binary_counter_4bit_0_not_11 = !aux_level4_binary_counter_4bit_0_and_8;
    aux_level4_binary_counter_4bit_0_and_14 = aux_level4_binary_counter_4bit_0_not_4 && aux_ff0_24_q;
    aux_level4_binary_counter_4bit_0_and_15 = aux_ff1_25_q && aux_level4_binary_counter_4bit_0_not_9;
    aux_level4_binary_counter_4bit_0_and_16 = aux_level4_binary_counter_4bit_0_not_5 && aux_level4_binary_counter_4bit_0_and_7;
    aux_level4_binary_counter_4bit_0_and_17 = aux_ff2_26_q && aux_level4_binary_counter_4bit_0_not_10;
    aux_level4_binary_counter_4bit_0_and_18 = aux_level4_binary_counter_4bit_0_not_6 && aux_level4_binary_counter_4bit_0_and_8;
    aux_level4_binary_counter_4bit_0_and_19 = aux_ff3_27_q && aux_level4_binary_counter_4bit_0_not_11;
    aux_level4_binary_counter_4bit_0_not_3 = !aux_ff0_24_q;
    aux_level4_binary_counter_4bit_0_and_12 = LOW && LOW;
    aux_level4_binary_counter_4bit_0_and_13 = LOW && LOW;
    aux_level4_binary_counter_4bit_0_or_20 = LOW || LOW;
    aux_level4_binary_counter_4bit_0_or_21 = aux_level4_binary_counter_4bit_0_and_14 || aux_level4_binary_counter_4bit_0_and_15;
    aux_level4_binary_counter_4bit_0_or_22 = aux_level4_binary_counter_4bit_0_and_16 || aux_level4_binary_counter_4bit_0_and_17;
    aux_level4_binary_counter_4bit_0_or_23 = aux_level4_binary_counter_4bit_0_and_18 || aux_level4_binary_counter_4bit_0_and_19;
    aux_level4_binary_counter_4bit_0_node_28 = aux_ff0_24_q;
    aux_level4_binary_counter_4bit_0_node_29 = aux_ff1_25_q;
    aux_level4_binary_counter_4bit_0_node_30 = aux_ff2_26_q;
    aux_level4_binary_counter_4bit_0_node_31 = aux_ff3_27_q;
    aux_level4_binary_counter_4bit_0_q0 = aux_level4_binary_counter_4bit_0_node_28;
    aux_level4_binary_counter_4bit_0_q1_1 = aux_level4_binary_counter_4bit_0_node_29;
    aux_level4_binary_counter_4bit_0_q2_2 = aux_level4_binary_counter_4bit_0_node_30;
    aux_level4_binary_counter_4bit_0_q3_3 = aux_level4_binary_counter_4bit_0_node_31;
    // End IC: LEVEL4_BINARY_COUNTER_4BIT
}

void loop() {
    // Reading input data //.
    input_switch1_val = digitalRead(input_switch1);
    input_switch2_val = digitalRead(input_switch2);

    // Updating clocks. //

    computeLogic();

    // Writing output data. //
    digitalWrite(led1_0, aux_level4_binary_counter_4bit_0_q0);
    digitalWrite(led2_0, aux_level4_binary_counter_4bit_0_q1_1);
    digitalWrite(led3_0, aux_level4_binary_counter_4bit_0_q2_2);
    digitalWrite(led4_0, aux_level4_binary_counter_4bit_0_q3_3);
}
