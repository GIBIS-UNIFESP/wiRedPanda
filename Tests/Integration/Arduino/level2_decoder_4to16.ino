// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //
//
// Target Board: Arduino Mega 2560
// Pin Usage: 20/68 pins
//


#include <elapsedMillis.h>
/* ========= Inputs ========== */
const int input_switch1 = A0;
const int input_switch2 = A1;
const int input_switch3 = A2;
const int input_switch4 = A3;

/* ========= Outputs ========== */
const int led1_1 = A4;
const int led2_1 = A5;
const int led3_1 = A6;
const int led4_1 = A7;
const int led5_1 = A8;
const int led6_1 = A9;
const int led7_1 = A10;
const int led8_1 = A11;
const int led9_1 = A12;
const int led10_1 = A13;
const int led11_1 = A14;
const int led12_1 = A15;
const int led13_1 = 2;
const int led14_1 = 3;
const int led15_1 = 4;
const int led16_1 = 5;

/* ====== Aux. Variables ====== */
bool input_switch1_val = LOW;
bool input_switch2_val = LOW;
bool input_switch3_val = LOW;
bool input_switch4_val = LOW;
// IC: LEVEL2_DECODER_4TO16
bool aux_level2_decoder_4to16_0_out0 = LOW;
bool aux_level2_decoder_4to16_0_out1_1 = LOW;
bool aux_level2_decoder_4to16_0_out2_2 = LOW;
bool aux_level2_decoder_4to16_0_out3_3 = LOW;
bool aux_level2_decoder_4to16_0_out4_4 = LOW;
bool aux_level2_decoder_4to16_0_out5_5 = LOW;
bool aux_level2_decoder_4to16_0_out6_6 = LOW;
bool aux_level2_decoder_4to16_0_out7_7 = LOW;
bool aux_level2_decoder_4to16_0_out8_8 = LOW;
bool aux_level2_decoder_4to16_0_out9_9 = LOW;
bool aux_level2_decoder_4to16_0_out10_10 = LOW;
bool aux_level2_decoder_4to16_0_out11_11 = LOW;
bool aux_level2_decoder_4to16_0_out12_12 = LOW;
bool aux_level2_decoder_4to16_0_out13_13 = LOW;
bool aux_level2_decoder_4to16_0_out14_14 = LOW;
bool aux_level2_decoder_4to16_0_out15_15 = LOW;
bool aux_level2_decoder_4to16_0_node_0 = LOW;
bool aux_level2_decoder_4to16_0_node_1 = LOW;
bool aux_level2_decoder_4to16_0_node_2 = LOW;
bool aux_level2_decoder_4to16_0_node_3 = LOW;
bool aux_level2_decoder_4to16_0_not_4 = LOW;
bool aux_level2_decoder_4to16_0_not_5 = LOW;
bool aux_level2_decoder_4to16_0_not_6 = LOW;
bool aux_level2_decoder_4to16_0_not_7 = LOW;
bool aux_level2_decoder_4to16_0_and_8 = LOW;
bool aux_level2_decoder_4to16_0_node_9 = LOW;
bool aux_level2_decoder_4to16_0_and_10 = LOW;
bool aux_level2_decoder_4to16_0_node_11 = LOW;
bool aux_level2_decoder_4to16_0_and_12 = LOW;
bool aux_level2_decoder_4to16_0_node_13 = LOW;
bool aux_level2_decoder_4to16_0_and_14 = LOW;
bool aux_level2_decoder_4to16_0_node_15 = LOW;
bool aux_level2_decoder_4to16_0_and_16 = LOW;
bool aux_level2_decoder_4to16_0_node_17 = LOW;
bool aux_level2_decoder_4to16_0_and_18 = LOW;
bool aux_level2_decoder_4to16_0_node_19 = LOW;
bool aux_level2_decoder_4to16_0_and_20 = LOW;
bool aux_level2_decoder_4to16_0_node_21 = LOW;
bool aux_level2_decoder_4to16_0_and_22 = LOW;
bool aux_level2_decoder_4to16_0_node_23 = LOW;
bool aux_level2_decoder_4to16_0_and_24 = LOW;
bool aux_level2_decoder_4to16_0_node_25 = LOW;
bool aux_level2_decoder_4to16_0_and_26 = LOW;
bool aux_level2_decoder_4to16_0_node_27 = LOW;
bool aux_level2_decoder_4to16_0_and_28 = LOW;
bool aux_level2_decoder_4to16_0_node_29 = LOW;
bool aux_level2_decoder_4to16_0_and_30 = LOW;
bool aux_level2_decoder_4to16_0_node_31 = LOW;
bool aux_level2_decoder_4to16_0_and_32 = LOW;
bool aux_level2_decoder_4to16_0_node_33 = LOW;
bool aux_level2_decoder_4to16_0_and_34 = LOW;
bool aux_level2_decoder_4to16_0_node_35 = LOW;
bool aux_level2_decoder_4to16_0_and_36 = LOW;
bool aux_level2_decoder_4to16_0_node_37 = LOW;
bool aux_level2_decoder_4to16_0_and_38 = LOW;
bool aux_level2_decoder_4to16_0_node_39 = LOW;
bool aux_ic_input_level2_decoder_4to16_0_0 = LOW;
bool aux_ic_input_level2_decoder_4to16_0_1 = LOW;
bool aux_ic_input_level2_decoder_4to16_0_2 = LOW;
bool aux_ic_input_level2_decoder_4to16_0_3 = LOW;
// End IC: LEVEL2_DECODER_4TO16

