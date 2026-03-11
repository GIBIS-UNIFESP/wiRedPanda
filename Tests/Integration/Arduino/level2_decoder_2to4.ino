// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //
//
// Target Board: Arduino UNO R3/R4
// Pin Usage: 6/18 pins
//


#include <elapsedMillis.h>
/* ========= Inputs ========== */
const int input_switch1 = A0;
const int input_switch2 = A1;

/* ========= Outputs ========== */
const int led1_0 = A2;
const int led2_0 = A3;
const int led3_0 = A4;
const int led4_0 = A5;

/* ====== Aux. Variables ====== */
bool input_switch1_val = LOW;
bool input_switch2_val = LOW;
// IC: LEVEL2_DECODER_2TO4
bool aux_level2_decoder_2to4_0_out0 = LOW;
bool aux_level2_decoder_2to4_0_out1_1 = LOW;
bool aux_level2_decoder_2to4_0_out2_2 = LOW;
bool aux_level2_decoder_2to4_0_out3_3 = LOW;
bool aux_level2_decoder_2to4_0_node_0 = LOW;
bool aux_level2_decoder_2to4_0_node_1 = LOW;
bool aux_level2_decoder_2to4_0_not_2 = LOW;
bool aux_level2_decoder_2to4_0_not_3 = LOW;
bool aux_level2_decoder_2to4_0_and_4 = LOW;
bool aux_level2_decoder_2to4_0_node_5 = LOW;
bool aux_level2_decoder_2to4_0_and_6 = LOW;
bool aux_level2_decoder_2to4_0_node_7 = LOW;
bool aux_level2_decoder_2to4_0_and_8 = LOW;
bool aux_level2_decoder_2to4_0_node_9 = LOW;
bool aux_level2_decoder_2to4_0_and_10 = LOW;
bool aux_level2_decoder_2to4_0_node_11 = LOW;
bool aux_ic_input_level2_decoder_2to4_0 = LOW;
bool aux_ic_input_level2_decoder_2to4_1 = LOW;
// End IC: LEVEL2_DECODER_2TO4

void setup() {
    pinMode(input_switch1, INPUT);
    pinMode(input_switch2, INPUT);
    pinMode(led1_0, OUTPUT);
    pinMode(led2_0, OUTPUT);
    pinMode(led3_0, OUTPUT);
    pinMode(led4_0, OUTPUT);
}

void computeLogic() {
    // Assigning aux variables. //
    // IC: LEVEL2_DECODER_2TO4
    aux_ic_input_level2_decoder_2to4_0 = input_switch1_val;
    aux_ic_input_level2_decoder_2to4_1 = input_switch2_val;
    aux_level2_decoder_2to4_0_node_0 = aux_ic_input_level2_decoder_2to4_0;
    aux_level2_decoder_2to4_0_node_1 = aux_ic_input_level2_decoder_2to4_1;
    aux_level2_decoder_2to4_0_not_2 = !aux_level2_decoder_2to4_0_node_0;
    aux_level2_decoder_2to4_0_not_3 = !aux_level2_decoder_2to4_0_node_1;
    aux_level2_decoder_2to4_0_and_4 = aux_level2_decoder_2to4_0_not_2 && aux_level2_decoder_2to4_0_not_3;
    aux_level2_decoder_2to4_0_and_6 = aux_level2_decoder_2to4_0_node_0 && aux_level2_decoder_2to4_0_not_3;
    aux_level2_decoder_2to4_0_and_8 = aux_level2_decoder_2to4_0_not_2 && aux_level2_decoder_2to4_0_node_1;
    aux_level2_decoder_2to4_0_and_10 = aux_level2_decoder_2to4_0_node_0 && aux_level2_decoder_2to4_0_node_1;
    aux_level2_decoder_2to4_0_node_5 = aux_level2_decoder_2to4_0_and_4;
    aux_level2_decoder_2to4_0_node_7 = aux_level2_decoder_2to4_0_and_6;
    aux_level2_decoder_2to4_0_node_9 = aux_level2_decoder_2to4_0_and_8;
    aux_level2_decoder_2to4_0_node_11 = aux_level2_decoder_2to4_0_and_10;
    aux_level2_decoder_2to4_0_out0 = aux_level2_decoder_2to4_0_node_5;
    aux_level2_decoder_2to4_0_out1_1 = aux_level2_decoder_2to4_0_node_7;
    aux_level2_decoder_2to4_0_out2_2 = aux_level2_decoder_2to4_0_node_9;
    aux_level2_decoder_2to4_0_out3_3 = aux_level2_decoder_2to4_0_node_11;
    // End IC: LEVEL2_DECODER_2TO4
}

void loop() {
    // Reading input data //.
    input_switch1_val = digitalRead(input_switch1);
    input_switch2_val = digitalRead(input_switch2);

    // Updating clocks. //

    computeLogic();

    // Writing output data. //
    digitalWrite(led1_0, aux_level2_decoder_2to4_0_out0);
    digitalWrite(led2_0, aux_level2_decoder_2to4_0_out1_1);
    digitalWrite(led3_0, aux_level2_decoder_2to4_0_out2_2);
    digitalWrite(led4_0, aux_level2_decoder_2to4_0_out3_3);
}
