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
const int input_switch5 = A4;
const int input_switch6 = A5;
const int input_switch7 = 2;
const int input_switch8 = 3;

/* ========= Outputs ========== */
const int led1_1 = 4;
const int led2_1 = 5;
const int led3_1 = 6;

/* ====== Aux. Variables ====== */
bool input_switch1_val = LOW;
bool input_switch2_val = LOW;
bool input_switch3_val = LOW;
bool input_switch4_val = LOW;
bool input_switch5_val = LOW;
bool input_switch6_val = LOW;
bool input_switch7_val = LOW;
bool input_switch8_val = LOW;
// IC: LEVEL2_PRIORITY_ENCODER_8TO3
bool aux_level2_priority_encoder_8to3_0_addr0 = LOW;
bool aux_level2_priority_encoder_8to3_0_addr1_1 = LOW;
bool aux_level2_priority_encoder_8to3_0_addr2_2 = LOW;
bool aux_level2_priority_encoder_8to3_0_node_0 = LOW;
bool aux_level2_priority_encoder_8to3_0_node_1 = LOW;
bool aux_level2_priority_encoder_8to3_0_node_2 = LOW;
bool aux_level2_priority_encoder_8to3_0_node_3 = LOW;
bool aux_level2_priority_encoder_8to3_0_node_4 = LOW;
bool aux_level2_priority_encoder_8to3_0_node_5 = LOW;
bool aux_level2_priority_encoder_8to3_0_node_6 = LOW;
bool aux_level2_priority_encoder_8to3_0_node_7 = LOW;
bool aux_level2_priority_encoder_8to3_0_not_8 = LOW;
bool aux_level2_priority_encoder_8to3_0_or_9 = LOW;
bool aux_level2_priority_encoder_8to3_0_or_10 = LOW;
bool aux_level2_priority_encoder_8to3_0_or_11 = LOW;
bool aux_level2_priority_encoder_8to3_0_or_12 = LOW;
bool aux_level2_priority_encoder_8to3_0_or_13 = LOW;
bool aux_level2_priority_encoder_8to3_0_or_14 = LOW;
bool aux_level2_priority_encoder_8to3_0_or_15 = LOW;
bool aux_level2_priority_encoder_8to3_0_not_16 = LOW;
bool aux_level2_priority_encoder_8to3_0_not_17 = LOW;
bool aux_level2_priority_encoder_8to3_0_not_18 = LOW;
bool aux_level2_priority_encoder_8to3_0_not_19 = LOW;
bool aux_level2_priority_encoder_8to3_0_not_20 = LOW;
bool aux_level2_priority_encoder_8to3_0_not_21 = LOW;
bool aux_level2_priority_encoder_8to3_0_and_22 = LOW;
bool aux_level2_priority_encoder_8to3_0_and_23 = LOW;
bool aux_level2_priority_encoder_8to3_0_and_24 = LOW;
bool aux_level2_priority_encoder_8to3_0_and_25 = LOW;
bool aux_level2_priority_encoder_8to3_0_and_26 = LOW;
bool aux_level2_priority_encoder_8to3_0_and_27 = LOW;
bool aux_level2_priority_encoder_8to3_0_and_28 = LOW;
bool aux_level2_priority_encoder_8to3_0_or_29 = LOW;
bool aux_level2_priority_encoder_8to3_0_or_30 = LOW;
bool aux_level2_priority_encoder_8to3_0_or_31 = LOW;
bool aux_level2_priority_encoder_8to3_0_or_32 = LOW;
bool aux_level2_priority_encoder_8to3_0_or_33 = LOW;
bool aux_level2_priority_encoder_8to3_0_or_34 = LOW;
bool aux_level2_priority_encoder_8to3_0_or_35 = LOW;
bool aux_level2_priority_encoder_8to3_0_or_36 = LOW;
bool aux_level2_priority_encoder_8to3_0_or_37 = LOW;
bool aux_level2_priority_encoder_8to3_0_node_38 = LOW;
bool aux_level2_priority_encoder_8to3_0_node_39 = LOW;
bool aux_level2_priority_encoder_8to3_0_node_40 = LOW;
bool aux_ic_input_level2_priority_encoder_8to3_0_0 = LOW;
bool aux_ic_input_level2_priority_encoder_8to3_0_1 = LOW;
bool aux_ic_input_level2_priority_encoder_8to3_0_2 = LOW;
bool aux_ic_input_level2_priority_encoder_8to3_0_3 = LOW;
bool aux_ic_input_level2_priority_encoder_8to3_0_4 = LOW;
bool aux_ic_input_level2_priority_encoder_8to3_0_5 = LOW;
bool aux_ic_input_level2_priority_encoder_8to3_0_6 = LOW;
bool aux_ic_input_level2_priority_encoder_8to3_0_7 = LOW;
// End IC: LEVEL2_PRIORITY_ENCODER_8TO3

