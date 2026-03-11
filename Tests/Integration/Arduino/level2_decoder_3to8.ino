// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //
//
// Target Board: Arduino UNO R3/R4
// Pin Usage: 11/18 pins
//


#include <elapsedMillis.h>
/* ========= Inputs ========== */
const int input_switch1 = A0;
const int input_switch2 = A1;
const int input_switch3 = A2;

/* ========= Outputs ========== */
const int led1_0 = A3;
const int led2_0 = A4;
const int led3_0 = A5;
const int led4_0 = 2;
const int led5_0 = 3;
const int led6_0 = 4;
const int led7_0 = 5;
const int led8_0 = 6;

/* ====== Aux. Variables ====== */
bool input_switch1_val = LOW;
bool input_switch2_val = LOW;
bool input_switch3_val = LOW;
// IC: LEVEL2_DECODER_3TO8
bool aux_level2_decoder_3to8_0_out0 = LOW;
bool aux_level2_decoder_3to8_0_out1_1 = LOW;
bool aux_level2_decoder_3to8_0_out2_2 = LOW;
bool aux_level2_decoder_3to8_0_out3_3 = LOW;
bool aux_level2_decoder_3to8_0_out4_4 = LOW;
bool aux_level2_decoder_3to8_0_out5_5 = LOW;
bool aux_level2_decoder_3to8_0_out6_6 = LOW;
bool aux_level2_decoder_3to8_0_out7_7 = LOW;
bool aux_level2_decoder_3to8_0_node_0 = LOW;
bool aux_level2_decoder_3to8_0_node_1 = LOW;
bool aux_level2_decoder_3to8_0_node_2 = LOW;
bool aux_level2_decoder_3to8_0_not_3 = LOW;
bool aux_level2_decoder_3to8_0_not_4 = LOW;
bool aux_level2_decoder_3to8_0_not_5 = LOW;
bool aux_level2_decoder_3to8_0_and_6 = LOW;
bool aux_level2_decoder_3to8_0_node_7 = LOW;
bool aux_level2_decoder_3to8_0_and_8 = LOW;
bool aux_level2_decoder_3to8_0_node_9 = LOW;
bool aux_level2_decoder_3to8_0_and_10 = LOW;
bool aux_level2_decoder_3to8_0_node_11 = LOW;
bool aux_level2_decoder_3to8_0_and_12 = LOW;
bool aux_level2_decoder_3to8_0_node_13 = LOW;
bool aux_level2_decoder_3to8_0_and_14 = LOW;
bool aux_level2_decoder_3to8_0_node_15 = LOW;
bool aux_level2_decoder_3to8_0_and_16 = LOW;
bool aux_level2_decoder_3to8_0_node_17 = LOW;
bool aux_level2_decoder_3to8_0_and_18 = LOW;
bool aux_level2_decoder_3to8_0_node_19 = LOW;
bool aux_level2_decoder_3to8_0_and_20 = LOW;
bool aux_level2_decoder_3to8_0_node_21 = LOW;
bool aux_ic_input_level2_decoder_3to8_0 = LOW;
bool aux_ic_input_level2_decoder_3to8_1 = LOW;
bool aux_ic_input_level2_decoder_3to8_2 = LOW;
// End IC: LEVEL2_DECODER_3TO8

void setup() {
    pinMode(input_switch1, INPUT);
    pinMode(input_switch2, INPUT);
    pinMode(input_switch3, INPUT);
    pinMode(led1_0, OUTPUT);
    pinMode(led2_0, OUTPUT);
    pinMode(led3_0, OUTPUT);
    pinMode(led4_0, OUTPUT);
    pinMode(led5_0, OUTPUT);
    pinMode(led6_0, OUTPUT);
    pinMode(led7_0, OUTPUT);
    pinMode(led8_0, OUTPUT);
}

