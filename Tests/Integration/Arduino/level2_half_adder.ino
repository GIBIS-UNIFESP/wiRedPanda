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
const int led1_1 = A2;
const int led2_1 = A3;

/* ====== Aux. Variables ====== */
bool input_switch1_val = LOW;
bool input_switch2_val = LOW;
// IC: LEVEL2_HALF_ADDER
bool aux_level2_half_adder_0_sum = LOW;
bool aux_level2_half_adder_0_carry_1 = LOW;
bool aux_level2_half_adder_0_node_0 = LOW;
bool aux_level2_half_adder_0_node_1 = LOW;
bool aux_level2_half_adder_0_xor_2 = LOW;
bool aux_level2_half_adder_0_and_3 = LOW;
bool aux_level2_half_adder_0_node_4 = LOW;
bool aux_level2_half_adder_0_node_5 = LOW;
bool aux_ic_input_level2_half_adder_0_0 = LOW;
bool aux_ic_input_level2_half_adder_0_1 = LOW;
// End IC: LEVEL2_HALF_ADDER

void setup() {
    pinMode(input_switch1, INPUT);
    pinMode(input_switch2, INPUT);
    pinMode(led1_1, OUTPUT);
    pinMode(led2_1, OUTPUT);
}

void computeLogic() {
    // Assigning aux variables. //
    // IC: LEVEL2_HALF_ADDER
    aux_ic_input_level2_half_adder_0_0 = input_switch1_val;
    aux_ic_input_level2_half_adder_0_1 = input_switch2_val;
    aux_level2_half_adder_0_node_0 = aux_ic_input_level2_half_adder_0_0;
    aux_level2_half_adder_0_node_1 = aux_ic_input_level2_half_adder_0_1;
    aux_level2_half_adder_0_xor_2 = aux_level2_half_adder_0_node_0 ^ aux_level2_half_adder_0_node_1;
    aux_level2_half_adder_0_and_3 = aux_level2_half_adder_0_node_0 && aux_level2_half_adder_0_node_1;
    aux_level2_half_adder_0_node_4 = aux_level2_half_adder_0_xor_2;
    aux_level2_half_adder_0_node_5 = aux_level2_half_adder_0_and_3;
    aux_level2_half_adder_0_sum = aux_level2_half_adder_0_node_4;
    aux_level2_half_adder_0_carry_1 = aux_level2_half_adder_0_node_5;
    // End IC: LEVEL2_HALF_ADDER
}

void loop() {
    // Reading input data //.
    input_switch1_val = digitalRead(input_switch1);
    input_switch2_val = digitalRead(input_switch2);

    // Updating clocks. //

    computeLogic();

    // Writing output data. //
    digitalWrite(led1_1, aux_level2_half_adder_0_sum);
    digitalWrite(led2_1, aux_level2_half_adder_0_carry_1);
}
