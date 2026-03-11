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
const int led1_0 = 4;
const int led2_0 = 5;
const int led3_0 = 6;

/* ====== Aux. Variables ====== */
bool input_switch1_val = LOW;
bool input_switch2_val = LOW;
bool input_switch3_val = LOW;
bool input_switch4_val = LOW;
bool input_switch5_val = LOW;
bool input_switch6_val = LOW;
bool input_switch7_val = LOW;
bool input_switch8_val = LOW;
// IC: LEVEL3_COMPARATOR_4BIT
bool aux_level3_comparator_4bit_0_greater = LOW;
bool aux_level3_comparator_4bit_0_equal_1 = LOW;
bool aux_level3_comparator_4bit_0_less_2 = LOW;
bool aux_level3_comparator_4bit_0_node_0 = LOW;
bool aux_level3_comparator_4bit_0_node_1 = LOW;
bool aux_level3_comparator_4bit_0_node_2 = LOW;
bool aux_level3_comparator_4bit_0_node_3 = LOW;
bool aux_level3_comparator_4bit_0_node_4 = LOW;
bool aux_level3_comparator_4bit_0_node_5 = LOW;
bool aux_level3_comparator_4bit_0_node_6 = LOW;
bool aux_level3_comparator_4bit_0_node_7 = LOW;
bool aux_level3_comparator_4bit_0_not_8 = LOW;
bool aux_level3_comparator_4bit_0_not_9 = LOW;
bool aux_level3_comparator_4bit_0_not_10 = LOW;
bool aux_level3_comparator_4bit_0_not_11 = LOW;
bool aux_level3_comparator_4bit_0_not_12 = LOW;
bool aux_level3_comparator_4bit_0_not_13 = LOW;
bool aux_level3_comparator_4bit_0_not_14 = LOW;
bool aux_level3_comparator_4bit_0_not_15 = LOW;
bool aux_level3_comparator_4bit_0_xnor_16 = LOW;
bool aux_level3_comparator_4bit_0_and_17 = LOW;
bool aux_level3_comparator_4bit_0_and_18 = LOW;
bool aux_level3_comparator_4bit_0_xnor_19 = LOW;
bool aux_level3_comparator_4bit_0_and_20 = LOW;
bool aux_level3_comparator_4bit_0_and_21 = LOW;
bool aux_level3_comparator_4bit_0_xnor_22 = LOW;
bool aux_level3_comparator_4bit_0_and_23 = LOW;
bool aux_level3_comparator_4bit_0_and_24 = LOW;
bool aux_level3_comparator_4bit_0_xnor_25 = LOW;
bool aux_level3_comparator_4bit_0_and_26 = LOW;
bool aux_level3_comparator_4bit_0_and_27 = LOW;
bool aux_level3_comparator_4bit_0_and_28 = LOW;
bool aux_level3_comparator_4bit_0_and_29 = LOW;
bool aux_level3_comparator_4bit_0_or_30 = LOW;
bool aux_level3_comparator_4bit_0_and_31 = LOW;
bool aux_level3_comparator_4bit_0_or_32 = LOW;
bool aux_level3_comparator_4bit_0_and_33 = LOW;
bool aux_level3_comparator_4bit_0_or_34 = LOW;
bool aux_level3_comparator_4bit_0_and_35 = LOW;
bool aux_level3_comparator_4bit_0_or_36 = LOW;
bool aux_level3_comparator_4bit_0_and_37 = LOW;
bool aux_level3_comparator_4bit_0_or_38 = LOW;
bool aux_level3_comparator_4bit_0_and_39 = LOW;
bool aux_level3_comparator_4bit_0_or_40 = LOW;
bool aux_level3_comparator_4bit_0_node_41 = LOW;
bool aux_level3_comparator_4bit_0_node_42 = LOW;
bool aux_level3_comparator_4bit_0_node_43 = LOW;
bool aux_ic_input_level3_comparator_4bit_0 = LOW;
bool aux_ic_input_level3_comparator_4bit_1 = LOW;
bool aux_ic_input_level3_comparator_4bit_2 = LOW;
bool aux_ic_input_level3_comparator_4bit_3 = LOW;
bool aux_ic_input_level3_comparator_4bit_4 = LOW;
bool aux_ic_input_level3_comparator_4bit_5 = LOW;
bool aux_ic_input_level3_comparator_4bit_6 = LOW;
bool aux_ic_input_level3_comparator_4bit_7 = LOW;
// End IC: LEVEL3_COMPARATOR_4BIT

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
    pinMode(led2_0, OUTPUT);
    pinMode(led3_0, OUTPUT);
}

