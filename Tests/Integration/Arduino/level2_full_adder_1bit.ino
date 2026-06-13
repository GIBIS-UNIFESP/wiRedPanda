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
// IC: HA1
bool aux_level2_full_adder_1bit_0_ha1_3_sum = LOW;
bool aux_level2_full_adder_1bit_0_ha1_3_carry_1 = LOW;
bool aux_level2_full_adder_1bit_0_ha1_3_node_0 = LOW;
bool aux_level2_full_adder_1bit_0_ha1_3_node_1 = LOW;
bool aux_level2_full_adder_1bit_0_ha1_3_xor_2 = LOW;
bool aux_level2_full_adder_1bit_0_ha1_3_and_3 = LOW;
bool aux_level2_full_adder_1bit_0_ha1_3_node_4 = LOW;
bool aux_level2_full_adder_1bit_0_ha1_3_node_5 = LOW;
bool aux_ic_input_level2_full_adder_1bit_0_ha1_3_0 = LOW;
bool aux_ic_input_level2_full_adder_1bit_0_ha1_3_1 = LOW;
// End IC: HA1
// IC: HA2
bool aux_level2_full_adder_1bit_0_ha2_4_sum = LOW;
bool aux_level2_full_adder_1bit_0_ha2_4_carry_1 = LOW;
bool aux_level2_full_adder_1bit_0_ha2_4_node_0 = LOW;
bool aux_level2_full_adder_1bit_0_ha2_4_node_1 = LOW;
bool aux_level2_full_adder_1bit_0_ha2_4_xor_2 = LOW;
bool aux_level2_full_adder_1bit_0_ha2_4_and_3 = LOW;
bool aux_level2_full_adder_1bit_0_ha2_4_node_4 = LOW;
bool aux_level2_full_adder_1bit_0_ha2_4_node_5 = LOW;
bool aux_ic_input_level2_full_adder_1bit_0_ha2_4_0 = LOW;
bool aux_ic_input_level2_full_adder_1bit_0_ha2_4_1 = LOW;
// End IC: HA2
bool aux_level2_full_adder_1bit_0_or_5 = LOW;
bool aux_level2_full_adder_1bit_0_node_6 = LOW;
bool aux_level2_full_adder_1bit_0_node_7 = LOW;
bool aux_ic_input_level2_full_adder_1bit_0_0 = LOW;
bool aux_ic_input_level2_full_adder_1bit_0_1 = LOW;
bool aux_ic_input_level2_full_adder_1bit_0_2 = LOW;
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
    aux_ic_input_level2_full_adder_1bit_0_0 = input_switch1_val;
    aux_ic_input_level2_full_adder_1bit_0_1 = input_switch2_val;
    aux_ic_input_level2_full_adder_1bit_0_2 = input_switch3_val;
    aux_level2_full_adder_1bit_0_node_0 = aux_ic_input_level2_full_adder_1bit_0_0;
    aux_level2_full_adder_1bit_0_node_1 = aux_ic_input_level2_full_adder_1bit_0_1;
    aux_level2_full_adder_1bit_0_node_2 = aux_ic_input_level2_full_adder_1bit_0_2;
    // IC: HA1
    aux_ic_input_level2_full_adder_1bit_0_ha1_3_0 = aux_level2_full_adder_1bit_0_node_0;
    aux_ic_input_level2_full_adder_1bit_0_ha1_3_1 = aux_level2_full_adder_1bit_0_node_1;
    aux_level2_full_adder_1bit_0_ha1_3_node_0 = aux_ic_input_level2_full_adder_1bit_0_ha1_3_0;
    aux_level2_full_adder_1bit_0_ha1_3_node_1 = aux_ic_input_level2_full_adder_1bit_0_ha1_3_1;
    aux_level2_full_adder_1bit_0_ha1_3_xor_2 = aux_level2_full_adder_1bit_0_ha1_3_node_0 ^ aux_level2_full_adder_1bit_0_ha1_3_node_1;
    aux_level2_full_adder_1bit_0_ha1_3_and_3 = aux_level2_full_adder_1bit_0_ha1_3_node_0 && aux_level2_full_adder_1bit_0_ha1_3_node_1;
    aux_level2_full_adder_1bit_0_ha1_3_node_4 = aux_level2_full_adder_1bit_0_ha1_3_xor_2;
    aux_level2_full_adder_1bit_0_ha1_3_node_5 = aux_level2_full_adder_1bit_0_ha1_3_and_3;
    aux_level2_full_adder_1bit_0_ha1_3_sum = aux_level2_full_adder_1bit_0_ha1_3_node_4;
    aux_level2_full_adder_1bit_0_ha1_3_carry_1 = aux_level2_full_adder_1bit_0_ha1_3_node_5;
    // End IC: HA1
    // IC: HA2
    aux_ic_input_level2_full_adder_1bit_0_ha2_4_0 = aux_level2_full_adder_1bit_0_ha1_3_sum;
    aux_ic_input_level2_full_adder_1bit_0_ha2_4_1 = aux_level2_full_adder_1bit_0_node_2;
    aux_level2_full_adder_1bit_0_ha2_4_node_0 = aux_ic_input_level2_full_adder_1bit_0_ha2_4_0;
    aux_level2_full_adder_1bit_0_ha2_4_node_1 = aux_ic_input_level2_full_adder_1bit_0_ha2_4_1;
    aux_level2_full_adder_1bit_0_ha2_4_xor_2 = aux_level2_full_adder_1bit_0_ha2_4_node_0 ^ aux_level2_full_adder_1bit_0_ha2_4_node_1;
    aux_level2_full_adder_1bit_0_ha2_4_and_3 = aux_level2_full_adder_1bit_0_ha2_4_node_0 && aux_level2_full_adder_1bit_0_ha2_4_node_1;
    aux_level2_full_adder_1bit_0_ha2_4_node_4 = aux_level2_full_adder_1bit_0_ha2_4_xor_2;
    aux_level2_full_adder_1bit_0_ha2_4_node_5 = aux_level2_full_adder_1bit_0_ha2_4_and_3;
    aux_level2_full_adder_1bit_0_ha2_4_sum = aux_level2_full_adder_1bit_0_ha2_4_node_4;
    aux_level2_full_adder_1bit_0_ha2_4_carry_1 = aux_level2_full_adder_1bit_0_ha2_4_node_5;
    // End IC: HA2
    aux_level2_full_adder_1bit_0_or_5 = aux_level2_full_adder_1bit_0_ha1_3_carry_1 || aux_level2_full_adder_1bit_0_ha2_4_carry_1;
    aux_level2_full_adder_1bit_0_node_6 = aux_level2_full_adder_1bit_0_ha2_4_sum;
    aux_level2_full_adder_1bit_0_node_7 = aux_level2_full_adder_1bit_0_or_5;
    aux_level2_full_adder_1bit_0_sum = aux_level2_full_adder_1bit_0_node_6;
    aux_level2_full_adder_1bit_0_cout_1 = aux_level2_full_adder_1bit_0_node_7;
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
