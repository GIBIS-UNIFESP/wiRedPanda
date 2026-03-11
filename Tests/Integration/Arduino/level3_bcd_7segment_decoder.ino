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
const int input_switch4 = A3;

/* ========= Outputs ========== */
const int led1_0 = A4;
const int led2_0 = A5;
const int led3_0 = 2;
const int led4_0 = 3;
const int led5_0 = 4;
const int led6_0 = 5;
const int led7_0 = 6;

/* ====== Aux. Variables ====== */
bool input_switch1_val = LOW;
bool input_switch2_val = LOW;
bool input_switch3_val = LOW;
bool input_switch4_val = LOW;
// IC: LEVEL3_BCD_7SEGMENT_DECODER
bool aux_level3_bcd_7segment_decoder_0_segment_a = LOW;
bool aux_level3_bcd_7segment_decoder_0_segment_b_1 = LOW;
bool aux_level3_bcd_7segment_decoder_0_segment_c_2 = LOW;
bool aux_level3_bcd_7segment_decoder_0_segment_d_3 = LOW;
bool aux_level3_bcd_7segment_decoder_0_segment_e_4 = LOW;
bool aux_level3_bcd_7segment_decoder_0_segment_f_5 = LOW;
bool aux_level3_bcd_7segment_decoder_0_segment_g_6 = LOW;
bool aux_level3_bcd_7segment_decoder_0_node_0 = LOW;
bool aux_level3_bcd_7segment_decoder_0_node_1 = LOW;
bool aux_level3_bcd_7segment_decoder_0_node_2 = LOW;
bool aux_level3_bcd_7segment_decoder_0_node_3 = LOW;
bool aux_level3_bcd_7segment_decoder_0_node_4 = LOW;
bool aux_level3_bcd_7segment_decoder_0_node_5 = LOW;
bool aux_level3_bcd_7segment_decoder_0_node_6 = LOW;
bool aux_level3_bcd_7segment_decoder_0_node_7 = LOW;
bool aux_level3_bcd_7segment_decoder_0_node_8 = LOW;
bool aux_level3_bcd_7segment_decoder_0_node_9 = LOW;
bool aux_level3_bcd_7segment_decoder_0_node_10 = LOW;
bool aux_level3_bcd_7segment_decoder_0_not_11 = LOW;
bool aux_level3_bcd_7segment_decoder_0_not_12 = LOW;
bool aux_level3_bcd_7segment_decoder_0_not_13 = LOW;
bool aux_level3_bcd_7segment_decoder_0_not_14 = LOW;
bool aux_level3_bcd_7segment_decoder_0_and_15 = LOW;
bool aux_level3_bcd_7segment_decoder_0_and_16 = LOW;
bool aux_level3_bcd_7segment_decoder_0_and_17 = LOW;
bool aux_level3_bcd_7segment_decoder_0_and_18 = LOW;
bool aux_level3_bcd_7segment_decoder_0_and_19 = LOW;
bool aux_level3_bcd_7segment_decoder_0_and_20 = LOW;
bool aux_level3_bcd_7segment_decoder_0_and_21 = LOW;
bool aux_level3_bcd_7segment_decoder_0_and_22 = LOW;
bool aux_level3_bcd_7segment_decoder_0_and_23 = LOW;
bool aux_level3_bcd_7segment_decoder_0_and_24 = LOW;
bool aux_level3_bcd_7segment_decoder_0_or_25 = LOW;
bool aux_level3_bcd_7segment_decoder_0_or_26 = LOW;
bool aux_level3_bcd_7segment_decoder_0_or_27 = LOW;
bool aux_level3_bcd_7segment_decoder_0_or_28 = LOW;
bool aux_level3_bcd_7segment_decoder_0_or_29 = LOW;
bool aux_level3_bcd_7segment_decoder_0_or_30 = LOW;
bool aux_level3_bcd_7segment_decoder_0_or_31 = LOW;
bool aux_level3_bcd_7segment_decoder_0_or_32 = LOW;
bool aux_ic_input_level3_bcd_7segment_decoder_0 = LOW;
bool aux_ic_input_level3_bcd_7segment_decoder_1 = LOW;
bool aux_ic_input_level3_bcd_7segment_decoder_2 = LOW;
bool aux_ic_input_level3_bcd_7segment_decoder_3 = LOW;
// End IC: LEVEL3_BCD_7SEGMENT_DECODER

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
}

