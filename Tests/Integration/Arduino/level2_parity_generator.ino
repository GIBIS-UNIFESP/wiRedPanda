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
const int input_switch4 = A3;

/* ========= Outputs ========== */
const int led1_1 = A4;

/* ====== Aux. Variables ====== */
bool input_switch1_val = LOW;
bool input_switch2_val = LOW;
bool input_switch3_val = LOW;
bool input_switch4_val = LOW;
// IC: LEVEL2_PARITY_GENERATOR
bool aux_level2_parity_generator_0_parity = LOW;
bool aux_level2_parity_generator_0_node_0 = LOW;
bool aux_level2_parity_generator_0_node_1 = LOW;
bool aux_level2_parity_generator_0_node_2 = LOW;
bool aux_level2_parity_generator_0_node_3 = LOW;
bool aux_level2_parity_generator_0_xor_4 = LOW;
bool aux_level2_parity_generator_0_xor_5 = LOW;
bool aux_level2_parity_generator_0_xor_6 = LOW;
bool aux_level2_parity_generator_0_node_7 = LOW;
bool aux_ic_input_level2_parity_generator_0_0 = LOW;
bool aux_ic_input_level2_parity_generator_0_1 = LOW;
bool aux_ic_input_level2_parity_generator_0_2 = LOW;
bool aux_ic_input_level2_parity_generator_0_3 = LOW;
// End IC: LEVEL2_PARITY_GENERATOR

void setup() {
    pinMode(input_switch1, INPUT);
    pinMode(input_switch2, INPUT);
    pinMode(input_switch3, INPUT);
    pinMode(input_switch4, INPUT);
    pinMode(led1_1, OUTPUT);
}

void computeLogic() {
    // Assigning aux variables. //
    // IC: LEVEL2_PARITY_GENERATOR
    aux_ic_input_level2_parity_generator_0_0 = input_switch1_val;
    aux_ic_input_level2_parity_generator_0_1 = input_switch2_val;
    aux_ic_input_level2_parity_generator_0_2 = input_switch3_val;
    aux_ic_input_level2_parity_generator_0_3 = input_switch4_val;
    aux_level2_parity_generator_0_node_0 = aux_ic_input_level2_parity_generator_0_0;
    aux_level2_parity_generator_0_node_1 = aux_ic_input_level2_parity_generator_0_1;
    aux_level2_parity_generator_0_node_2 = aux_ic_input_level2_parity_generator_0_2;
    aux_level2_parity_generator_0_node_3 = aux_ic_input_level2_parity_generator_0_3;
    aux_level2_parity_generator_0_xor_4 = aux_level2_parity_generator_0_node_0 ^ aux_level2_parity_generator_0_node_1;
    aux_level2_parity_generator_0_xor_5 = aux_level2_parity_generator_0_node_2 ^ aux_level2_parity_generator_0_node_3;
    aux_level2_parity_generator_0_xor_6 = aux_level2_parity_generator_0_xor_4 ^ aux_level2_parity_generator_0_xor_5;
    aux_level2_parity_generator_0_node_7 = aux_level2_parity_generator_0_xor_6;
    aux_level2_parity_generator_0_parity = aux_level2_parity_generator_0_node_7;
    // End IC: LEVEL2_PARITY_GENERATOR
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
    digitalWrite(led1_1, aux_level2_parity_generator_0_parity);
}
