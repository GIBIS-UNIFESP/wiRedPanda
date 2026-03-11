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
const int led1_0 = A4;
const int led2_0 = A5;
const int led3_0 = A6;
const int led4_0 = A7;
const int led5_0 = A8;
const int led6_0 = A9;
const int led7_0 = A10;
const int led8_0 = A11;
const int led9_0 = A12;
const int led10_0 = A13;
const int led11_0 = A14;
const int led12_0 = A15;
const int led13_0 = 2;
const int led14_0 = 3;
const int led15_0 = 4;
const int led16_0 = 5;

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
bool aux_ic_input_level2_decoder_4to16_0 = LOW;
bool aux_ic_input_level2_decoder_4to16_1 = LOW;
bool aux_ic_input_level2_decoder_4to16_2 = LOW;
bool aux_ic_input_level2_decoder_4to16_3 = LOW;
// End IC: LEVEL2_DECODER_4TO16

void setup() {
    pinMode(input_switch1, INPUT);
    pinMode(input_switch2, INPUT);
    pinMode(input_switch3, INPUT);
    pinMode(input_switch4, INPUT);
    pinMode(led1_0, OUTPUT);
    pinMode(led2_0, OUTPUT);
    pinMode(led3_0, OUTPUT);
    pinMode(led4_0, OUTPUT);
    pinMode(led5_0, OUTPUT);
    pinMode(led6_0, OUTPUT);
    pinMode(led7_0, OUTPUT);
    pinMode(led8_0, OUTPUT);
    pinMode(led9_0, OUTPUT);
    pinMode(led10_0, OUTPUT);
    pinMode(led11_0, OUTPUT);
    pinMode(led12_0, OUTPUT);
    pinMode(led13_0, OUTPUT);
    pinMode(led14_0, OUTPUT);
    pinMode(led15_0, OUTPUT);
    pinMode(led16_0, OUTPUT);
}

void computeLogic() {
    // Assigning aux variables. //
    // IC: LEVEL2_DECODER_4TO16
    aux_ic_input_level2_decoder_4to16_0 = input_switch1_val;
    aux_ic_input_level2_decoder_4to16_1 = input_switch2_val;
    aux_ic_input_level2_decoder_4to16_2 = input_switch3_val;
    aux_ic_input_level2_decoder_4to16_3 = input_switch4_val;
    aux_level2_decoder_4to16_0_node_0 = aux_ic_input_level2_decoder_4to16_0;
    aux_level2_decoder_4to16_0_node_1 = aux_ic_input_level2_decoder_4to16_1;
    aux_level2_decoder_4to16_0_node_2 = aux_ic_input_level2_decoder_4to16_2;
    aux_level2_decoder_4to16_0_node_3 = aux_ic_input_level2_decoder_4to16_3;
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
    digitalWrite(led1_0, aux_level2_decoder_4to16_0_out0);
    digitalWrite(led2_0, aux_level2_decoder_4to16_0_out1_1);
    digitalWrite(led3_0, aux_level2_decoder_4to16_0_out2_2);
    digitalWrite(led4_0, aux_level2_decoder_4to16_0_out3_3);
    digitalWrite(led5_0, aux_level2_decoder_4to16_0_out4_4);
    digitalWrite(led6_0, aux_level2_decoder_4to16_0_out5_5);
    digitalWrite(led7_0, aux_level2_decoder_4to16_0_out6_6);
    digitalWrite(led8_0, aux_level2_decoder_4to16_0_out7_7);
    digitalWrite(led9_0, aux_level2_decoder_4to16_0_out8_8);
    digitalWrite(led10_0, aux_level2_decoder_4to16_0_out9_9);
    digitalWrite(led11_0, aux_level2_decoder_4to16_0_out10_10);
    digitalWrite(led12_0, aux_level2_decoder_4to16_0_out11_11);
    digitalWrite(led13_0, aux_level2_decoder_4to16_0_out12_12);
    digitalWrite(led14_0, aux_level2_decoder_4to16_0_out13_13);
    digitalWrite(led15_0, aux_level2_decoder_4to16_0_out14_14);
    digitalWrite(led16_0, aux_level2_decoder_4to16_0_out15_15);
}