void setup() {
    pinMode(input_switch1, INPUT);
    pinMode(input_switch2, INPUT);
    pinMode(input_switch3, INPUT);
    pinMode(input_switch4, INPUT);
    pinMode(led1_1, OUTPUT);
    pinMode(led2_1, OUTPUT);
    pinMode(led3_1, OUTPUT);
    pinMode(led4_1, OUTPUT);
    pinMode(led5_1, OUTPUT);
    pinMode(led6_1, OUTPUT);
    pinMode(led7_1, OUTPUT);
    pinMode(led8_1, OUTPUT);
    pinMode(led9_1, OUTPUT);
    pinMode(led10_1, OUTPUT);
    pinMode(led11_1, OUTPUT);
    pinMode(led12_1, OUTPUT);
    pinMode(led13_1, OUTPUT);
    pinMode(led14_1, OUTPUT);
    pinMode(led15_1, OUTPUT);
    pinMode(led16_1, OUTPUT);
}

void computeLogic() {
    // Assigning aux variables. //
    // IC: LEVEL2_DECODER_4TO16
    aux_ic_input_level2_decoder_4to16_0_0 = input_switch1_val;
    aux_ic_input_level2_decoder_4to16_0_1 = input_switch2_val;
    aux_ic_input_level2_decoder_4to16_0_2 = input_switch3_val;
    aux_ic_input_level2_decoder_4to16_0_3 = input_switch4_val;
    aux_level2_decoder_4to16_0_node_0 = aux_ic_input_level2_decoder_4to16_0_0;
    aux_level2_decoder_4to16_0_node_1 = aux_ic_input_level2_decoder_4to16_0_1;
    aux_level2_decoder_4to16_0_node_2 = aux_ic_input_level2_decoder_4to16_0_2;
    aux_level2_decoder_4to16_0_node_3 = aux_ic_input_level2_decoder_4to16_0_3;
    aux_level2_decoder_4to16_0_not_4 = !aux_level2_decoder_4to16_0_node_0;
    aux_level2_decoder_4to16_0_not_5 = !aux_level2_decoder_4to16_0_node_1;
    aux_level2_decoder_4to16_0_not_6 = !aux_level2_decoder_4to16_0_node_2;
    aux_level2_decoder_4to16_0_not_7 = !aux_level2_decoder_4to16_0_node_3;
    aux_level2_decoder_4to16_0_and_8 = aux_level2_decoder_4to16_0_not_4 && aux_level2_decoder_4to16_0_not_5 && aux_level2_decoder_4to16_0_not_6 && aux_level2_decoder_4to16_0_not_7;
    aux_level2_decoder_4to16_0_and_10 = aux_level2_decoder_4to16_0_node_0 && aux_level2_decoder_4to16_0_not_5 && aux_level2_decoder_4to16_0_not_6 && aux_level2_decoder_4to16_0_not_7;
    aux_level2_decoder_4to16_0_and_12 = aux_level2_decoder_4to16_0_not_4 && aux_level2_decoder_4to16_0_node_1 && aux_level2_decoder_4to16_0_not_6 && aux_level2_decoder_4to16_0_not_7;
    aux_level2_decoder_4to16_0_and_14 = aux_level2_decoder_4to16_0_node_0 && aux_level2_decoder_4to16_0_node_1 && aux_level2_decoder_4to16_0_not_6 && aux_level2_decoder_4to16_0_not_7;
    aux_level2_decoder_4to16_0_and_16 = aux_level2_decoder_4to16_0_not_4 && aux_level2_decoder_4to16_0_not_5 && aux_level2_decoder_4to16_0_node_2 && aux_level2_decoder_4to16_0_not_7;
    aux_level2_decoder_4to16_0_and_18 = aux_level2_decoder_4to16_0_node_0 && aux_level2_decoder_4to16_0_not_5 && aux_level2_decoder_4to16_0_node_2 && aux_level2_decoder_4to16_0_not_7;
    aux_level2_decoder_4to16_0_and_20 = aux_level2_decoder_4to16_0_not_4 && aux_level2_decoder_4to16_0_node_1 && aux_level2_decoder_4to16_0_node_2 && aux_level2_decoder_4to16_0_not_7;
    aux_level2_decoder_4to16_0_and_22 = aux_level2_decoder_4to16_0_node_0 && aux_level2_decoder_4to16_0_node_1 && aux_level2_decoder_4to16_0_node_2 && aux_level2_decoder_4to16_0_not_7;
    aux_level2_decoder_4to16_0_and_24 = aux_level2_decoder_4to16_0_not_4 && aux_level2_decoder_4to16_0_not_5 && aux_level2_decoder_4to16_0_not_6 && aux_level2_decoder_4to16_0_node_3;
    aux_level2_decoder_4to16_0_and_26 = aux_level2_decoder_4to16_0_node_0 && aux_level2_decoder_4to16_0_not_5 && aux_level2_decoder_4to16_0_not_6 && aux_level2_decoder_4to16_0_node_3;
    aux_level2_decoder_4to16_0_and_28 = aux_level2_decoder_4to16_0_not_4 && aux_level2_decoder_4to16_0_node_1 && aux_level2_decoder_4to16_0_not_6 && aux_level2_decoder_4to16_0_node_3;
    aux_level2_decoder_4to16_0_and_30 = aux_level2_decoder_4to16_0_node_0 && aux_level2_decoder_4to16_0_node_1 && aux_level2_decoder_4to16_0_not_6 && aux_level2_decoder_4to16_0_node_3;
    aux_level2_decoder_4to16_0_and_32 = aux_level2_decoder_4to16_0_not_4 && aux_level2_decoder_4to16_0_not_5 && aux_level2_decoder_4to16_0_node_2 && aux_level2_decoder_4to16_0_node_3;
    aux_level2_decoder_4to16_0_and_34 = aux_level2_decoder_4to16_0_node_0 && aux_level2_decoder_4to16_0_not_5 && aux_level2_decoder_4to16_0_node_2 && aux_level2_decoder_4to16_0_node_3;
    aux_level2_decoder_4to16_0_and_36 = aux_level2_decoder_4to16_0_not_4 && aux_level2_decoder_4to16_0_node_1 && aux_level2_decoder_4to16_0_node_2 && aux_level2_decoder_4to16_0_node_3;
    aux_level2_decoder_4to16_0_and_38 = aux_level2_decoder_4to16_0_node_0 && aux_level2_decoder_4to16_0_node_1 && aux_level2_decoder_4to16_0_node_2 && aux_level2_decoder_4to16_0_node_3;
    aux_level2_decoder_4to16_0_node_9 = aux_level2_decoder_4to16_0_and_8;
    aux_level2_decoder_4to16_0_node_11 = aux_level2_decoder_4to16_0_and_10;
    aux_level2_decoder_4to16_0_node_13 = aux_level2_decoder_4to16_0_and_12;
    aux_level2_decoder_4to16_0_node_15 = aux_level2_decoder_4to16_0_and_14;
    aux_level2_decoder_4to16_0_node_17 = aux_level2_decoder_4to16_0_and_16;
    aux_level2_decoder_4to16_0_node_19 = aux_level2_decoder_4to16_0_and_18;
    aux_level2_decoder_4to16_0_node_21 = aux_level2_decoder_4to16_0_and_20;
    aux_level2_decoder_4to16_0_node_23 = aux_level2_decoder_4to16_0_and_22;
    aux_level2_decoder_4to16_0_node_25 = aux_level2_decoder_4to16_0_and_24;
    aux_level2_decoder_4to16_0_node_27 = aux_level2_decoder_4to16_0_and_26;
    aux_level2_decoder_4to16_0_node_29 = aux_level2_decoder_4to16_0_and_28;
    aux_level2_decoder_4to16_0_node_31 = aux_level2_decoder_4to16_0_and_30;
    aux_level2_decoder_4to16_0_node_33 = aux_level2_decoder_4to16_0_and_32;
    aux_level2_decoder_4to16_0_node_35 = aux_level2_decoder_4to16_0_and_34;
    aux_level2_decoder_4to16_0_node_37 = aux_level2_decoder_4to16_0_and_36;
    aux_level2_decoder_4to16_0_node_39 = aux_level2_decoder_4to16_0_and_38;
    aux_level2_decoder_4to16_0_out0 = aux_level2_decoder_4to16_0_node_9;
    aux_level2_decoder_4to16_0_out1_1 = aux_level2_decoder_4to16_0_node_11;
    aux_level2_decoder_4to16_0_out2_2 = aux_level2_decoder_4to16_0_node_13;
    aux_level2_decoder_4to16_0_out3_3 = aux_level2_decoder_4to16_0_node_15;
    aux_level2_decoder_4to16_0_out4_4 = aux_level2_decoder_4to16_0_node_17;
    aux_level2_decoder_4to16_0_out5_5 = aux_level2_decoder_4to16_0_node_19;
    aux_level2_decoder_4to16_0_out6_6 = aux_level2_decoder_4to16_0_node_21;
    aux_level2_decoder_4to16_0_out7_7 = aux_level2_decoder_4to16_0_node_23;
    aux_level2_decoder_4to16_0_out8_8 = aux_level2_decoder_4to16_0_node_25;
    aux_level2_decoder_4to16_0_out9_9 = aux_level2_decoder_4to16_0_node_27;
    aux_level2_decoder_4to16_0_out10_10 = aux_level2_decoder_4to16_0_node_29;
    aux_level2_decoder_4to16_0_out11_11 = aux_level2_decoder_4to16_0_node_31;
    aux_level2_decoder_4to16_0_out12_12 = aux_level2_decoder_4to16_0_node_33;
    aux_level2_decoder_4to16_0_out13_13 = aux_level2_decoder_4to16_0_node_35;
    aux_level2_decoder_4to16_0_out14_14 = aux_level2_decoder_4to16_0_node_37;
    aux_level2_decoder_4to16_0_out15_15 = aux_level2_decoder_4to16_0_node_39;
    // End IC: LEVEL2_DECODER_4TO16
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
    digitalWrite(led1_1, aux_level2_decoder_4to16_0_out0);
    digitalWrite(led2_1, aux_level2_decoder_4to16_0_out1_1);
    digitalWrite(led3_1, aux_level2_decoder_4to16_0_out2_2);
    digitalWrite(led4_1, aux_level2_decoder_4to16_0_out3_3);
    digitalWrite(led5_1, aux_level2_decoder_4to16_0_out4_4);
    digitalWrite(led6_1, aux_level2_decoder_4to16_0_out5_5);
    digitalWrite(led7_1, aux_level2_decoder_4to16_0_out6_6);
    digitalWrite(led8_1, aux_level2_decoder_4to16_0_out7_7);
    digitalWrite(led9_1, aux_level2_decoder_4to16_0_out8_8);
    digitalWrite(led10_1, aux_level2_decoder_4to16_0_out9_9);
    digitalWrite(led11_1, aux_level2_decoder_4to16_0_out10_10);
    digitalWrite(led12_1, aux_level2_decoder_4to16_0_out11_11);
    digitalWrite(led13_1, aux_level2_decoder_4to16_0_out12_12);
    digitalWrite(led14_1, aux_level2_decoder_4to16_0_out13_13);
    digitalWrite(led15_1, aux_level2_decoder_4to16_0_out14_14);
    digitalWrite(led16_1, aux_level2_decoder_4to16_0_out15_15);
}
