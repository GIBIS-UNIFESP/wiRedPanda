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
const int led1_0 = 4;

/* ====== Aux. Variables ====== */
bool input_switch1_val = LOW;
bool input_switch2_val = LOW;
bool input_switch3_val = LOW;
bool input_switch4_val = LOW;
bool input_switch5_val = LOW;
bool input_switch6_val = LOW;
bool input_switch7_val = LOW;
bool input_switch8_val = LOW;
// IC: LEVEL2_PARITY_CHECKER
bool aux_level2_parity_checker_0_parity = LOW;
bool aux_level2_parity_checker_0_node_0 = LOW;
bool aux_level2_parity_checker_0_node_1 = LOW;
bool aux_level2_parity_checker_0_node_2 = LOW;
bool aux_level2_parity_checker_0_node_3 = LOW;
bool aux_level2_parity_checker_0_node_4 = LOW;
bool aux_level2_parity_checker_0_node_5 = LOW;
bool aux_level2_parity_checker_0_node_6 = LOW;
bool aux_level2_parity_checker_0_node_7 = LOW;
bool aux_level2_parity_checker_0_xor_8 = LOW;
bool aux_level2_parity_checker_0_xor_9 = LOW;
bool aux_level2_parity_checker_0_xor_10 = LOW;
bool aux_level2_parity_checker_0_xor_11 = LOW;
bool aux_level2_parity_checker_0_xor_12 = LOW;
bool aux_level2_parity_checker_0_xor_13 = LOW;
bool aux_level2_parity_checker_0_xor_14 = LOW;
bool aux_level2_parity_checker_0_node_15 = LOW;
bool aux_ic_input_level2_parity_checker_0 = LOW;
bool aux_ic_input_level2_parity_checker_1 = LOW;
bool aux_ic_input_level2_parity_checker_2 = LOW;
bool aux_ic_input_level2_parity_checker_3 = LOW;
bool aux_ic_input_level2_parity_checker_4 = LOW;
bool aux_ic_input_level2_parity_checker_5 = LOW;
bool aux_ic_input_level2_parity_checker_6 = LOW;
bool aux_ic_input_level2_parity_checker_7 = LOW;
// End IC: LEVEL2_PARITY_CHECKER

void setup() {
    pinMode(input_switch1, INPUT);
    pinMode(input_switch2, INPUT);
    pinMode(input_switch3, INPUT);
    pinMode(input_switch4, INPUT);
    pinMode(input_switch5, INPUT);
    pinMode(input_switch6, INPUT);
    pinMode(input_switch7, INPUT);
    pinMode(input_switch8, INPUT);
    pinMode(led1_0, OUTPUT);
}

void computeLogic() {
    // Assigning aux variables. //
    // IC: LEVEL2_PARITY_CHECKER
    aux_ic_input_level2_parity_checker_0 = input_switch1_val;
    aux_ic_input_level2_parity_checker_1 = input_switch2_val;
    aux_ic_input_level2_parity_checker_2 = input_switch3_val;
    aux_ic_input_level2_parity_checker_3 = input_switch4_val;
    aux_ic_input_level2_parity_checker_4 = input_switch5_val;
    aux_ic_input_level2_parity_checker_5 = input_switch6_val;
    aux_ic_input_level2_parity_checker_6 = input_switch7_val;
    aux_ic_input_level2_parity_checker_7 = input_switch8_val;
    aux_level2_parity_checker_0_node_0 = aux_ic_input_level2_parity_checker_0;
    aux_level2_parity_checker_0_node_1 = aux_ic_input_level2_parity_checker_1;
    aux_level2_parity_checker_0_node_2 = aux_ic_input_level2_parity_checker_2;
    aux_level2_parity_checker_0_node_3 = aux_ic_input_level2_parity_checker_3;
    aux_level2_parity_checker_0_node_4 = aux_ic_input_level2_parity_checker_4;
    aux_level2_parity_checker_0_node_5 = aux_ic_input_level2_parity_checker_5;
    aux_level2_parity_checker_0_node_6 = aux_ic_input_level2_parity_checker_6;
    aux_level2_parity_checker_0_node_7 = aux_ic_input_level2_parity_checker_7;
    aux_level2_parity_checker_0_xor_8 = aux_level2_parity_checker_0_node_0 ^ aux_level2_parity_checker_0_node_1;
    aux_level2_parity_checker_0_xor_9 = aux_level2_parity_checker_0_node_2 ^ aux_level2_parity_checker_0_node_3;
    aux_level2_parity_checker_0_xor_10 = aux_level2_parity_checker_0_node_4 ^ aux_level2_parity_checker_0_node_5;
    aux_level2_parity_checker_0_xor_11 = aux_level2_parity_checker_0_node_6 ^ aux_level2_parity_checker_0_node_7;
    aux_level2_parity_checker_0_xor_12 = aux_level2_parity_checker_0_xor_8 ^ aux_level2_parity_checker_0_xor_9;
    aux_level2_parity_checker_0_xor_13 = aux_level2_parity_checker_0_xor_10 ^ aux_level2_parity_checker_0_xor_11;
    aux_level2_parity_checker_0_xor_14 = aux_level2_parity_checker_0_xor_12 ^ aux_level2_parity_checker_0_xor_13;
    aux_level2_parity_checker_0_node_15 = aux_level2_parity_checker_0_xor_14;
    aux_level2_parity_checker_0_parity = aux_level2_parity_checker_0_node_15;
    // End IC: LEVEL2_PARITY_CHECKER
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
    digitalWrite(led1_0, aux_level2_parity_checker_0_parity);
}
