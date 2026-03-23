// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //
//
// Target Board: Arduino UNO R3/R4
// Pin Usage: 14/18 pins
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
const int input_switch9 = 4;

/* ========= Outputs ========== */
const int led1_1 = 5;
const int led2_1 = 6;
const int led3_1 = 7;
const int led4_1 = 8;
const int led5_1 = 9;

/* ====== Aux. Variables ====== */
bool input_switch1_val = LOW;
bool input_switch2_val = LOW;
bool input_switch3_val = LOW;
bool input_switch4_val = LOW;
bool input_switch5_val = LOW;
bool input_switch6_val = LOW;
bool input_switch7_val = LOW;
bool input_switch8_val = LOW;
bool input_switch9_val = LOW;
// IC: LEVEL4_RIPPLE_ADDER_4BIT
bool aux_level4_ripple_adder_4bit_0_sum0 = LOW;
bool aux_level4_ripple_adder_4bit_0_sum1_1 = LOW;
bool aux_level4_ripple_adder_4bit_0_sum2_2 = LOW;
bool aux_level4_ripple_adder_4bit_0_carryout_3 = LOW;
bool aux_level4_ripple_adder_4bit_0_sum3_4 = LOW;
bool aux_level4_ripple_adder_4bit_0_node_0 = LOW;
bool aux_level4_ripple_adder_4bit_0_node_1 = LOW;
bool aux_level4_ripple_adder_4bit_0_node_2 = LOW;
bool aux_level4_ripple_adder_4bit_0_node_3 = LOW;
bool aux_level4_ripple_adder_4bit_0_node_4 = LOW;
bool aux_level4_ripple_adder_4bit_0_node_5 = LOW;
bool aux_level4_ripple_adder_4bit_0_node_6 = LOW;
bool aux_level4_ripple_adder_4bit_0_node_7 = LOW;
bool aux_level4_ripple_adder_4bit_0_node_8 = LOW;
// IC: FA[0]
bool aux_fa0_9_sum = LOW;
bool aux_fa0_9_cout_1 = LOW;
bool aux_fa0_9_node_0 = LOW;
bool aux_fa0_9_node_1 = LOW;
bool aux_fa0_9_node_2 = LOW;
bool aux_fa0_9_xor_3 = LOW;
bool aux_fa0_9_and_4 = LOW;
bool aux_fa0_9_xor_5 = LOW;
bool aux_fa0_9_and_6 = LOW;
bool aux_fa0_9_or_7 = LOW;
bool aux_fa0_9_node_8 = LOW;
bool aux_fa0_9_node_9 = LOW;
bool aux_ic_input_fa0_9_0 = LOW;
bool aux_ic_input_fa0_9_1 = LOW;
bool aux_ic_input_fa0_9_2 = LOW;
// End IC: FA[0]
// IC: FA[1]
bool aux_fa1_10_sum = LOW;
bool aux_fa1_10_cout_1 = LOW;
bool aux_fa1_10_node_0 = LOW;
bool aux_fa1_10_node_1 = LOW;
bool aux_fa1_10_node_2 = LOW;
bool aux_fa1_10_xor_3 = LOW;
bool aux_fa1_10_and_4 = LOW;
bool aux_fa1_10_xor_5 = LOW;
bool aux_fa1_10_and_6 = LOW;
bool aux_fa1_10_or_7 = LOW;
bool aux_fa1_10_node_8 = LOW;
bool aux_fa1_10_node_9 = LOW;
bool aux_ic_input_fa1_10_0 = LOW;
bool aux_ic_input_fa1_10_1 = LOW;
bool aux_ic_input_fa1_10_2 = LOW;
// End IC: FA[1]
// IC: FA[2]
bool aux_fa2_11_sum = LOW;
bool aux_fa2_11_cout_1 = LOW;
bool aux_fa2_11_node_0 = LOW;
bool aux_fa2_11_node_1 = LOW;
bool aux_fa2_11_node_2 = LOW;
bool aux_fa2_11_xor_3 = LOW;
bool aux_fa2_11_and_4 = LOW;
bool aux_fa2_11_xor_5 = LOW;
bool aux_fa2_11_and_6 = LOW;
bool aux_fa2_11_or_7 = LOW;
bool aux_fa2_11_node_8 = LOW;
bool aux_fa2_11_node_9 = LOW;
bool aux_ic_input_fa2_11_0 = LOW;
bool aux_ic_input_fa2_11_1 = LOW;
bool aux_ic_input_fa2_11_2 = LOW;
// End IC: FA[2]
// IC: FA[3]
bool aux_fa3_12_sum = LOW;
bool aux_fa3_12_cout_1 = LOW;
bool aux_fa3_12_node_0 = LOW;
bool aux_fa3_12_node_1 = LOW;
bool aux_fa3_12_node_2 = LOW;
bool aux_fa3_12_xor_3 = LOW;
bool aux_fa3_12_and_4 = LOW;
bool aux_fa3_12_xor_5 = LOW;
bool aux_fa3_12_and_6 = LOW;
bool aux_fa3_12_or_7 = LOW;
bool aux_fa3_12_node_8 = LOW;
bool aux_fa3_12_node_9 = LOW;
bool aux_ic_input_fa3_12_0 = LOW;
bool aux_ic_input_fa3_12_1 = LOW;
bool aux_ic_input_fa3_12_2 = LOW;
// End IC: FA[3]
bool aux_level4_ripple_adder_4bit_0_node_13 = LOW;
bool aux_level4_ripple_adder_4bit_0_node_14 = LOW;
bool aux_level4_ripple_adder_4bit_0_node_15 = LOW;
bool aux_level4_ripple_adder_4bit_0_node_16 = LOW;
bool aux_level4_ripple_adder_4bit_0_node_17 = LOW;
bool aux_ic_input_level4_ripple_adder_4bit_0_0 = LOW;
bool aux_ic_input_level4_ripple_adder_4bit_0_1 = LOW;
bool aux_ic_input_level4_ripple_adder_4bit_0_2 = LOW;
bool aux_ic_input_level4_ripple_adder_4bit_0_3 = LOW;
bool aux_ic_input_level4_ripple_adder_4bit_0_4 = LOW;
bool aux_ic_input_level4_ripple_adder_4bit_0_5 = LOW;
bool aux_ic_input_level4_ripple_adder_4bit_0_6 = LOW;
bool aux_ic_input_level4_ripple_adder_4bit_0_7 = LOW;
bool aux_ic_input_level4_ripple_adder_4bit_0_8 = LOW;
// End IC: LEVEL4_RIPPLE_ADDER_4BIT