void setup() {
    pinMode(input_switch1, INPUT);
    pinMode(input_switch2, INPUT);
    pinMode(input_switch3, INPUT);
    pinMode(input_switch4, INPUT);
    pinMode(input_switch5, INPUT);
    pinMode(input_switch6, INPUT);
    pinMode(input_switch7, INPUT);
    pinMode(input_switch8, INPUT);
    pinMode(led1_1, OUTPUT);
    pinMode(led2_1, OUTPUT);
    pinMode(led3_1, OUTPUT);
}

void computeLogic() {
    // Assigning aux variables. //
    // IC: LEVEL2_PRIORITY_ENCODER_8TO3
    aux_ic_input_level2_priority_encoder_8to3_0_0 = input_switch1_val;
    aux_ic_input_level2_priority_encoder_8to3_0_1 = input_switch2_val;
    aux_ic_input_level2_priority_encoder_8to3_0_2 = input_switch3_val;
    aux_ic_input_level2_priority_encoder_8to3_0_3 = input_switch4_val;
    aux_ic_input_level2_priority_encoder_8to3_0_4 = input_switch5_val;
    aux_ic_input_level2_priority_encoder_8to3_0_5 = input_switch6_val;
    aux_ic_input_level2_priority_encoder_8to3_0_6 = input_switch7_val;
    aux_ic_input_level2_priority_encoder_8to3_0_7 = input_switch8_val;
    aux_level2_priority_encoder_8to3_0_node_2 = aux_ic_input_level2_priority_encoder_8to3_0_2;
    aux_level2_priority_encoder_8to3_0_node_3 = aux_ic_input_level2_priority_encoder_8to3_0_3;
    aux_level2_priority_encoder_8to3_0_node_4 = aux_ic_input_level2_priority_encoder_8to3_0_4;
    aux_level2_priority_encoder_8to3_0_node_5 = aux_ic_input_level2_priority_encoder_8to3_0_5;
    aux_level2_priority_encoder_8to3_0_node_6 = aux_ic_input_level2_priority_encoder_8to3_0_6;
    aux_level2_priority_encoder_8to3_0_node_7 = aux_ic_input_level2_priority_encoder_8to3_0_7;
    aux_level2_priority_encoder_8to3_0_or_9 = aux_level2_priority_encoder_8to3_0_node_7 || aux_level2_priority_encoder_8to3_0_node_6;
    aux_level2_priority_encoder_8to3_0_or_10 = aux_level2_priority_encoder_8to3_0_node_7 || aux_level2_priority_encoder_8to3_0_node_6 || aux_level2_priority_encoder_8to3_0_node_5;
    aux_level2_priority_encoder_8to3_0_or_11 = aux_level2_priority_encoder_8to3_0_node_7 || aux_level2_priority_encoder_8to3_0_node_6 || aux_level2_priority_encoder_8to3_0_node_5 || aux_level2_priority_encoder_8to3_0_node_4;
    aux_level2_priority_encoder_8to3_0_or_12 = aux_level2_priority_encoder_8to3_0_node_7 || aux_level2_priority_encoder_8to3_0_node_6 || aux_level2_priority_encoder_8to3_0_node_5 || aux_level2_priority_encoder_8to3_0_node_4 || aux_level2_priority_encoder_8to3_0_node_3;
    aux_level2_priority_encoder_8to3_0_or_13 = aux_level2_priority_encoder_8to3_0_node_7 || aux_level2_priority_encoder_8to3_0_node_6 || aux_level2_priority_encoder_8to3_0_node_5 || aux_level2_priority_encoder_8to3_0_node_4 || aux_level2_priority_encoder_8to3_0_node_3 || aux_level2_priority_encoder_8to3_0_node_2;
    aux_level2_priority_encoder_8to3_0_node_1 = aux_ic_input_level2_priority_encoder_8to3_0_1;
    aux_level2_priority_encoder_8to3_0_not_8 = !aux_level2_priority_encoder_8to3_0_node_7;
    aux_level2_priority_encoder_8to3_0_not_16 = !aux_level2_priority_encoder_8to3_0_or_9;
    aux_level2_priority_encoder_8to3_0_not_17 = !aux_level2_priority_encoder_8to3_0_or_10;
    aux_level2_priority_encoder_8to3_0_not_18 = !aux_level2_priority_encoder_8to3_0_or_11;
    aux_level2_priority_encoder_8to3_0_not_19 = !aux_level2_priority_encoder_8to3_0_or_12;
    aux_level2_priority_encoder_8to3_0_not_20 = !aux_level2_priority_encoder_8to3_0_or_13;
    aux_level2_priority_encoder_8to3_0_and_22 = aux_level2_priority_encoder_8to3_0_node_6 && aux_level2_priority_encoder_8to3_0_not_8;
    aux_level2_priority_encoder_8to3_0_and_23 = aux_level2_priority_encoder_8to3_0_node_5 && aux_level2_priority_encoder_8to3_0_not_16;
    aux_level2_priority_encoder_8to3_0_and_24 = aux_level2_priority_encoder_8to3_0_node_4 && aux_level2_priority_encoder_8to3_0_not_17;
    aux_level2_priority_encoder_8to3_0_and_25 = aux_level2_priority_encoder_8to3_0_node_3 && aux_level2_priority_encoder_8to3_0_not_18;
    aux_level2_priority_encoder_8to3_0_and_26 = aux_level2_priority_encoder_8to3_0_node_2 && aux_level2_priority_encoder_8to3_0_not_19;
    aux_level2_priority_encoder_8to3_0_and_27 = aux_level2_priority_encoder_8to3_0_node_1 && aux_level2_priority_encoder_8to3_0_not_20;
    aux_level2_priority_encoder_8to3_0_or_14 = aux_level2_priority_encoder_8to3_0_node_7 || aux_level2_priority_encoder_8to3_0_node_6 || aux_level2_priority_encoder_8to3_0_node_5 || aux_level2_priority_encoder_8to3_0_node_4 || aux_level2_priority_encoder_8to3_0_node_3 || aux_level2_priority_encoder_8to3_0_node_2 || aux_level2_priority_encoder_8to3_0_node_1;
    aux_level2_priority_encoder_8to3_0_or_29 = aux_level2_priority_encoder_8to3_0_and_24 || aux_level2_priority_encoder_8to3_0_and_23;
    aux_level2_priority_encoder_8to3_0_or_30 = aux_level2_priority_encoder_8to3_0_and_22 || aux_level2_priority_encoder_8to3_0_node_7;
    aux_level2_priority_encoder_8to3_0_or_32 = aux_level2_priority_encoder_8to3_0_and_26 || aux_level2_priority_encoder_8to3_0_and_25;
    aux_level2_priority_encoder_8to3_0_or_33 = aux_level2_priority_encoder_8to3_0_and_22 || aux_level2_priority_encoder_8to3_0_node_7;
    aux_level2_priority_encoder_8to3_0_or_35 = aux_level2_priority_encoder_8to3_0_and_27 || aux_level2_priority_encoder_8to3_0_and_25;
    aux_level2_priority_encoder_8to3_0_or_36 = aux_level2_priority_encoder_8to3_0_and_23 || aux_level2_priority_encoder_8to3_0_node_7;
    aux_level2_priority_encoder_8to3_0_node_0 = aux_ic_input_level2_priority_encoder_8to3_0_0;
    aux_level2_priority_encoder_8to3_0_not_21 = !aux_level2_priority_encoder_8to3_0_or_14;
    aux_level2_priority_encoder_8to3_0_or_31 = aux_level2_priority_encoder_8to3_0_or_29 || aux_level2_priority_encoder_8to3_0_or_30;
    aux_level2_priority_encoder_8to3_0_or_34 = aux_level2_priority_encoder_8to3_0_or_32 || aux_level2_priority_encoder_8to3_0_or_33;
    aux_level2_priority_encoder_8to3_0_or_37 = aux_level2_priority_encoder_8to3_0_or_35 || aux_level2_priority_encoder_8to3_0_or_36;
    aux_level2_priority_encoder_8to3_0_or_15 = aux_level2_priority_encoder_8to3_0_node_7 || aux_level2_priority_encoder_8to3_0_node_6 || aux_level2_priority_encoder_8to3_0_node_5 || aux_level2_priority_encoder_8to3_0_node_4 || aux_level2_priority_encoder_8to3_0_node_3 || aux_level2_priority_encoder_8to3_0_node_2 || aux_level2_priority_encoder_8to3_0_node_1 || aux_level2_priority_encoder_8to3_0_node_0;
    aux_level2_priority_encoder_8to3_0_and_28 = aux_level2_priority_encoder_8to3_0_node_0 && aux_level2_priority_encoder_8to3_0_not_21;
    aux_level2_priority_encoder_8to3_0_node_38 = aux_level2_priority_encoder_8to3_0_or_37;
    aux_level2_priority_encoder_8to3_0_node_39 = aux_level2_priority_encoder_8to3_0_or_34;
    aux_level2_priority_encoder_8to3_0_node_40 = aux_level2_priority_encoder_8to3_0_or_31;
    aux_level2_priority_encoder_8to3_0_addr0 = aux_level2_priority_encoder_8to3_0_node_38;
    aux_level2_priority_encoder_8to3_0_addr1_1 = aux_level2_priority_encoder_8to3_0_node_39;
    aux_level2_priority_encoder_8to3_0_addr2_2 = aux_level2_priority_encoder_8to3_0_node_40;
    // End IC: LEVEL2_PRIORITY_ENCODER_8TO3
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
    digitalWrite(led1_1, aux_level2_priority_encoder_8to3_0_addr0);
    digitalWrite(led2_1, aux_level2_priority_encoder_8to3_0_addr1_1);
    digitalWrite(led3_1, aux_level2_priority_encoder_8to3_0_addr2_2);
}