void computeLogic() {
    // Assigning aux variables. //
    // IC: LEVEL3_BCD_7SEGMENT_DECODER
    aux_ic_input_level3_bcd_7segment_decoder_0 = input_switch1_val;
    aux_ic_input_level3_bcd_7segment_decoder_1 = input_switch2_val;
    aux_ic_input_level3_bcd_7segment_decoder_2 = input_switch3_val;
    aux_ic_input_level3_bcd_7segment_decoder_3 = input_switch4_val;
    aux_level3_bcd_7segment_decoder_0_node_0 = aux_ic_input_level3_bcd_7segment_decoder_0;
    aux_level3_bcd_7segment_decoder_0_node_1 = aux_ic_input_level3_bcd_7segment_decoder_1;
    aux_level3_bcd_7segment_decoder_0_node_2 = aux_ic_input_level3_bcd_7segment_decoder_2;
    aux_level3_bcd_7segment_decoder_0_node_3 = aux_ic_input_level3_bcd_7segment_decoder_3;
    aux_level3_bcd_7segment_decoder_0_not_11 = !aux_level3_bcd_7segment_decoder_0_node_0;
    aux_level3_bcd_7segment_decoder_0_not_12 = !aux_level3_bcd_7segment_decoder_0_node_1;
    aux_level3_bcd_7segment_decoder_0_not_13 = !aux_level3_bcd_7segment_decoder_0_node_2;
    aux_level3_bcd_7segment_decoder_0_not_14 = !aux_level3_bcd_7segment_decoder_0_node_3;
    aux_level3_bcd_7segment_decoder_0_and_15 = aux_level3_bcd_7segment_decoder_0_not_11 && aux_level3_bcd_7segment_decoder_0_not_12 && aux_level3_bcd_7segment_decoder_0_not_13 && aux_level3_bcd_7segment_decoder_0_not_14;
    aux_level3_bcd_7segment_decoder_0_and_16 = aux_level3_bcd_7segment_decoder_0_node_0 && aux_level3_bcd_7segment_decoder_0_not_12 && aux_level3_bcd_7segment_decoder_0_not_13 && aux_level3_bcd_7segment_decoder_0_not_14;
    aux_level3_bcd_7segment_decoder_0_and_18 = aux_level3_bcd_7segment_decoder_0_node_0 && aux_level3_bcd_7segment_decoder_0_node_1 && aux_level3_bcd_7segment_decoder_0_not_13 && aux_level3_bcd_7segment_decoder_0_not_14;
    aux_level3_bcd_7segment_decoder_0_and_19 = aux_level3_bcd_7segment_decoder_0_not_11 && aux_level3_bcd_7segment_decoder_0_not_12 && aux_level3_bcd_7segment_decoder_0_node_2 && aux_level3_bcd_7segment_decoder_0_not_14;
    aux_level3_bcd_7segment_decoder_0_and_20 = aux_level3_bcd_7segment_decoder_0_node_0 && aux_level3_bcd_7segment_decoder_0_not_12 && aux_level3_bcd_7segment_decoder_0_node_2 && aux_level3_bcd_7segment_decoder_0_not_14;
    aux_level3_bcd_7segment_decoder_0_and_21 = aux_level3_bcd_7segment_decoder_0_not_11 && aux_level3_bcd_7segment_decoder_0_node_1 && aux_level3_bcd_7segment_decoder_0_node_2 && aux_level3_bcd_7segment_decoder_0_not_14;
    aux_level3_bcd_7segment_decoder_0_and_22 = aux_level3_bcd_7segment_decoder_0_node_0 && aux_level3_bcd_7segment_decoder_0_node_1 && aux_level3_bcd_7segment_decoder_0_node_2 && aux_level3_bcd_7segment_decoder_0_not_14;
    aux_level3_bcd_7segment_decoder_0_and_23 = aux_level3_bcd_7segment_decoder_0_not_11 && aux_level3_bcd_7segment_decoder_0_not_12 && aux_level3_bcd_7segment_decoder_0_not_13 && aux_level3_bcd_7segment_decoder_0_node_3;
    aux_level3_bcd_7segment_decoder_0_and_17 = aux_level3_bcd_7segment_decoder_0_not_11 && aux_level3_bcd_7segment_decoder_0_node_1 && aux_level3_bcd_7segment_decoder_0_not_13 && aux_level3_bcd_7segment_decoder_0_not_14;
    aux_level3_bcd_7segment_decoder_0_and_24 = aux_level3_bcd_7segment_decoder_0_node_0 && aux_level3_bcd_7segment_decoder_0_not_12 && aux_level3_bcd_7segment_decoder_0_not_13 && aux_level3_bcd_7segment_decoder_0_node_3;
    aux_level3_bcd_7segment_decoder_0_or_27 = aux_level3_bcd_7segment_decoder_0_and_15 || aux_level3_bcd_7segment_decoder_0_and_16 || aux_level3_bcd_7segment_decoder_0_and_18 || aux_level3_bcd_7segment_decoder_0_and_19 || aux_level3_bcd_7segment_decoder_0_and_20 || aux_level3_bcd_7segment_decoder_0_and_21 || aux_level3_bcd_7segment_decoder_0_and_22 || aux_level3_bcd_7segment_decoder_0_and_23;
    aux_level3_bcd_7segment_decoder_0_or_25 = aux_level3_bcd_7segment_decoder_0_and_15 || aux_level3_bcd_7segment_decoder_0_and_17 || aux_level3_bcd_7segment_decoder_0_and_18 || aux_level3_bcd_7segment_decoder_0_and_20 || aux_level3_bcd_7segment_decoder_0_and_21 || aux_level3_bcd_7segment_decoder_0_and_22 || aux_level3_bcd_7segment_decoder_0_and_23 || aux_level3_bcd_7segment_decoder_0_and_24;
    aux_level3_bcd_7segment_decoder_0_or_26 = aux_level3_bcd_7segment_decoder_0_and_15 || aux_level3_bcd_7segment_decoder_0_and_16 || aux_level3_bcd_7segment_decoder_0_and_17 || aux_level3_bcd_7segment_decoder_0_and_18 || aux_level3_bcd_7segment_decoder_0_and_19 || aux_level3_bcd_7segment_decoder_0_and_22 || aux_level3_bcd_7segment_decoder_0_and_23 || aux_level3_bcd_7segment_decoder_0_and_24;
    aux_level3_bcd_7segment_decoder_0_or_28 = aux_level3_bcd_7segment_decoder_0_and_24 || aux_level3_bcd_7segment_decoder_0_or_27;
    aux_level3_bcd_7segment_decoder_0_or_29 = aux_level3_bcd_7segment_decoder_0_and_15 || aux_level3_bcd_7segment_decoder_0_and_17 || aux_level3_bcd_7segment_decoder_0_and_18 || aux_level3_bcd_7segment_decoder_0_and_20 || aux_level3_bcd_7segment_decoder_0_and_21 || aux_level3_bcd_7segment_decoder_0_and_23 || aux_level3_bcd_7segment_decoder_0_and_24;
    aux_level3_bcd_7segment_decoder_0_or_30 = aux_level3_bcd_7segment_decoder_0_and_15 || aux_level3_bcd_7segment_decoder_0_and_17 || aux_level3_bcd_7segment_decoder_0_and_21 || aux_level3_bcd_7segment_decoder_0_and_23;
    aux_level3_bcd_7segment_decoder_0_or_31 = aux_level3_bcd_7segment_decoder_0_and_15 || aux_level3_bcd_7segment_decoder_0_and_19 || aux_level3_bcd_7segment_decoder_0_and_20 || aux_level3_bcd_7segment_decoder_0_and_21 || aux_level3_bcd_7segment_decoder_0_and_23 || aux_level3_bcd_7segment_decoder_0_and_24;
    aux_level3_bcd_7segment_decoder_0_or_32 = aux_level3_bcd_7segment_decoder_0_and_17 || aux_level3_bcd_7segment_decoder_0_and_18 || aux_level3_bcd_7segment_decoder_0_and_19 || aux_level3_bcd_7segment_decoder_0_and_20 || aux_level3_bcd_7segment_decoder_0_and_21 || aux_level3_bcd_7segment_decoder_0_and_23 || aux_level3_bcd_7segment_decoder_0_and_24;
    aux_level3_bcd_7segment_decoder_0_node_4 = aux_level3_bcd_7segment_decoder_0_or_25;
    aux_level3_bcd_7segment_decoder_0_node_5 = aux_level3_bcd_7segment_decoder_0_or_26;
    aux_level3_bcd_7segment_decoder_0_node_6 = aux_level3_bcd_7segment_decoder_0_or_28;
    aux_level3_bcd_7segment_decoder_0_node_7 = aux_level3_bcd_7segment_decoder_0_or_29;
    aux_level3_bcd_7segment_decoder_0_node_8 = aux_level3_bcd_7segment_decoder_0_or_30;
    aux_level3_bcd_7segment_decoder_0_node_9 = aux_level3_bcd_7segment_decoder_0_or_31;
    aux_level3_bcd_7segment_decoder_0_node_10 = aux_level3_bcd_7segment_decoder_0_or_32;
    aux_level3_bcd_7segment_decoder_0_segment_a = aux_level3_bcd_7segment_decoder_0_node_4;
    aux_level3_bcd_7segment_decoder_0_segment_b_1 = aux_level3_bcd_7segment_decoder_0_node_5;
    aux_level3_bcd_7segment_decoder_0_segment_c_2 = aux_level3_bcd_7segment_decoder_0_node_6;
    aux_level3_bcd_7segment_decoder_0_segment_d_3 = aux_level3_bcd_7segment_decoder_0_node_7;
    aux_level3_bcd_7segment_decoder_0_segment_e_4 = aux_level3_bcd_7segment_decoder_0_node_8;
    aux_level3_bcd_7segment_decoder_0_segment_f_5 = aux_level3_bcd_7segment_decoder_0_node_9;
    aux_level3_bcd_7segment_decoder_0_segment_g_6 = aux_level3_bcd_7segment_decoder_0_node_10;
    // End IC: LEVEL3_BCD_7SEGMENT_DECODER
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
    digitalWrite(led1_0, aux_level3_bcd_7segment_decoder_0_segment_a);
    digitalWrite(led2_0, aux_level3_bcd_7segment_decoder_0_segment_b_1);
    digitalWrite(led3_0, aux_level3_bcd_7segment_decoder_0_segment_c_2);
    digitalWrite(led4_0, aux_level3_bcd_7segment_decoder_0_segment_d_3);
    digitalWrite(led5_0, aux_level3_bcd_7segment_decoder_0_segment_e_4);
    digitalWrite(led6_0, aux_level3_bcd_7segment_decoder_0_segment_f_5);
    digitalWrite(led7_0, aux_level3_bcd_7segment_decoder_0_segment_g_6);
}