void setup() {
    pinMode(input_switch1, INPUT);
    pinMode(input_switch2, INPUT);
    pinMode(input_switch3, INPUT);
    pinMode(input_switch4, INPUT);
    pinMode(input_switch5, INPUT);
    pinMode(input_switch6, INPUT);
    pinMode(input_switch7, INPUT);
    pinMode(input_switch8, INPUT);
    pinMode(input_switch9, INPUT);
    pinMode(led1_1, OUTPUT);
    pinMode(led2_1, OUTPUT);
    pinMode(led3_1, OUTPUT);
    pinMode(led4_1, OUTPUT);
    pinMode(led5_1, OUTPUT);
}

void computeLogic() {
    // Assigning aux variables. //
    // IC: LEVEL4_RIPPLE_ADDER_4BIT
    aux_ic_input_level4_ripple_adder_4bit_0_0 = input_switch1_val;
    aux_ic_input_level4_ripple_adder_4bit_0_1 = input_switch2_val;
    aux_ic_input_level4_ripple_adder_4bit_0_2 = input_switch3_val;
    aux_ic_input_level4_ripple_adder_4bit_0_3 = input_switch4_val;
    aux_ic_input_level4_ripple_adder_4bit_0_4 = input_switch5_val;
    aux_ic_input_level4_ripple_adder_4bit_0_5 = input_switch6_val;
    aux_ic_input_level4_ripple_adder_4bit_0_6 = input_switch7_val;
    aux_ic_input_level4_ripple_adder_4bit_0_7 = input_switch8_val;
    aux_ic_input_level4_ripple_adder_4bit_0_8 = input_switch9_val;
    aux_level4_ripple_adder_4bit_0_node_0 = aux_ic_input_level4_ripple_adder_4bit_0_0;
    aux_level4_ripple_adder_4bit_0_node_4 = aux_ic_input_level4_ripple_adder_4bit_0_2;
    aux_level4_ripple_adder_4bit_0_node_8 = aux_ic_input_level4_ripple_adder_4bit_0_5;
    aux_level4_ripple_adder_4bit_0_node_1 = aux_ic_input_level4_ripple_adder_4bit_0_1;
    aux_level4_ripple_adder_4bit_0_node_5 = aux_ic_input_level4_ripple_adder_4bit_0_4;
    // IC: FA[0]
    aux_ic_input_fa0_9_0 = aux_level4_ripple_adder_4bit_0_node_0;
    aux_ic_input_fa0_9_1 = aux_level4_ripple_adder_4bit_0_node_4;
    aux_ic_input_fa0_9_2 = aux_level4_ripple_adder_4bit_0_node_8;
    aux_fa0_9_node_0 = aux_ic_input_fa0_9_0;
    aux_fa0_9_node_1 = aux_ic_input_fa0_9_1;
    aux_fa0_9_node_2 = aux_ic_input_fa0_9_2;
    aux_fa0_9_xor_3 = aux_fa0_9_node_0 ^ aux_fa0_9_node_1;
    aux_fa0_9_and_4 = aux_fa0_9_node_0 && aux_fa0_9_node_1;
    aux_fa0_9_and_6 = aux_fa0_9_xor_3 && aux_fa0_9_node_2;
    aux_fa0_9_xor_5 = aux_fa0_9_xor_3 ^ aux_fa0_9_node_2;
    aux_fa0_9_or_7 = aux_fa0_9_and_4 || aux_fa0_9_and_6;
    aux_fa0_9_node_8 = aux_fa0_9_xor_5;
    aux_fa0_9_node_9 = aux_fa0_9_or_7;
    aux_fa0_9_sum = aux_fa0_9_node_8;
    aux_fa0_9_cout_1 = aux_fa0_9_node_9;
    // End IC: FA[0]
    aux_level4_ripple_adder_4bit_0_node_2 = aux_ic_input_level4_ripple_adder_4bit_0_3;
    aux_level4_ripple_adder_4bit_0_node_6 = aux_ic_input_level4_ripple_adder_4bit_0_7;
    // IC: FA[1]
    aux_ic_input_fa1_10_0 = aux_level4_ripple_adder_4bit_0_node_1;
    aux_ic_input_fa1_10_1 = aux_level4_ripple_adder_4bit_0_node_5;
    aux_ic_input_fa1_10_2 = aux_fa0_9_cout_1;
    aux_fa1_10_node_0 = aux_ic_input_fa1_10_0;
    aux_fa1_10_node_1 = aux_ic_input_fa1_10_1;
    aux_fa1_10_node_2 = aux_ic_input_fa1_10_2;
    aux_fa1_10_xor_3 = aux_fa1_10_node_0 ^ aux_fa1_10_node_1;
    aux_fa1_10_and_4 = aux_fa1_10_node_0 && aux_fa1_10_node_1;
    aux_fa1_10_and_6 = aux_fa1_10_xor_3 && aux_fa1_10_node_2;
    aux_fa1_10_xor_5 = aux_fa1_10_xor_3 ^ aux_fa1_10_node_2;
    aux_fa1_10_or_7 = aux_fa1_10_and_4 || aux_fa1_10_and_6;
    aux_fa1_10_node_8 = aux_fa1_10_xor_5;
    aux_fa1_10_node_9 = aux_fa1_10_or_7;
    aux_fa1_10_sum = aux_fa1_10_node_8;
    aux_fa1_10_cout_1 = aux_fa1_10_node_9;
    // End IC: FA[1]
    aux_level4_ripple_adder_4bit_0_node_3 = aux_ic_input_level4_ripple_adder_4bit_0_6;
    aux_level4_ripple_adder_4bit_0_node_7 = aux_ic_input_level4_ripple_adder_4bit_0_8;
    // IC: FA[2]
    aux_ic_input_fa2_11_0 = aux_level4_ripple_adder_4bit_0_node_2;
    aux_ic_input_fa2_11_1 = aux_level4_ripple_adder_4bit_0_node_6;
    aux_ic_input_fa2_11_2 = aux_fa1_10_cout_1;
    aux_fa2_11_node_0 = aux_ic_input_fa2_11_0;
    aux_fa2_11_node_1 = aux_ic_input_fa2_11_1;
    aux_fa2_11_node_2 = aux_ic_input_fa2_11_2;
    aux_fa2_11_xor_3 = aux_fa2_11_node_0 ^ aux_fa2_11_node_1;
    aux_fa2_11_and_4 = aux_fa2_11_node_0 && aux_fa2_11_node_1;
    aux_fa2_11_and_6 = aux_fa2_11_xor_3 && aux_fa2_11_node_2;
    aux_fa2_11_xor_5 = aux_fa2_11_xor_3 ^ aux_fa2_11_node_2;
    aux_fa2_11_or_7 = aux_fa2_11_and_4 || aux_fa2_11_and_6;
    aux_fa2_11_node_8 = aux_fa2_11_xor_5;
    aux_fa2_11_node_9 = aux_fa2_11_or_7;
    aux_fa2_11_sum = aux_fa2_11_node_8;
    aux_fa2_11_cout_1 = aux_fa2_11_node_9;
    // End IC: FA[2]
    // IC: FA[3]
    aux_ic_input_fa3_12_0 = aux_level4_ripple_adder_4bit_0_node_3;
    aux_ic_input_fa3_12_1 = aux_level4_ripple_adder_4bit_0_node_7;
    aux_ic_input_fa3_12_2 = aux_fa2_11_cout_1;
    aux_fa3_12_node_0 = aux_ic_input_fa3_12_0;
    aux_fa3_12_node_1 = aux_ic_input_fa3_12_1;
    aux_fa3_12_node_2 = aux_ic_input_fa3_12_2;
    aux_fa3_12_xor_3 = aux_fa3_12_node_0 ^ aux_fa3_12_node_1;
    aux_fa3_12_and_4 = aux_fa3_12_node_0 && aux_fa3_12_node_1;
    aux_fa3_12_and_6 = aux_fa3_12_xor_3 && aux_fa3_12_node_2;
    aux_fa3_12_xor_5 = aux_fa3_12_xor_3 ^ aux_fa3_12_node_2;
    aux_fa3_12_or_7 = aux_fa3_12_and_4 || aux_fa3_12_and_6;
    aux_fa3_12_node_8 = aux_fa3_12_xor_5;
    aux_fa3_12_node_9 = aux_fa3_12_or_7;
    aux_fa3_12_sum = aux_fa3_12_node_8;
    aux_fa3_12_cout_1 = aux_fa3_12_node_9;
    // End IC: FA[3]
    aux_level4_ripple_adder_4bit_0_node_13 = aux_fa0_9_sum;
    aux_level4_ripple_adder_4bit_0_node_14 = aux_fa1_10_sum;
    aux_level4_ripple_adder_4bit_0_node_15 = aux_fa2_11_sum;
    aux_level4_ripple_adder_4bit_0_node_16 = aux_fa3_12_sum;
    aux_level4_ripple_adder_4bit_0_node_17 = aux_fa3_12_cout_1;
    aux_level4_ripple_adder_4bit_0_sum0 = aux_level4_ripple_adder_4bit_0_node_13;
    aux_level4_ripple_adder_4bit_0_sum1_1 = aux_level4_ripple_adder_4bit_0_node_14;
    aux_level4_ripple_adder_4bit_0_sum2_2 = aux_level4_ripple_adder_4bit_0_node_15;
    aux_level4_ripple_adder_4bit_0_carryout_3 = aux_level4_ripple_adder_4bit_0_node_17;
    aux_level4_ripple_adder_4bit_0_sum3_4 = aux_level4_ripple_adder_4bit_0_node_16;
    // End IC: LEVEL4_RIPPLE_ADDER_4BIT
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
    input_switch9_val = digitalRead(input_switch9);

    // Updating clocks. //

    computeLogic();

    // Writing output data. //
    digitalWrite(led1_1, aux_level4_ripple_adder_4bit_0_sum0);
    digitalWrite(led2_1, aux_level4_ripple_adder_4bit_0_sum1_1);
    digitalWrite(led3_1, aux_level4_ripple_adder_4bit_0_sum2_2);
    digitalWrite(led4_1, aux_level4_ripple_adder_4bit_0_carryout_3);
    digitalWrite(led5_1, aux_level4_ripple_adder_4bit_0_sum3_4);
}