void computeLogic() {
    // Assigning aux variables. //
    // IC: LEVEL3_COMPARATOR_4BIT
    aux_ic_input_level3_comparator_4bit_0 = input_switch1_val;
    aux_ic_input_level3_comparator_4bit_1 = input_switch2_val;
    aux_ic_input_level3_comparator_4bit_2 = input_switch3_val;
    aux_ic_input_level3_comparator_4bit_3 = input_switch4_val;
    aux_ic_input_level3_comparator_4bit_4 = input_switch5_val;
    aux_ic_input_level3_comparator_4bit_5 = input_switch6_val;
    aux_ic_input_level3_comparator_4bit_6 = input_switch7_val;
    aux_ic_input_level3_comparator_4bit_7 = input_switch8_val;
    aux_level3_comparator_4bit_0_node_0 = aux_ic_input_level3_comparator_4bit_0;
    aux_level3_comparator_4bit_0_node_4 = aux_ic_input_level3_comparator_4bit_4;
    aux_level3_comparator_4bit_0_node_1 = aux_ic_input_level3_comparator_4bit_1;
    aux_level3_comparator_4bit_0_node_5 = aux_ic_input_level3_comparator_4bit_5;
    aux_level3_comparator_4bit_0_not_8 = !aux_level3_comparator_4bit_0_node_0;
    aux_level3_comparator_4bit_0_not_9 = !aux_level3_comparator_4bit_0_node_4;
    aux_level3_comparator_4bit_0_not_10 = !aux_level3_comparator_4bit_0_node_1;
    aux_level3_comparator_4bit_0_not_11 = !aux_level3_comparator_4bit_0_node_5;
    aux_level3_comparator_4bit_0_and_17 = aux_level3_comparator_4bit_0_node_0 && aux_level3_comparator_4bit_0_not_9;
    aux_level3_comparator_4bit_0_and_18 = aux_level3_comparator_4bit_0_not_8 && aux_level3_comparator_4bit_0_node_4;
    aux_level3_comparator_4bit_0_xnor_19 = !(aux_level3_comparator_4bit_0_node_1 ^ aux_level3_comparator_4bit_0_node_5);
    aux_level3_comparator_4bit_0_node_2 = aux_ic_input_level3_comparator_4bit_2;
    aux_level3_comparator_4bit_0_node_6 = aux_ic_input_level3_comparator_4bit_6;
    aux_level3_comparator_4bit_0_and_20 = aux_level3_comparator_4bit_0_node_1 && aux_level3_comparator_4bit_0_not_11;
    aux_level3_comparator_4bit_0_and_21 = aux_level3_comparator_4bit_0_not_10 && aux_level3_comparator_4bit_0_node_5;
    aux_level3_comparator_4bit_0_and_29 = aux_level3_comparator_4bit_0_xnor_19 && aux_level3_comparator_4bit_0_and_17;
    aux_level3_comparator_4bit_0_and_31 = aux_level3_comparator_4bit_0_xnor_19 && aux_level3_comparator_4bit_0_and_18;
    aux_level3_comparator_4bit_0_not_12 = !aux_level3_comparator_4bit_0_node_2;
    aux_level3_comparator_4bit_0_not_13 = !aux_level3_comparator_4bit_0_node_6;
    aux_level3_comparator_4bit_0_xnor_22 = !(aux_level3_comparator_4bit_0_node_2 ^ aux_level3_comparator_4bit_0_node_6);
    aux_level3_comparator_4bit_0_or_30 = aux_level3_comparator_4bit_0_and_20 || aux_level3_comparator_4bit_0_and_29;
    aux_level3_comparator_4bit_0_or_32 = aux_level3_comparator_4bit_0_and_21 || aux_level3_comparator_4bit_0_and_31;
    aux_level3_comparator_4bit_0_node_3 = aux_ic_input_level3_comparator_4bit_3;
    aux_level3_comparator_4bit_0_node_7 = aux_ic_input_level3_comparator_4bit_7;
    aux_level3_comparator_4bit_0_and_23 = aux_level3_comparator_4bit_0_node_2 && aux_level3_comparator_4bit_0_not_13;
    aux_level3_comparator_4bit_0_and_24 = aux_level3_comparator_4bit_0_not_12 && aux_level3_comparator_4bit_0_node_6;
    aux_level3_comparator_4bit_0_and_33 = aux_level3_comparator_4bit_0_xnor_22 && aux_level3_comparator_4bit_0_or_30;
    aux_level3_comparator_4bit_0_and_35 = aux_level3_comparator_4bit_0_xnor_22 && aux_level3_comparator_4bit_0_or_32;
    aux_level3_comparator_4bit_0_not_14 = !aux_level3_comparator_4bit_0_node_3;
    aux_level3_comparator_4bit_0_not_15 = !aux_level3_comparator_4bit_0_node_7;
    aux_level3_comparator_4bit_0_xnor_25 = !(aux_level3_comparator_4bit_0_node_3 ^ aux_level3_comparator_4bit_0_node_7);
    aux_level3_comparator_4bit_0_or_34 = aux_level3_comparator_4bit_0_and_23 || aux_level3_comparator_4bit_0_and_33;
    aux_level3_comparator_4bit_0_or_36 = aux_level3_comparator_4bit_0_and_24 || aux_level3_comparator_4bit_0_and_35;
    aux_level3_comparator_4bit_0_xnor_16 = !(aux_level3_comparator_4bit_0_node_0 ^ aux_level3_comparator_4bit_0_node_4);
    aux_level3_comparator_4bit_0_and_26 = aux_level3_comparator_4bit_0_node_3 && aux_level3_comparator_4bit_0_not_15;
    aux_level3_comparator_4bit_0_and_27 = aux_level3_comparator_4bit_0_not_14 && aux_level3_comparator_4bit_0_node_7;
    aux_level3_comparator_4bit_0_and_37 = aux_level3_comparator_4bit_0_xnor_25 && aux_level3_comparator_4bit_0_or_34;
    aux_level3_comparator_4bit_0_and_39 = aux_level3_comparator_4bit_0_xnor_25 && aux_level3_comparator_4bit_0_or_36;
    aux_level3_comparator_4bit_0_and_28 = aux_level3_comparator_4bit_0_xnor_16 && aux_level3_comparator_4bit_0_xnor_19 && aux_level3_comparator_4bit_0_xnor_22 && aux_level3_comparator_4bit_0_xnor_25;
    aux_level3_comparator_4bit_0_or_38 = aux_level3_comparator_4bit_0_and_26 || aux_level3_comparator_4bit_0_and_37;
    aux_level3_comparator_4bit_0_or_40 = aux_level3_comparator_4bit_0_and_27 || aux_level3_comparator_4bit_0_and_39;
    aux_level3_comparator_4bit_0_node_41 = aux_level3_comparator_4bit_0_or_38;
    aux_level3_comparator_4bit_0_node_42 = aux_level3_comparator_4bit_0_and_28;
    aux_level3_comparator_4bit_0_node_43 = aux_level3_comparator_4bit_0_or_40;
    aux_level3_comparator_4bit_0_greater = aux_level3_comparator_4bit_0_node_41;
    aux_level3_comparator_4bit_0_equal_1 = aux_level3_comparator_4bit_0_node_42;
    aux_level3_comparator_4bit_0_less_2 = aux_level3_comparator_4bit_0_node_43;
    // End IC: LEVEL3_COMPARATOR_4BIT
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
    digitalWrite(led1_0, aux_level3_comparator_4bit_0_greater);
    digitalWrite(led2_0, aux_level3_comparator_4bit_0_equal_1);
    digitalWrite(led3_0, aux_level3_comparator_4bit_0_less_2);
}