void computeLogic() {
    // Assigning aux variables. //
    // IC: LEVEL2_DECODER_3TO8
    aux_ic_input_level2_decoder_3to8_0 = input_switch1_val;
    aux_ic_input_level2_decoder_3to8_1 = input_switch2_val;
    aux_ic_input_level2_decoder_3to8_2 = input_switch3_val;
    aux_level2_decoder_3to8_0_node_0 = aux_ic_input_level2_decoder_3to8_0;
    aux_level2_decoder_3to8_0_node_1 = aux_ic_input_level2_decoder_3to8_1;
    aux_level2_decoder_3to8_0_node_2 = aux_ic_input_level2_decoder_3to8_2;
    aux_level2_decoder_3to8_0_not_3 = !aux_level2_decoder_3to8_0_node_0;
    aux_level2_decoder_3to8_0_not_4 = !aux_level2_decoder_3to8_0_node_1;
    aux_level2_decoder_3to8_0_not_5 = !aux_level2_decoder_3to8_0_node_2;
    aux_level2_decoder_3to8_0_and_6 = aux_level2_decoder_3to8_0_not_3 && aux_level2_decoder_3to8_0_not_4 && aux_level2_decoder_3to8_0_not_5;
    aux_level2_decoder_3to8_0_and_8 = aux_level2_decoder_3to8_0_node_0 && aux_level2_decoder_3to8_0_not_4 && aux_level2_decoder_3to8_0_not_5;
    aux_level2_decoder_3to8_0_and_10 = aux_level2_decoder_3to8_0_not_3 && aux_level2_decoder_3to8_0_node_1 && aux_level2_decoder_3to8_0_not_5;
    aux_level2_decoder_3to8_0_and_12 = aux_level2_decoder_3to8_0_node_0 && aux_level2_decoder_3to8_0_node_1 && aux_level2_decoder_3to8_0_not_5;
    aux_level2_decoder_3to8_0_and_14 = aux_level2_decoder_3to8_0_not_3 && aux_level2_decoder_3to8_0_not_4 && aux_level2_decoder_3to8_0_node_2;
    aux_level2_decoder_3to8_0_and_16 = aux_level2_decoder_3to8_0_node_0 && aux_level2_decoder_3to8_0_not_4 && aux_level2_decoder_3to8_0_node_2;
    aux_level2_decoder_3to8_0_and_18 = aux_level2_decoder_3to8_0_not_3 && aux_level2_decoder_3to8_0_node_1 && aux_level2_decoder_3to8_0_node_2;
    aux_level2_decoder_3to8_0_and_20 = aux_level2_decoder_3to8_0_node_0 && aux_level2_decoder_3to8_0_node_1 && aux_level2_decoder_3to8_0_node_2;
    aux_level2_decoder_3to8_0_node_7 = aux_level2_decoder_3to8_0_and_6;
    aux_level2_decoder_3to8_0_node_9 = aux_level2_decoder_3to8_0_and_8;
    aux_level2_decoder_3to8_0_node_11 = aux_level2_decoder_3to8_0_and_10;
    aux_level2_decoder_3to8_0_node_13 = aux_level2_decoder_3to8_0_and_12;
    aux_level2_decoder_3to8_0_node_15 = aux_level2_decoder_3to8_0_and_14;
    aux_level2_decoder_3to8_0_node_17 = aux_level2_decoder_3to8_0_and_16;
    aux_level2_decoder_3to8_0_node_19 = aux_level2_decoder_3to8_0_and_18;
    aux_level2_decoder_3to8_0_node_21 = aux_level2_decoder_3to8_0_and_20;
    aux_level2_decoder_3to8_0_out0 = aux_level2_decoder_3to8_0_node_7;
    aux_level2_decoder_3to8_0_out1_1 = aux_level2_decoder_3to8_0_node_9;
    aux_level2_decoder_3to8_0_out2_2 = aux_level2_decoder_3to8_0_node_11;
    aux_level2_decoder_3to8_0_out3_3 = aux_level2_decoder_3to8_0_node_13;
    aux_level2_decoder_3to8_0_out4_4 = aux_level2_decoder_3to8_0_node_15;
    aux_level2_decoder_3to8_0_out5_5 = aux_level2_decoder_3to8_0_node_17;
    aux_level2_decoder_3to8_0_out6_6 = aux_level2_decoder_3to8_0_node_19;
    aux_level2_decoder_3to8_0_out7_7 = aux_level2_decoder_3to8_0_node_21;
    // End IC: LEVEL2_DECODER_3TO8
}

void loop() {
    // Reading input data //.
    input_switch1_val = digitalRead(input_switch1);
    input_switch2_val = digitalRead(input_switch2);
    input_switch3_val = digitalRead(input_switch3);

    // Updating clocks. //

    computeLogic();

    // Writing output data. //
    digitalWrite(led1_0, aux_level2_decoder_3to8_0_out0);
    digitalWrite(led2_0, aux_level2_decoder_3to8_0_out1_1);
    digitalWrite(led3_0, aux_level2_decoder_3to8_0_out2_2);
    digitalWrite(led4_0, aux_level2_decoder_3to8_0_out3_3);
    digitalWrite(led5_0, aux_level2_decoder_3to8_0_out4_4);
    digitalWrite(led6_0, aux_level2_decoder_3to8_0_out5_5);
    digitalWrite(led7_0, aux_level2_decoder_3to8_0_out6_6);
    digitalWrite(led8_0, aux_level2_decoder_3to8_0_out7_7);
}
