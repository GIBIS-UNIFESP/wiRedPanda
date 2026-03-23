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
const int input_switch2 = A1;
const int input_switch3 = A2;

/* ========= Outputs ========== */
const int led1_1 = A3;
const int led2_1 = A4;

/* ====== Aux. Variables ====== */
bool input_switch1_val = LOW;
bool input_switch2_val = LOW;
bool input_switch3_val = LOW;
// IC: LEVEL2_FULL_ADDER_1BIT
bool aux_level2_full_adder_1bit_0_sum = LOW;
bool aux_level2_full_adder_1bit_0_cout_1 = LOW;
bool aux_level2_full_adder_1bit_0_node_0 = LOW;
bool aux_level2_full_adder_1bit_0_node_1 = LOW;
bool aux_level2_full_adder_1bit_0_node_2 = LOW;
bool aux_level2_full_adder_1bit_0_xor_3 = LOW;
bool aux_level2_full_adder_1bit_0_and_4 = LOW;
bool aux_level2_full_adder_1bit_0_xor_5 = LOW;
bool aux_level2_full_adder_1bit_0_and_6 = LOW;
bool aux_level2_full_adder_1bit_0_or_7 = LOW;
bool aux_level2_full_adder_1bit_0_node_8 = LOW;
bool aux_level2_full_adder_1bit_0_node_9 = LOW;
bool aux_ic_input_level2_full_adder_1bit_0 = LOW;
bool aux_ic_input_level2_full_adder_1bit_1 = LOW;
bool aux_ic_input_level2_full_adder_1bit_2 = LOW;
// End IC: LEVEL2_FULL_ADDER_1BIT

void setup() {
    pinMode(input_switch1, INPUT);
    pinMode(input_switch2, INPUT);
    pinMode(input_switch3, INPUT);
    pinMode(led1_1, OUTPUT);
    pinMode(led2_1, OUTPUT);
}

void computeLogic() {
    // Assigning aux variables. //
    // IC: LEVEL2_FULL_ADDER_1BIT
    aux_ic_input_level2_full_adder_1bit_0 = input_switch1_val;
    aux_ic_input_level2_full_adder_1bit_1 = input_switch2_val;
    aux_ic_input_level2_full_adder_1bit_2 = input_switch3_val;
    aux_level2_full_adder_1bit_0_node_0 = aux_ic_input_level2_full_adder_1bit_0;
    aux_level2_full_adder_1bit_0_node_1 = aux_ic_input_level2_full_adder_1bit_1;
    aux_level2_full_adder_1bit_0_node_2 = aux_ic_input_level2_full_adder_1bit_2;
    aux_level2_full_adder_1bit_0_xor_3 = aux_level2_full_adder_1bit_0_node_0 ^ aux_level2_full_adder_1bit_0_node_1;
    aux_level2_full_adder_1bit_0_and_4 = aux_level2_full_adder_1bit_0_node_0 && aux_level2_full_adder_1bit_0_node_1;
    aux_level2_full_adder_1bit_0_and_6 = aux_level2_full_adder_1bit_0_xor_3 && aux_level2_full_adder_1bit_0_node_2;
    aux_level2_full_adder_1bit_0_xor_5 = aux_level2_full_adder_1bit_0_xor_3 ^ aux_level2_full_adder_1bit_0_node_2;
    aux_level2_full_adder_1bit_0_or_7 = aux_level2_full_adder_1bit_0_and_4 || aux_level2_full_adder_1bit_0_and_6;
    aux_level2_full_adder_1bit_0_node_8 = aux_level2_full_adder_1bit_0_xor_5;
    aux_level2_full_adder_1bit_0_node_9 = aux_level2_full_adder_1bit_0_or_7;
    aux_level2_full_adder_1bit_0_sum = aux_level2_full_adder_1bit_0_node_8;
    aux_level2_full_adder_1bit_0_cout_1 = aux_level2_full_adder_1bit_0_node_9;
    // End IC: LEVEL2_FULL_ADDER_1BIT
}

void loop() {
    // Reading input data //.
    input_switch1_val = digitalRead(input_switch1);
    input_switch2_val = digitalRead(input_switch2);
    input_switch3_val = digitalRead(input_switch3);

    // Updating clocks. //

    computeLogic();

    // Writing output data. //
    digitalWrite(led1_1, aux_level2_full_adder_1bit_0_sum);
    digitalWrite(led2_1, aux_level2_full_adder_1bit_0_cout_1);
}
