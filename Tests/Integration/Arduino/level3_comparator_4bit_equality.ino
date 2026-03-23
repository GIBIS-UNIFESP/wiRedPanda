// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //
//
// Target Board: Arduino UNO R3/R4
// Pin Usage: 9/18 pins
//


#include <elapsedMillis.h>
/* ========= Inputs ========== */
const int input_switch1 = A0;
const int input_switch2 = A1;
const int input_switch3 = A2;
const int input_switch4 = A3;
const int input_switch5 = A4;
const int input_switch6 = A5;
const int input_switch7 = 2;
const int input_switch8 = 3;

/* ========= Outputs ========== */
const int led1_1 = 4;

/* ====== Aux. Variables ====== */
bool input_switch1_val = LOW;
bool input_switch2_val = LOW;
bool input_switch3_val = LOW;
bool input_switch4_val = LOW;
bool input_switch5_val = LOW;
bool input_switch6_val = LOW;
bool input_switch7_val = LOW;
bool input_switch8_val = LOW;
// IC: LEVEL3_COMPARATOR_4BIT_EQUALITY
bool aux_level3_comparator_4bit_equality_0_equal = LOW;
bool aux_level3_comparator_4bit_equality_0_node_0 = LOW;
bool aux_level3_comparator_4bit_equality_0_node_1 = LOW;
bool aux_level3_comparator_4bit_equality_0_node_2 = LOW;
bool aux_level3_comparator_4bit_equality_0_node_3 = LOW;
bool aux_level3_comparator_4bit_equality_0_node_4 = LOW;
bool aux_level3_comparator_4bit_equality_0_node_5 = LOW;
bool aux_level3_comparator_4bit_equality_0_node_6 = LOW;
bool aux_level3_comparator_4bit_equality_0_node_7 = LOW;
bool aux_level3_comparator_4bit_equality_0_xnor_8 = LOW;
bool aux_level3_comparator_4bit_equality_0_xnor_9 = LOW;
bool aux_level3_comparator_4bit_equality_0_xnor_10 = LOW;
bool aux_level3_comparator_4bit_equality_0_xnor_11 = LOW;
bool aux_level3_comparator_4bit_equality_0_and_12 = LOW;
bool aux_level3_comparator_4bit_equality_0_node_13 = LOW;
bool aux_ic_input_level3_comparator_4bit_equality_0 = LOW;
bool aux_ic_input_level3_comparator_4bit_equality_1 = LOW;
bool aux_ic_input_level3_comparator_4bit_equality_2 = LOW;
bool aux_ic_input_level3_comparator_4bit_equality_3 = LOW;
bool aux_ic_input_level3_comparator_4bit_equality_4 = LOW;
bool aux_ic_input_level3_comparator_4bit_equality_5 = LOW;
bool aux_ic_input_level3_comparator_4bit_equality_6 = LOW;
bool aux_ic_input_level3_comparator_4bit_equality_7 = LOW;
// End IC: LEVEL3_COMPARATOR_4BIT_EQUALITY

void setup() {
    pinMode(input_switch1, INPUT);
    pinMode(input_switch2, INPUT);
    pinMode(input_switch3, INPUT);
    pinMode(input_switch4, INPUT);
    pinMode(input_switch5, INPUT);
    pinMode(input_switch6, INPUT);
    pinMode(input_switch7, INPUT);
    pinMode(input_switch8, INPUT);
    pinMode(led1_1, OUTPUT);
}

void computeLogic() {
    // Assigning aux variables. //
    // IC: LEVEL3_COMPARATOR_4BIT_EQUALITY
    aux_ic_input_level3_comparator_4bit_equality_0 = input_switch1_val;
    aux_ic_input_level3_comparator_4bit_equality_1 = input_switch2_val;
    aux_ic_input_level3_comparator_4bit_equality_2 = input_switch3_val;
    aux_ic_input_level3_comparator_4bit_equality_3 = input_switch4_val;
    aux_ic_input_level3_comparator_4bit_equality_4 = input_switch5_val;
    aux_ic_input_level3_comparator_4bit_equality_5 = input_switch6_val;
    aux_ic_input_level3_comparator_4bit_equality_6 = input_switch7_val;
    aux_ic_input_level3_comparator_4bit_equality_7 = input_switch8_val;
    aux_level3_comparator_4bit_equality_0_node_0 = aux_ic_input_level3_comparator_4bit_equality_0;
    aux_level3_comparator_4bit_equality_0_node_1 = aux_ic_input_level3_comparator_4bit_equality_1;
    aux_level3_comparator_4bit_equality_0_node_2 = aux_ic_input_level3_comparator_4bit_equality_2;
    aux_level3_comparator_4bit_equality_0_node_3 = aux_ic_input_level3_comparator_4bit_equality_3;
    aux_level3_comparator_4bit_equality_0_node_4 = aux_ic_input_level3_comparator_4bit_equality_4;
    aux_level3_comparator_4bit_equality_0_node_5 = aux_ic_input_level3_comparator_4bit_equality_5;
    aux_level3_comparator_4bit_equality_0_node_6 = aux_ic_input_level3_comparator_4bit_equality_6;
    aux_level3_comparator_4bit_equality_0_node_7 = aux_ic_input_level3_comparator_4bit_equality_7;
    aux_level3_comparator_4bit_equality_0_xnor_8 = !(aux_level3_comparator_4bit_equality_0_node_0 ^ aux_level3_comparator_4bit_equality_0_node_4);
    aux_level3_comparator_4bit_equality_0_xnor_9 = !(aux_level3_comparator_4bit_equality_0_node_1 ^ aux_level3_comparator_4bit_equality_0_node_5);
    aux_level3_comparator_4bit_equality_0_xnor_10 = !(aux_level3_comparator_4bit_equality_0_node_2 ^ aux_level3_comparator_4bit_equality_0_node_6);
    aux_level3_comparator_4bit_equality_0_xnor_11 = !(aux_level3_comparator_4bit_equality_0_node_3 ^ aux_level3_comparator_4bit_equality_0_node_7);
    aux_level3_comparator_4bit_equality_0_and_12 = aux_level3_comparator_4bit_equality_0_xnor_8 && aux_level3_comparator_4bit_equality_0_xnor_9 && aux_level3_comparator_4bit_equality_0_xnor_10 && aux_level3_comparator_4bit_equality_0_xnor_11;
    aux_level3_comparator_4bit_equality_0_node_13 = aux_level3_comparator_4bit_equality_0_and_12;
    aux_level3_comparator_4bit_equality_0_equal = aux_level3_comparator_4bit_equality_0_node_13;
    // End IC: LEVEL3_COMPARATOR_4BIT_EQUALITY
}

void loop() {
    // Reading input data //.
    input_switch1_val = digitalRead(input_switch1);
    input_switch2_val = digitalRead(input_switch2);
    input_switch3_val = digitalRead(input_switch3);
    input_switch4_val = digitalRead(input_switch4);
    input_switch5_val = digitalRead(input_switch5);
    input_switch6_val = digitalRead(input_switch6);
    input_switch7_val = digitalRead(input_switch7);
    input_switch8_val = digitalRead(input_switch8);

    // Updating clocks. //

    computeLogic();

    // Writing output data. //
    digitalWrite(led1_1, aux_level3_comparator_4bit_equality_0_equal);
}
