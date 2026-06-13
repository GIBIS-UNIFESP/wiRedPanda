// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //
//
// Target Board: Arduino UNO R3/R4
// Pin Usage: 12/18 pins
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
const int led4_1 = 7;

/* ====== Aux. Variables ====== */
bool input_switch1_val = LOW;
bool input_switch2_val = LOW;
bool input_switch3_val = LOW;
bool input_switch4_val = LOW;
bool input_switch5_val = LOW;
bool input_switch6_val = LOW;
bool input_switch7_val = LOW;
bool input_switch8_val = LOW;
// IC: LEVEL4_BINARY_COUNTER_4BIT
bool aux_level4_binary_counter_4bit_0_q0 = LOW;
bool aux_level4_binary_counter_4bit_0_q1_1 = LOW;
bool aux_level4_binary_counter_4bit_0_q2_2 = LOW;
bool aux_level4_binary_counter_4bit_0_q3_3 = LOW;
bool aux_level4_binary_counter_4bit_0_node_0 = LOW;
bool aux_level4_binary_counter_4bit_0_node_1 = LOW;
bool aux_level4_binary_counter_4bit_0_node_2 = LOW;
bool aux_level4_binary_counter_4bit_0_node_3 = LOW;
bool aux_level4_binary_counter_4bit_0_node_4 = LOW;
bool aux_level4_binary_counter_4bit_0_node_5 = LOW;
bool aux_level4_binary_counter_4bit_0_node_6 = LOW;
bool aux_level4_binary_counter_4bit_0_node_7 = LOW;
bool aux_level4_binary_counter_4bit_0_not_9 = LOW;
bool aux_level4_binary_counter_4bit_0_not_10 = LOW;
bool aux_level4_binary_counter_4bit_0_not_11 = LOW;
bool aux_level4_binary_counter_4bit_0_not_12 = LOW;
bool aux_level4_binary_counter_4bit_0_and_13 = LOW;
bool aux_level4_binary_counter_4bit_0_and_14 = LOW;
bool aux_level4_binary_counter_4bit_0_not_15 = LOW;
bool aux_level4_binary_counter_4bit_0_not_16 = LOW;
bool aux_level4_binary_counter_4bit_0_not_17 = LOW;
bool aux_level4_binary_counter_4bit_0_and_18 = LOW;
bool aux_level4_binary_counter_4bit_0_and_19 = LOW;
bool aux_level4_binary_counter_4bit_0_and_20 = LOW;
bool aux_level4_binary_counter_4bit_0_and_21 = LOW;
bool aux_level4_binary_counter_4bit_0_and_22 = LOW;
bool aux_level4_binary_counter_4bit_0_and_23 = LOW;
bool aux_level4_binary_counter_4bit_0_or_24 = LOW;
bool aux_level4_binary_counter_4bit_0_or_25 = LOW;
bool aux_level4_binary_counter_4bit_0_or_26 = LOW;
// IC: FF0
bool aux_level4_binary_counter_4bit_0_ff0_27_q = LOW;
bool aux_level4_binary_counter_4bit_0_ff0_27_q_bar_1 = LOW;
bool aux_level4_binary_counter_4bit_0_ff0_27_node_0 = LOW;
bool aux_level4_binary_counter_4bit_0_ff0_27_node_1 = LOW;
bool aux_level4_binary_counter_4bit_0_ff0_27_node_2 = LOW;
bool aux_level4_binary_counter_4bit_0_ff0_27_node_3 = LOW;
bool aux_level4_binary_counter_4bit_0_ff0_27_not_4 = LOW;
bool aux_level4_binary_counter_4bit_0_ff0_27_not_5 = LOW;
bool aux_level4_binary_counter_4bit_0_ff0_27_not_6 = LOW;
bool aux_level4_binary_counter_4bit_0_ff0_27_or_7 = LOW;
bool aux_level4_binary_counter_4bit_0_ff0_27_or_8 = LOW;
bool aux_level4_binary_counter_4bit_0_ff0_27_or_9 = LOW;
bool aux_level4_binary_counter_4bit_0_ff0_27_or_10 = LOW;
bool aux_level4_binary_counter_4bit_0_ff0_27_not_11 = LOW;
bool aux_level4_binary_counter_4bit_0_ff0_27_and_12 = LOW;
bool aux_level4_binary_counter_4bit_0_ff0_27_and_13 = LOW;
bool aux_level4_binary_counter_4bit_0_ff0_27_nor_14 = LOW;
bool aux_level4_binary_counter_4bit_0_ff0_27_nor_15 = LOW;
bool aux_level4_binary_counter_4bit_0_ff0_27_and_16 = LOW;
bool aux_level4_binary_counter_4bit_0_ff0_27_and_17 = LOW;
bool aux_level4_binary_counter_4bit_0_ff0_27_nor_18 = LOW;
bool aux_level4_binary_counter_4bit_0_ff0_27_nor_19 = LOW;
bool aux_level4_binary_counter_4bit_0_ff0_27_node_20 = LOW;
bool aux_level4_binary_counter_4bit_0_ff0_27_node_21 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_ff0_27_0 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_ff0_27_1 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_ff0_27_2 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_ff0_27_3 = LOW;
// End IC: FF0
// IC: FF1
bool aux_level4_binary_counter_4bit_0_ff1_28_q = LOW;
bool aux_level4_binary_counter_4bit_0_ff1_28_q_bar_1 = LOW;
bool aux_level4_binary_counter_4bit_0_ff1_28_node_0 = LOW;
bool aux_level4_binary_counter_4bit_0_ff1_28_node_1 = LOW;
bool aux_level4_binary_counter_4bit_0_ff1_28_node_2 = LOW;
bool aux_level4_binary_counter_4bit_0_ff1_28_node_3 = LOW;
bool aux_level4_binary_counter_4bit_0_ff1_28_not_4 = LOW;
bool aux_level4_binary_counter_4bit_0_ff1_28_not_5 = LOW;
bool aux_level4_binary_counter_4bit_0_ff1_28_not_6 = LOW;
bool aux_level4_binary_counter_4bit_0_ff1_28_or_7 = LOW;
bool aux_level4_binary_counter_4bit_0_ff1_28_or_8 = LOW;
bool aux_level4_binary_counter_4bit_0_ff1_28_or_9 = LOW;
bool aux_level4_binary_counter_4bit_0_ff1_28_or_10 = LOW;
bool aux_level4_binary_counter_4bit_0_ff1_28_not_11 = LOW;
bool aux_level4_binary_counter_4bit_0_ff1_28_and_12 = LOW;
bool aux_level4_binary_counter_4bit_0_ff1_28_and_13 = LOW;
bool aux_level4_binary_counter_4bit_0_ff1_28_nor_14 = LOW;
bool aux_level4_binary_counter_4bit_0_ff1_28_nor_15 = LOW;
bool aux_level4_binary_counter_4bit_0_ff1_28_and_16 = LOW;
bool aux_level4_binary_counter_4bit_0_ff1_28_and_17 = LOW;
bool aux_level4_binary_counter_4bit_0_ff1_28_nor_18 = LOW;
bool aux_level4_binary_counter_4bit_0_ff1_28_nor_19 = LOW;
bool aux_level4_binary_counter_4bit_0_ff1_28_node_20 = LOW;
bool aux_level4_binary_counter_4bit_0_ff1_28_node_21 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_ff1_28_0 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_ff1_28_1 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_ff1_28_2 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_ff1_28_3 = LOW;
// End IC: FF1
// IC: FF2
bool aux_level4_binary_counter_4bit_0_ff2_29_q = LOW;
bool aux_level4_binary_counter_4bit_0_ff2_29_q_bar_1 = LOW;
bool aux_level4_binary_counter_4bit_0_ff2_29_node_0 = LOW;
bool aux_level4_binary_counter_4bit_0_ff2_29_node_1 = LOW;
bool aux_level4_binary_counter_4bit_0_ff2_29_node_2 = LOW;
bool aux_level4_binary_counter_4bit_0_ff2_29_node_3 = LOW;
bool aux_level4_binary_counter_4bit_0_ff2_29_not_4 = LOW;
bool aux_level4_binary_counter_4bit_0_ff2_29_not_5 = LOW;
bool aux_level4_binary_counter_4bit_0_ff2_29_not_6 = LOW;
bool aux_level4_binary_counter_4bit_0_ff2_29_or_7 = LOW;
bool aux_level4_binary_counter_4bit_0_ff2_29_or_8 = LOW;
bool aux_level4_binary_counter_4bit_0_ff2_29_or_9 = LOW;
bool aux_level4_binary_counter_4bit_0_ff2_29_or_10 = LOW;
bool aux_level4_binary_counter_4bit_0_ff2_29_not_11 = LOW;
bool aux_level4_binary_counter_4bit_0_ff2_29_and_12 = LOW;
bool aux_level4_binary_counter_4bit_0_ff2_29_and_13 = LOW;
bool aux_level4_binary_counter_4bit_0_ff2_29_nor_14 = LOW;
bool aux_level4_binary_counter_4bit_0_ff2_29_nor_15 = LOW;
bool aux_level4_binary_counter_4bit_0_ff2_29_and_16 = LOW;
bool aux_level4_binary_counter_4bit_0_ff2_29_and_17 = LOW;
bool aux_level4_binary_counter_4bit_0_ff2_29_nor_18 = LOW;
bool aux_level4_binary_counter_4bit_0_ff2_29_nor_19 = LOW;
bool aux_level4_binary_counter_4bit_0_ff2_29_node_20 = LOW;
bool aux_level4_binary_counter_4bit_0_ff2_29_node_21 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_ff2_29_0 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_ff2_29_1 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_ff2_29_2 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_ff2_29_3 = LOW;
// End IC: FF2
// IC: FF3
bool aux_level4_binary_counter_4bit_0_ff3_30_q = LOW;
bool aux_level4_binary_counter_4bit_0_ff3_30_q_bar_1 = LOW;
bool aux_level4_binary_counter_4bit_0_ff3_30_node_0 = LOW;
bool aux_level4_binary_counter_4bit_0_ff3_30_node_1 = LOW;
bool aux_level4_binary_counter_4bit_0_ff3_30_node_2 = LOW;
bool aux_level4_binary_counter_4bit_0_ff3_30_node_3 = LOW;
bool aux_level4_binary_counter_4bit_0_ff3_30_not_4 = LOW;
bool aux_level4_binary_counter_4bit_0_ff3_30_not_5 = LOW;
bool aux_level4_binary_counter_4bit_0_ff3_30_not_6 = LOW;
bool aux_level4_binary_counter_4bit_0_ff3_30_or_7 = LOW;
bool aux_level4_binary_counter_4bit_0_ff3_30_or_8 = LOW;
bool aux_level4_binary_counter_4bit_0_ff3_30_or_9 = LOW;
bool aux_level4_binary_counter_4bit_0_ff3_30_or_10 = LOW;
bool aux_level4_binary_counter_4bit_0_ff3_30_not_11 = LOW;
bool aux_level4_binary_counter_4bit_0_ff3_30_and_12 = LOW;
bool aux_level4_binary_counter_4bit_0_ff3_30_and_13 = LOW;
bool aux_level4_binary_counter_4bit_0_ff3_30_nor_14 = LOW;
bool aux_level4_binary_counter_4bit_0_ff3_30_nor_15 = LOW;
bool aux_level4_binary_counter_4bit_0_ff3_30_and_16 = LOW;
bool aux_level4_binary_counter_4bit_0_ff3_30_and_17 = LOW;
bool aux_level4_binary_counter_4bit_0_ff3_30_nor_18 = LOW;
bool aux_level4_binary_counter_4bit_0_ff3_30_nor_19 = LOW;
bool aux_level4_binary_counter_4bit_0_ff3_30_node_20 = LOW;
bool aux_level4_binary_counter_4bit_0_ff3_30_node_21 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_ff3_30_0 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_ff3_30_1 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_ff3_30_2 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_ff3_30_3 = LOW;
// End IC: FF3
// IC: hold_mux0
bool aux_level4_binary_counter_4bit_0_hold_mux0_31_output = LOW;
bool aux_level4_binary_counter_4bit_0_hold_mux0_31_node_0 = LOW;
bool aux_level4_binary_counter_4bit_0_hold_mux0_31_node_1 = LOW;
bool aux_level4_binary_counter_4bit_0_hold_mux0_31_node_2 = LOW;
bool aux_level4_binary_counter_4bit_0_hold_mux0_31_mux_3 = LOW;
bool aux_level4_binary_counter_4bit_0_hold_mux0_31_node_4 = LOW;
bool aux_level4_binary_counter_4bit_0_hold_mux0_31_and_5 = LOW;
bool aux_level4_binary_counter_4bit_0_hold_mux0_31_node_6 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_hold_mux0_31_0 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_hold_mux0_31_1 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_hold_mux0_31_2 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_hold_mux0_31_3 = LOW;
// End IC: hold_mux0
// IC: load_mux0
bool aux_level4_binary_counter_4bit_0_load_mux0_32_output = LOW;
bool aux_level4_binary_counter_4bit_0_load_mux0_32_node_0 = LOW;
bool aux_level4_binary_counter_4bit_0_load_mux0_32_node_1 = LOW;
bool aux_level4_binary_counter_4bit_0_load_mux0_32_node_2 = LOW;
bool aux_level4_binary_counter_4bit_0_load_mux0_32_mux_3 = LOW;
bool aux_level4_binary_counter_4bit_0_load_mux0_32_node_4 = LOW;
bool aux_level4_binary_counter_4bit_0_load_mux0_32_and_5 = LOW;
bool aux_level4_binary_counter_4bit_0_load_mux0_32_node_6 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_load_mux0_32_0 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_load_mux0_32_1 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_load_mux0_32_2 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_load_mux0_32_3 = LOW;
// End IC: load_mux0
// IC: hold_mux1
bool aux_level4_binary_counter_4bit_0_hold_mux1_33_output = LOW;
bool aux_level4_binary_counter_4bit_0_hold_mux1_33_node_0 = LOW;
bool aux_level4_binary_counter_4bit_0_hold_mux1_33_node_1 = LOW;
bool aux_level4_binary_counter_4bit_0_hold_mux1_33_node_2 = LOW;
bool aux_level4_binary_counter_4bit_0_hold_mux1_33_mux_3 = LOW;
bool aux_level4_binary_counter_4bit_0_hold_mux1_33_node_4 = LOW;
bool aux_level4_binary_counter_4bit_0_hold_mux1_33_and_5 = LOW;
bool aux_level4_binary_counter_4bit_0_hold_mux1_33_node_6 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_hold_mux1_33_0 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_hold_mux1_33_1 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_hold_mux1_33_2 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_hold_mux1_33_3 = LOW;
// End IC: hold_mux1
// IC: load_mux1
bool aux_level4_binary_counter_4bit_0_load_mux1_34_output = LOW;
bool aux_level4_binary_counter_4bit_0_load_mux1_34_node_0 = LOW;
bool aux_level4_binary_counter_4bit_0_load_mux1_34_node_1 = LOW;
bool aux_level4_binary_counter_4bit_0_load_mux1_34_node_2 = LOW;
bool aux_level4_binary_counter_4bit_0_load_mux1_34_mux_3 = LOW;
bool aux_level4_binary_counter_4bit_0_load_mux1_34_node_4 = LOW;
bool aux_level4_binary_counter_4bit_0_load_mux1_34_and_5 = LOW;
bool aux_level4_binary_counter_4bit_0_load_mux1_34_node_6 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_load_mux1_34_0 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_load_mux1_34_1 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_load_mux1_34_2 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_load_mux1_34_3 = LOW;
// End IC: load_mux1
// IC: hold_mux2
bool aux_level4_binary_counter_4bit_0_hold_mux2_35_output = LOW;
bool aux_level4_binary_counter_4bit_0_hold_mux2_35_node_0 = LOW;
bool aux_level4_binary_counter_4bit_0_hold_mux2_35_node_1 = LOW;
bool aux_level4_binary_counter_4bit_0_hold_mux2_35_node_2 = LOW;
bool aux_level4_binary_counter_4bit_0_hold_mux2_35_mux_3 = LOW;
bool aux_level4_binary_counter_4bit_0_hold_mux2_35_node_4 = LOW;
bool aux_level4_binary_counter_4bit_0_hold_mux2_35_and_5 = LOW;
bool aux_level4_binary_counter_4bit_0_hold_mux2_35_node_6 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_hold_mux2_35_0 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_hold_mux2_35_1 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_hold_mux2_35_2 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_hold_mux2_35_3 = LOW;
// End IC: hold_mux2
// IC: load_mux2
bool aux_level4_binary_counter_4bit_0_load_mux2_36_output = LOW;
bool aux_level4_binary_counter_4bit_0_load_mux2_36_node_0 = LOW;
bool aux_level4_binary_counter_4bit_0_load_mux2_36_node_1 = LOW;
bool aux_level4_binary_counter_4bit_0_load_mux2_36_node_2 = LOW;
bool aux_level4_binary_counter_4bit_0_load_mux2_36_mux_3 = LOW;
bool aux_level4_binary_counter_4bit_0_load_mux2_36_node_4 = LOW;
bool aux_level4_binary_counter_4bit_0_load_mux2_36_and_5 = LOW;
bool aux_level4_binary_counter_4bit_0_load_mux2_36_node_6 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_load_mux2_36_0 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_load_mux2_36_1 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_load_mux2_36_2 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_load_mux2_36_3 = LOW;
// End IC: load_mux2
// IC: hold_mux3
bool aux_level4_binary_counter_4bit_0_hold_mux3_37_output = LOW;
bool aux_level4_binary_counter_4bit_0_hold_mux3_37_node_0 = LOW;
bool aux_level4_binary_counter_4bit_0_hold_mux3_37_node_1 = LOW;
bool aux_level4_binary_counter_4bit_0_hold_mux3_37_node_2 = LOW;
bool aux_level4_binary_counter_4bit_0_hold_mux3_37_mux_3 = LOW;
bool aux_level4_binary_counter_4bit_0_hold_mux3_37_node_4 = LOW;
bool aux_level4_binary_counter_4bit_0_hold_mux3_37_and_5 = LOW;
bool aux_level4_binary_counter_4bit_0_hold_mux3_37_node_6 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_hold_mux3_37_0 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_hold_mux3_37_1 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_hold_mux3_37_2 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_hold_mux3_37_3 = LOW;
// End IC: hold_mux3
// IC: load_mux3
bool aux_level4_binary_counter_4bit_0_load_mux3_38_output = LOW;
bool aux_level4_binary_counter_4bit_0_load_mux3_38_node_0 = LOW;
bool aux_level4_binary_counter_4bit_0_load_mux3_38_node_1 = LOW;
bool aux_level4_binary_counter_4bit_0_load_mux3_38_node_2 = LOW;
bool aux_level4_binary_counter_4bit_0_load_mux3_38_mux_3 = LOW;
bool aux_level4_binary_counter_4bit_0_load_mux3_38_node_4 = LOW;
bool aux_level4_binary_counter_4bit_0_load_mux3_38_and_5 = LOW;
bool aux_level4_binary_counter_4bit_0_load_mux3_38_node_6 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_load_mux3_38_0 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_load_mux3_38_1 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_load_mux3_38_2 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_load_mux3_38_3 = LOW;
// End IC: load_mux3
bool aux_level4_binary_counter_4bit_0_node_39 = LOW;
bool aux_level4_binary_counter_4bit_0_node_40 = LOW;
bool aux_level4_binary_counter_4bit_0_node_41 = LOW;
bool aux_level4_binary_counter_4bit_0_node_42 = LOW;
bool aux_level4_binary_counter_4bit_0_not_43 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_0 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_1 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_2 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_3 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_4 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_5 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_6 = LOW;
bool aux_ic_input_level4_binary_counter_4bit_0_7 = LOW;
// End IC: LEVEL4_BINARY_COUNTER_4BIT

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
    pinMode(led4_1, OUTPUT);
}

void computeLogic() {
    // Assigning aux variables. //
    // IC: LEVEL4_BINARY_COUNTER_4BIT
    aux_ic_input_level4_binary_counter_4bit_0_0 = input_switch1_val;
    aux_ic_input_level4_binary_counter_4bit_0_1 = input_switch2_val;
    aux_ic_input_level4_binary_counter_4bit_0_2 = input_switch3_val;
    aux_ic_input_level4_binary_counter_4bit_0_3 = input_switch4_val;
    aux_ic_input_level4_binary_counter_4bit_0_4 = input_switch5_val;
    aux_ic_input_level4_binary_counter_4bit_0_5 = input_switch6_val;
    aux_ic_input_level4_binary_counter_4bit_0_6 = input_switch7_val;
    aux_ic_input_level4_binary_counter_4bit_0_7 = input_switch8_val;
    aux_level4_binary_counter_4bit_0_node_3 = aux_ic_input_level4_binary_counter_4bit_0_0;
    aux_level4_binary_counter_4bit_0_node_0 = aux_ic_input_level4_binary_counter_4bit_0_4;
    aux_level4_binary_counter_4bit_0_node_1 = aux_ic_input_level4_binary_counter_4bit_0_2;
    aux_level4_binary_counter_4bit_0_node_2 = aux_ic_input_level4_binary_counter_4bit_0_1;
    aux_level4_binary_counter_4bit_0_node_4 = aux_ic_input_level4_binary_counter_4bit_0_3;
    aux_level4_binary_counter_4bit_0_node_5 = aux_ic_input_level4_binary_counter_4bit_0_5;
    aux_level4_binary_counter_4bit_0_node_6 = aux_ic_input_level4_binary_counter_4bit_0_6;
    aux_level4_binary_counter_4bit_0_node_7 = aux_ic_input_level4_binary_counter_4bit_0_7;
    aux_level4_binary_counter_4bit_0_not_43 = !aux_level4_binary_counter_4bit_0_node_3;
    // IC: FF0
    aux_ic_input_level4_binary_counter_4bit_0_ff0_27_0 = aux_level4_binary_counter_4bit_0_load_mux0_32_output;
    aux_ic_input_level4_binary_counter_4bit_0_ff0_27_1 = aux_level4_binary_counter_4bit_0_node_0;
    aux_ic_input_level4_binary_counter_4bit_0_ff0_27_2 = HIGH;
    aux_ic_input_level4_binary_counter_4bit_0_ff0_27_3 = aux_level4_binary_counter_4bit_0_not_43;
    aux_level4_binary_counter_4bit_0_ff0_27_node_0 = aux_ic_input_level4_binary_counter_4bit_0_ff0_27_0;
    aux_level4_binary_counter_4bit_0_ff0_27_node_2 = aux_ic_input_level4_binary_counter_4bit_0_ff0_27_2;
    aux_level4_binary_counter_4bit_0_ff0_27_node_1 = aux_ic_input_level4_binary_counter_4bit_0_ff0_27_1;
    aux_level4_binary_counter_4bit_0_ff0_27_node_3 = aux_ic_input_level4_binary_counter_4bit_0_ff0_27_3;
    aux_level4_binary_counter_4bit_0_ff0_27_not_4 = !aux_level4_binary_counter_4bit_0_ff0_27_node_1;
    aux_level4_binary_counter_4bit_0_ff0_27_not_5 = !aux_level4_binary_counter_4bit_0_ff0_27_node_2;
    aux_level4_binary_counter_4bit_0_ff0_27_and_12 = aux_level4_binary_counter_4bit_0_ff0_27_node_0 && aux_level4_binary_counter_4bit_0_ff0_27_not_4;
    aux_level4_binary_counter_4bit_0_ff0_27_or_10 = aux_level4_binary_counter_4bit_0_ff0_27_and_12 || aux_level4_binary_counter_4bit_0_ff0_27_not_5;
    aux_level4_binary_counter_4bit_0_ff0_27_nor_15 = !(aux_level4_binary_counter_4bit_0_ff0_27_or_10 || aux_level4_binary_counter_4bit_0_ff0_27_nor_14);
    aux_level4_binary_counter_4bit_0_ff0_27_not_6 = !aux_level4_binary_counter_4bit_0_ff0_27_node_3;
    aux_level4_binary_counter_4bit_0_ff0_27_not_11 = !aux_level4_binary_counter_4bit_0_ff0_27_node_0;
    aux_level4_binary_counter_4bit_0_ff0_27_and_17 = aux_level4_binary_counter_4bit_0_ff0_27_nor_15 && aux_level4_binary_counter_4bit_0_ff0_27_node_1;
    aux_level4_binary_counter_4bit_0_ff0_27_or_7 = aux_level4_binary_counter_4bit_0_ff0_27_and_17 || aux_level4_binary_counter_4bit_0_ff0_27_not_6;
    aux_level4_binary_counter_4bit_0_ff0_27_and_13 = aux_level4_binary_counter_4bit_0_ff0_27_not_11 && aux_level4_binary_counter_4bit_0_ff0_27_not_4;
    aux_level4_binary_counter_4bit_0_ff0_27_and_16 = aux_level4_binary_counter_4bit_0_ff0_27_nor_14 && aux_level4_binary_counter_4bit_0_ff0_27_node_1;
    aux_level4_binary_counter_4bit_0_ff0_27_or_8 = aux_level4_binary_counter_4bit_0_ff0_27_and_16 || aux_level4_binary_counter_4bit_0_ff0_27_not_5;
    aux_level4_binary_counter_4bit_0_ff0_27_or_9 = aux_level4_binary_counter_4bit_0_ff0_27_and_13 || aux_level4_binary_counter_4bit_0_ff0_27_not_6;
    aux_level4_binary_counter_4bit_0_ff0_27_nor_18 = !(aux_level4_binary_counter_4bit_0_ff0_27_or_7 || aux_level4_binary_counter_4bit_0_ff0_27_nor_19);
    aux_level4_binary_counter_4bit_0_ff0_27_nor_14 = !(aux_level4_binary_counter_4bit_0_ff0_27_or_9 || aux_level4_binary_counter_4bit_0_ff0_27_nor_15);
    aux_level4_binary_counter_4bit_0_ff0_27_nor_19 = !(aux_level4_binary_counter_4bit_0_ff0_27_or_8 || aux_level4_binary_counter_4bit_0_ff0_27_nor_18);
    aux_level4_binary_counter_4bit_0_ff0_27_node_20 = aux_level4_binary_counter_4bit_0_ff0_27_nor_18;
    aux_level4_binary_counter_4bit_0_ff0_27_node_21 = aux_level4_binary_counter_4bit_0_ff0_27_nor_19;
    aux_level4_binary_counter_4bit_0_ff0_27_q = aux_level4_binary_counter_4bit_0_ff0_27_node_20;
    aux_level4_binary_counter_4bit_0_ff0_27_q_bar_1 = aux_level4_binary_counter_4bit_0_ff0_27_node_21;
    // End IC: FF0
    // IC: FF1
    aux_ic_input_level4_binary_counter_4bit_0_ff1_28_0 = aux_level4_binary_counter_4bit_0_load_mux1_34_output;
    aux_ic_input_level4_binary_counter_4bit_0_ff1_28_1 = aux_level4_binary_counter_4bit_0_node_0;
    aux_ic_input_level4_binary_counter_4bit_0_ff1_28_2 = HIGH;
    aux_ic_input_level4_binary_counter_4bit_0_ff1_28_3 = aux_level4_binary_counter_4bit_0_not_43;
    aux_level4_binary_counter_4bit_0_ff1_28_node_0 = aux_ic_input_level4_binary_counter_4bit_0_ff1_28_0;
    aux_level4_binary_counter_4bit_0_ff1_28_node_2 = aux_ic_input_level4_binary_counter_4bit_0_ff1_28_2;
    aux_level4_binary_counter_4bit_0_ff1_28_node_1 = aux_ic_input_level4_binary_counter_4bit_0_ff1_28_1;
    aux_level4_binary_counter_4bit_0_ff1_28_node_3 = aux_ic_input_level4_binary_counter_4bit_0_ff1_28_3;
    aux_level4_binary_counter_4bit_0_ff1_28_not_4 = !aux_level4_binary_counter_4bit_0_ff1_28_node_1;
    aux_level4_binary_counter_4bit_0_ff1_28_not_5 = !aux_level4_binary_counter_4bit_0_ff1_28_node_2;
    aux_level4_binary_counter_4bit_0_ff1_28_and_12 = aux_level4_binary_counter_4bit_0_ff1_28_node_0 && aux_level4_binary_counter_4bit_0_ff1_28_not_4;
    aux_level4_binary_counter_4bit_0_ff1_28_or_10 = aux_level4_binary_counter_4bit_0_ff1_28_and_12 || aux_level4_binary_counter_4bit_0_ff1_28_not_5;
    aux_level4_binary_counter_4bit_0_ff1_28_nor_15 = !(aux_level4_binary_counter_4bit_0_ff1_28_or_10 || aux_level4_binary_counter_4bit_0_ff1_28_nor_14);
    aux_level4_binary_counter_4bit_0_ff1_28_not_6 = !aux_level4_binary_counter_4bit_0_ff1_28_node_3;
    aux_level4_binary_counter_4bit_0_ff1_28_not_11 = !aux_level4_binary_counter_4bit_0_ff1_28_node_0;
    aux_level4_binary_counter_4bit_0_ff1_28_and_17 = aux_level4_binary_counter_4bit_0_ff1_28_nor_15 && aux_level4_binary_counter_4bit_0_ff1_28_node_1;
    aux_level4_binary_counter_4bit_0_ff1_28_or_7 = aux_level4_binary_counter_4bit_0_ff1_28_and_17 || aux_level4_binary_counter_4bit_0_ff1_28_not_6;
    aux_level4_binary_counter_4bit_0_ff1_28_and_13 = aux_level4_binary_counter_4bit_0_ff1_28_not_11 && aux_level4_binary_counter_4bit_0_ff1_28_not_4;
    aux_level4_binary_counter_4bit_0_ff1_28_and_16 = aux_level4_binary_counter_4bit_0_ff1_28_nor_14 && aux_level4_binary_counter_4bit_0_ff1_28_node_1;
    aux_level4_binary_counter_4bit_0_ff1_28_or_8 = aux_level4_binary_counter_4bit_0_ff1_28_and_16 || aux_level4_binary_counter_4bit_0_ff1_28_not_5;
    aux_level4_binary_counter_4bit_0_ff1_28_or_9 = aux_level4_binary_counter_4bit_0_ff1_28_and_13 || aux_level4_binary_counter_4bit_0_ff1_28_not_6;
    aux_level4_binary_counter_4bit_0_ff1_28_nor_18 = !(aux_level4_binary_counter_4bit_0_ff1_28_or_7 || aux_level4_binary_counter_4bit_0_ff1_28_nor_19);
    aux_level4_binary_counter_4bit_0_ff1_28_nor_14 = !(aux_level4_binary_counter_4bit_0_ff1_28_or_9 || aux_level4_binary_counter_4bit_0_ff1_28_nor_15);
    aux_level4_binary_counter_4bit_0_ff1_28_nor_19 = !(aux_level4_binary_counter_4bit_0_ff1_28_or_8 || aux_level4_binary_counter_4bit_0_ff1_28_nor_18);
    aux_level4_binary_counter_4bit_0_ff1_28_node_20 = aux_level4_binary_counter_4bit_0_ff1_28_nor_18;
    aux_level4_binary_counter_4bit_0_ff1_28_node_21 = aux_level4_binary_counter_4bit_0_ff1_28_nor_19;
    aux_level4_binary_counter_4bit_0_ff1_28_q = aux_level4_binary_counter_4bit_0_ff1_28_node_20;
    aux_level4_binary_counter_4bit_0_ff1_28_q_bar_1 = aux_level4_binary_counter_4bit_0_ff1_28_node_21;
    // End IC: FF1
    aux_level4_binary_counter_4bit_0_and_13 = aux_level4_binary_counter_4bit_0_ff0_27_q && aux_level4_binary_counter_4bit_0_ff1_28_q;
    // IC: FF2
    aux_ic_input_level4_binary_counter_4bit_0_ff2_29_0 = aux_level4_binary_counter_4bit_0_load_mux2_36_output;
    aux_ic_input_level4_binary_counter_4bit_0_ff2_29_1 = aux_level4_binary_counter_4bit_0_node_0;
    aux_ic_input_level4_binary_counter_4bit_0_ff2_29_2 = HIGH;
    aux_ic_input_level4_binary_counter_4bit_0_ff2_29_3 = aux_level4_binary_counter_4bit_0_not_43;
    aux_level4_binary_counter_4bit_0_ff2_29_node_0 = aux_ic_input_level4_binary_counter_4bit_0_ff2_29_0;
    aux_level4_binary_counter_4bit_0_ff2_29_node_2 = aux_ic_input_level4_binary_counter_4bit_0_ff2_29_2;
    aux_level4_binary_counter_4bit_0_ff2_29_node_1 = aux_ic_input_level4_binary_counter_4bit_0_ff2_29_1;
    aux_level4_binary_counter_4bit_0_ff2_29_node_3 = aux_ic_input_level4_binary_counter_4bit_0_ff2_29_3;
    aux_level4_binary_counter_4bit_0_ff2_29_not_4 = !aux_level4_binary_counter_4bit_0_ff2_29_node_1;
    aux_level4_binary_counter_4bit_0_ff2_29_not_5 = !aux_level4_binary_counter_4bit_0_ff2_29_node_2;
    aux_level4_binary_counter_4bit_0_ff2_29_and_12 = aux_level4_binary_counter_4bit_0_ff2_29_node_0 && aux_level4_binary_counter_4bit_0_ff2_29_not_4;
    aux_level4_binary_counter_4bit_0_ff2_29_or_10 = aux_level4_binary_counter_4bit_0_ff2_29_and_12 || aux_level4_binary_counter_4bit_0_ff2_29_not_5;
    aux_level4_binary_counter_4bit_0_ff2_29_nor_15 = !(aux_level4_binary_counter_4bit_0_ff2_29_or_10 || aux_level4_binary_counter_4bit_0_ff2_29_nor_14);
    aux_level4_binary_counter_4bit_0_ff2_29_not_6 = !aux_level4_binary_counter_4bit_0_ff2_29_node_3;
    aux_level4_binary_counter_4bit_0_ff2_29_not_11 = !aux_level4_binary_counter_4bit_0_ff2_29_node_0;
    aux_level4_binary_counter_4bit_0_ff2_29_and_17 = aux_level4_binary_counter_4bit_0_ff2_29_nor_15 && aux_level4_binary_counter_4bit_0_ff2_29_node_1;
    aux_level4_binary_counter_4bit_0_ff2_29_or_7 = aux_level4_binary_counter_4bit_0_ff2_29_and_17 || aux_level4_binary_counter_4bit_0_ff2_29_not_6;
    aux_level4_binary_counter_4bit_0_ff2_29_and_13 = aux_level4_binary_counter_4bit_0_ff2_29_not_11 && aux_level4_binary_counter_4bit_0_ff2_29_not_4;
    aux_level4_binary_counter_4bit_0_ff2_29_and_16 = aux_level4_binary_counter_4bit_0_ff2_29_nor_14 && aux_level4_binary_counter_4bit_0_ff2_29_node_1;
    aux_level4_binary_counter_4bit_0_ff2_29_or_8 = aux_level4_binary_counter_4bit_0_ff2_29_and_16 || aux_level4_binary_counter_4bit_0_ff2_29_not_5;
    aux_level4_binary_counter_4bit_0_ff2_29_or_9 = aux_level4_binary_counter_4bit_0_ff2_29_and_13 || aux_level4_binary_counter_4bit_0_ff2_29_not_6;
    aux_level4_binary_counter_4bit_0_ff2_29_nor_18 = !(aux_level4_binary_counter_4bit_0_ff2_29_or_7 || aux_level4_binary_counter_4bit_0_ff2_29_nor_19);
    aux_level4_binary_counter_4bit_0_ff2_29_nor_14 = !(aux_level4_binary_counter_4bit_0_ff2_29_or_9 || aux_level4_binary_counter_4bit_0_ff2_29_nor_15);
    aux_level4_binary_counter_4bit_0_ff2_29_nor_19 = !(aux_level4_binary_counter_4bit_0_ff2_29_or_8 || aux_level4_binary_counter_4bit_0_ff2_29_nor_18);
    aux_level4_binary_counter_4bit_0_ff2_29_node_20 = aux_level4_binary_counter_4bit_0_ff2_29_nor_18;
    aux_level4_binary_counter_4bit_0_ff2_29_node_21 = aux_level4_binary_counter_4bit_0_ff2_29_nor_19;
    aux_level4_binary_counter_4bit_0_ff2_29_q = aux_level4_binary_counter_4bit_0_ff2_29_node_20;
    aux_level4_binary_counter_4bit_0_ff2_29_q_bar_1 = aux_level4_binary_counter_4bit_0_ff2_29_node_21;
    // End IC: FF2
    aux_level4_binary_counter_4bit_0_and_14 = aux_level4_binary_counter_4bit_0_and_13 && aux_level4_binary_counter_4bit_0_ff2_29_q;
    // IC: FF3
    aux_ic_input_level4_binary_counter_4bit_0_ff3_30_0 = aux_level4_binary_counter_4bit_0_load_mux3_38_output;
    aux_ic_input_level4_binary_counter_4bit_0_ff3_30_1 = aux_level4_binary_counter_4bit_0_node_0;
    aux_ic_input_level4_binary_counter_4bit_0_ff3_30_2 = HIGH;
    aux_ic_input_level4_binary_counter_4bit_0_ff3_30_3 = aux_level4_binary_counter_4bit_0_not_43;
    aux_level4_binary_counter_4bit_0_ff3_30_node_0 = aux_ic_input_level4_binary_counter_4bit_0_ff3_30_0;
    aux_level4_binary_counter_4bit_0_ff3_30_node_2 = aux_ic_input_level4_binary_counter_4bit_0_ff3_30_2;
    aux_level4_binary_counter_4bit_0_ff3_30_node_1 = aux_ic_input_level4_binary_counter_4bit_0_ff3_30_1;
    aux_level4_binary_counter_4bit_0_ff3_30_node_3 = aux_ic_input_level4_binary_counter_4bit_0_ff3_30_3;
    aux_level4_binary_counter_4bit_0_ff3_30_not_4 = !aux_level4_binary_counter_4bit_0_ff3_30_node_1;
    aux_level4_binary_counter_4bit_0_ff3_30_not_5 = !aux_level4_binary_counter_4bit_0_ff3_30_node_2;
    aux_level4_binary_counter_4bit_0_ff3_30_and_12 = aux_level4_binary_counter_4bit_0_ff3_30_node_0 && aux_level4_binary_counter_4bit_0_ff3_30_not_4;
    aux_level4_binary_counter_4bit_0_ff3_30_or_10 = aux_level4_binary_counter_4bit_0_ff3_30_and_12 || aux_level4_binary_counter_4bit_0_ff3_30_not_5;
    aux_level4_binary_counter_4bit_0_ff3_30_nor_15 = !(aux_level4_binary_counter_4bit_0_ff3_30_or_10 || aux_level4_binary_counter_4bit_0_ff3_30_nor_14);
    aux_level4_binary_counter_4bit_0_ff3_30_not_6 = !aux_level4_binary_counter_4bit_0_ff3_30_node_3;
    aux_level4_binary_counter_4bit_0_ff3_30_not_11 = !aux_level4_binary_counter_4bit_0_ff3_30_node_0;
    aux_level4_binary_counter_4bit_0_ff3_30_and_17 = aux_level4_binary_counter_4bit_0_ff3_30_nor_15 && aux_level4_binary_counter_4bit_0_ff3_30_node_1;
    aux_level4_binary_counter_4bit_0_ff3_30_or_7 = aux_level4_binary_counter_4bit_0_ff3_30_and_17 || aux_level4_binary_counter_4bit_0_ff3_30_not_6;
    aux_level4_binary_counter_4bit_0_ff3_30_and_13 = aux_level4_binary_counter_4bit_0_ff3_30_not_11 && aux_level4_binary_counter_4bit_0_ff3_30_not_4;
    aux_level4_binary_counter_4bit_0_ff3_30_and_16 = aux_level4_binary_counter_4bit_0_ff3_30_nor_14 && aux_level4_binary_counter_4bit_0_ff3_30_node_1;
    aux_level4_binary_counter_4bit_0_ff3_30_or_8 = aux_level4_binary_counter_4bit_0_ff3_30_and_16 || aux_level4_binary_counter_4bit_0_ff3_30_not_5;
    aux_level4_binary_counter_4bit_0_ff3_30_or_9 = aux_level4_binary_counter_4bit_0_ff3_30_and_13 || aux_level4_binary_counter_4bit_0_ff3_30_not_6;
    aux_level4_binary_counter_4bit_0_ff3_30_nor_18 = !(aux_level4_binary_counter_4bit_0_ff3_30_or_7 || aux_level4_binary_counter_4bit_0_ff3_30_nor_19);
    aux_level4_binary_counter_4bit_0_ff3_30_nor_14 = !(aux_level4_binary_counter_4bit_0_ff3_30_or_9 || aux_level4_binary_counter_4bit_0_ff3_30_nor_15);
    aux_level4_binary_counter_4bit_0_ff3_30_nor_19 = !(aux_level4_binary_counter_4bit_0_ff3_30_or_8 || aux_level4_binary_counter_4bit_0_ff3_30_nor_18);
    aux_level4_binary_counter_4bit_0_ff3_30_node_20 = aux_level4_binary_counter_4bit_0_ff3_30_nor_18;
    aux_level4_binary_counter_4bit_0_ff3_30_node_21 = aux_level4_binary_counter_4bit_0_ff3_30_nor_19;
    aux_level4_binary_counter_4bit_0_ff3_30_q = aux_level4_binary_counter_4bit_0_ff3_30_node_20;
    aux_level4_binary_counter_4bit_0_ff3_30_q_bar_1 = aux_level4_binary_counter_4bit_0_ff3_30_node_21;
    // End IC: FF3
    aux_level4_binary_counter_4bit_0_not_10 = !aux_level4_binary_counter_4bit_0_ff1_28_q;
    aux_level4_binary_counter_4bit_0_not_11 = !aux_level4_binary_counter_4bit_0_ff2_29_q;
    aux_level4_binary_counter_4bit_0_not_12 = !aux_level4_binary_counter_4bit_0_ff3_30_q;
    aux_level4_binary_counter_4bit_0_not_15 = !aux_level4_binary_counter_4bit_0_ff0_27_q;
    aux_level4_binary_counter_4bit_0_not_16 = !aux_level4_binary_counter_4bit_0_and_13;
    aux_level4_binary_counter_4bit_0_not_17 = !aux_level4_binary_counter_4bit_0_and_14;
    aux_level4_binary_counter_4bit_0_and_18 = aux_level4_binary_counter_4bit_0_not_10 && aux_level4_binary_counter_4bit_0_ff0_27_q;
    aux_level4_binary_counter_4bit_0_and_19 = aux_level4_binary_counter_4bit_0_ff1_28_q && aux_level4_binary_counter_4bit_0_not_15;
    aux_level4_binary_counter_4bit_0_and_20 = aux_level4_binary_counter_4bit_0_not_11 && aux_level4_binary_counter_4bit_0_and_13;
    aux_level4_binary_counter_4bit_0_and_21 = aux_level4_binary_counter_4bit_0_ff2_29_q && aux_level4_binary_counter_4bit_0_not_16;
    aux_level4_binary_counter_4bit_0_and_22 = aux_level4_binary_counter_4bit_0_not_12 && aux_level4_binary_counter_4bit_0_and_14;
    aux_level4_binary_counter_4bit_0_and_23 = aux_level4_binary_counter_4bit_0_ff3_30_q && aux_level4_binary_counter_4bit_0_not_17;
    // IC: hold_mux0
    aux_ic_input_level4_binary_counter_4bit_0_hold_mux0_31_0 = aux_level4_binary_counter_4bit_0_ff0_27_q;
    aux_ic_input_level4_binary_counter_4bit_0_hold_mux0_31_1 = aux_level4_binary_counter_4bit_0_not_9;
    aux_ic_input_level4_binary_counter_4bit_0_hold_mux0_31_2 = aux_level4_binary_counter_4bit_0_node_1;
    aux_ic_input_level4_binary_counter_4bit_0_hold_mux0_31_3 = HIGH;
    aux_level4_binary_counter_4bit_0_hold_mux0_31_node_0 = aux_ic_input_level4_binary_counter_4bit_0_hold_mux0_31_0;
    aux_level4_binary_counter_4bit_0_hold_mux0_31_node_1 = aux_ic_input_level4_binary_counter_4bit_0_hold_mux0_31_1;
    aux_level4_binary_counter_4bit_0_hold_mux0_31_node_2 = aux_ic_input_level4_binary_counter_4bit_0_hold_mux0_31_2;
    aux_level4_binary_counter_4bit_0_hold_mux0_31_node_4 = aux_ic_input_level4_binary_counter_4bit_0_hold_mux0_31_3;
    //Multiplexer
    if ((aux_level4_binary_counter_4bit_0_hold_mux0_31_node_2) == 0) {
        aux_level4_binary_counter_4bit_0_hold_mux0_31_mux_3 = aux_level4_binary_counter_4bit_0_hold_mux0_31_node_0;
    } else if ((aux_level4_binary_counter_4bit_0_hold_mux0_31_node_2) == 1) {
        aux_level4_binary_counter_4bit_0_hold_mux0_31_mux_3 = aux_level4_binary_counter_4bit_0_hold_mux0_31_node_1;
    } else {
        aux_level4_binary_counter_4bit_0_hold_mux0_31_mux_3 = LOW;
    }
    //End of Multiplexer
    aux_level4_binary_counter_4bit_0_hold_mux0_31_and_5 = aux_level4_binary_counter_4bit_0_hold_mux0_31_mux_3 && aux_level4_binary_counter_4bit_0_hold_mux0_31_node_4;
    aux_level4_binary_counter_4bit_0_hold_mux0_31_node_6 = aux_level4_binary_counter_4bit_0_hold_mux0_31_and_5;
    aux_level4_binary_counter_4bit_0_hold_mux0_31_output = aux_level4_binary_counter_4bit_0_hold_mux0_31_node_6;
    // End IC: hold_mux0
    // IC: hold_mux1
    aux_ic_input_level4_binary_counter_4bit_0_hold_mux1_33_0 = aux_level4_binary_counter_4bit_0_ff1_28_q;
    aux_ic_input_level4_binary_counter_4bit_0_hold_mux1_33_1 = aux_level4_binary_counter_4bit_0_or_24;
    aux_ic_input_level4_binary_counter_4bit_0_hold_mux1_33_2 = aux_level4_binary_counter_4bit_0_node_1;
    aux_ic_input_level4_binary_counter_4bit_0_hold_mux1_33_3 = HIGH;
    aux_level4_binary_counter_4bit_0_hold_mux1_33_node_0 = aux_ic_input_level4_binary_counter_4bit_0_hold_mux1_33_0;
    aux_level4_binary_counter_4bit_0_hold_mux1_33_node_1 = aux_ic_input_level4_binary_counter_4bit_0_hold_mux1_33_1;
    aux_level4_binary_counter_4bit_0_hold_mux1_33_node_2 = aux_ic_input_level4_binary_counter_4bit_0_hold_mux1_33_2;
    aux_level4_binary_counter_4bit_0_hold_mux1_33_node_4 = aux_ic_input_level4_binary_counter_4bit_0_hold_mux1_33_3;
    //Multiplexer
    if ((aux_level4_binary_counter_4bit_0_hold_mux1_33_node_2) == 0) {
        aux_level4_binary_counter_4bit_0_hold_mux1_33_mux_3 = aux_level4_binary_counter_4bit_0_hold_mux1_33_node_0;
    } else if ((aux_level4_binary_counter_4bit_0_hold_mux1_33_node_2) == 1) {
        aux_level4_binary_counter_4bit_0_hold_mux1_33_mux_3 = aux_level4_binary_counter_4bit_0_hold_mux1_33_node_1;
    } else {
        aux_level4_binary_counter_4bit_0_hold_mux1_33_mux_3 = LOW;
    }
    //End of Multiplexer
    aux_level4_binary_counter_4bit_0_hold_mux1_33_and_5 = aux_level4_binary_counter_4bit_0_hold_mux1_33_mux_3 && aux_level4_binary_counter_4bit_0_hold_mux1_33_node_4;
    aux_level4_binary_counter_4bit_0_hold_mux1_33_node_6 = aux_level4_binary_counter_4bit_0_hold_mux1_33_and_5;
    aux_level4_binary_counter_4bit_0_hold_mux1_33_output = aux_level4_binary_counter_4bit_0_hold_mux1_33_node_6;
    // End IC: hold_mux1
    // IC: hold_mux2
    aux_ic_input_level4_binary_counter_4bit_0_hold_mux2_35_0 = aux_level4_binary_counter_4bit_0_ff2_29_q;
    aux_ic_input_level4_binary_counter_4bit_0_hold_mux2_35_1 = aux_level4_binary_counter_4bit_0_or_25;
    aux_ic_input_level4_binary_counter_4bit_0_hold_mux2_35_2 = aux_level4_binary_counter_4bit_0_node_1;
    aux_ic_input_level4_binary_counter_4bit_0_hold_mux2_35_3 = HIGH;
    aux_level4_binary_counter_4bit_0_hold_mux2_35_node_0 = aux_ic_input_level4_binary_counter_4bit_0_hold_mux2_35_0;
    aux_level4_binary_counter_4bit_0_hold_mux2_35_node_1 = aux_ic_input_level4_binary_counter_4bit_0_hold_mux2_35_1;
    aux_level4_binary_counter_4bit_0_hold_mux2_35_node_2 = aux_ic_input_level4_binary_counter_4bit_0_hold_mux2_35_2;
    aux_level4_binary_counter_4bit_0_hold_mux2_35_node_4 = aux_ic_input_level4_binary_counter_4bit_0_hold_mux2_35_3;
    //Multiplexer
    if ((aux_level4_binary_counter_4bit_0_hold_mux2_35_node_2) == 0) {
        aux_level4_binary_counter_4bit_0_hold_mux2_35_mux_3 = aux_level4_binary_counter_4bit_0_hold_mux2_35_node_0;
    } else if ((aux_level4_binary_counter_4bit_0_hold_mux2_35_node_2) == 1) {
        aux_level4_binary_counter_4bit_0_hold_mux2_35_mux_3 = aux_level4_binary_counter_4bit_0_hold_mux2_35_node_1;
    } else {
        aux_level4_binary_counter_4bit_0_hold_mux2_35_mux_3 = LOW;
    }
    //End of Multiplexer
    aux_level4_binary_counter_4bit_0_hold_mux2_35_and_5 = aux_level4_binary_counter_4bit_0_hold_mux2_35_mux_3 && aux_level4_binary_counter_4bit_0_hold_mux2_35_node_4;
    aux_level4_binary_counter_4bit_0_hold_mux2_35_node_6 = aux_level4_binary_counter_4bit_0_hold_mux2_35_and_5;
    aux_level4_binary_counter_4bit_0_hold_mux2_35_output = aux_level4_binary_counter_4bit_0_hold_mux2_35_node_6;
    // End IC: hold_mux2
    // IC: hold_mux3
    aux_ic_input_level4_binary_counter_4bit_0_hold_mux3_37_0 = aux_level4_binary_counter_4bit_0_ff3_30_q;
    aux_ic_input_level4_binary_counter_4bit_0_hold_mux3_37_1 = aux_level4_binary_counter_4bit_0_or_26;
    aux_ic_input_level4_binary_counter_4bit_0_hold_mux3_37_2 = aux_level4_binary_counter_4bit_0_node_1;
    aux_ic_input_level4_binary_counter_4bit_0_hold_mux3_37_3 = HIGH;
    aux_level4_binary_counter_4bit_0_hold_mux3_37_node_0 = aux_ic_input_level4_binary_counter_4bit_0_hold_mux3_37_0;
    aux_level4_binary_counter_4bit_0_hold_mux3_37_node_1 = aux_ic_input_level4_binary_counter_4bit_0_hold_mux3_37_1;
    aux_level4_binary_counter_4bit_0_hold_mux3_37_node_2 = aux_ic_input_level4_binary_counter_4bit_0_hold_mux3_37_2;
    aux_level4_binary_counter_4bit_0_hold_mux3_37_node_4 = aux_ic_input_level4_binary_counter_4bit_0_hold_mux3_37_3;
    //Multiplexer
    if ((aux_level4_binary_counter_4bit_0_hold_mux3_37_node_2) == 0) {
        aux_level4_binary_counter_4bit_0_hold_mux3_37_mux_3 = aux_level4_binary_counter_4bit_0_hold_mux3_37_node_0;
    } else if ((aux_level4_binary_counter_4bit_0_hold_mux3_37_node_2) == 1) {
        aux_level4_binary_counter_4bit_0_hold_mux3_37_mux_3 = aux_level4_binary_counter_4bit_0_hold_mux3_37_node_1;
    } else {
        aux_level4_binary_counter_4bit_0_hold_mux3_37_mux_3 = LOW;
    }
    //End of Multiplexer
    aux_level4_binary_counter_4bit_0_hold_mux3_37_and_5 = aux_level4_binary_counter_4bit_0_hold_mux3_37_mux_3 && aux_level4_binary_counter_4bit_0_hold_mux3_37_node_4;
    aux_level4_binary_counter_4bit_0_hold_mux3_37_node_6 = aux_level4_binary_counter_4bit_0_hold_mux3_37_and_5;
    aux_level4_binary_counter_4bit_0_hold_mux3_37_output = aux_level4_binary_counter_4bit_0_hold_mux3_37_node_6;
    // End IC: hold_mux3
    aux_level4_binary_counter_4bit_0_not_9 = !aux_level4_binary_counter_4bit_0_ff0_27_q;
    aux_level4_binary_counter_4bit_0_or_24 = aux_level4_binary_counter_4bit_0_and_18 || aux_level4_binary_counter_4bit_0_and_19;
    aux_level4_binary_counter_4bit_0_or_25 = aux_level4_binary_counter_4bit_0_and_20 || aux_level4_binary_counter_4bit_0_and_21;
    aux_level4_binary_counter_4bit_0_or_26 = aux_level4_binary_counter_4bit_0_and_22 || aux_level4_binary_counter_4bit_0_and_23;
    // IC: load_mux0
    aux_ic_input_level4_binary_counter_4bit_0_load_mux0_32_0 = aux_level4_binary_counter_4bit_0_hold_mux0_31_output;
    aux_ic_input_level4_binary_counter_4bit_0_load_mux0_32_1 = aux_level4_binary_counter_4bit_0_node_4;
    aux_ic_input_level4_binary_counter_4bit_0_load_mux0_32_2 = aux_level4_binary_counter_4bit_0_node_2;
    aux_ic_input_level4_binary_counter_4bit_0_load_mux0_32_3 = HIGH;
    aux_level4_binary_counter_4bit_0_load_mux0_32_node_0 = aux_ic_input_level4_binary_counter_4bit_0_load_mux0_32_0;
    aux_level4_binary_counter_4bit_0_load_mux0_32_node_1 = aux_ic_input_level4_binary_counter_4bit_0_load_mux0_32_1;
    aux_level4_binary_counter_4bit_0_load_mux0_32_node_2 = aux_ic_input_level4_binary_counter_4bit_0_load_mux0_32_2;
    aux_level4_binary_counter_4bit_0_load_mux0_32_node_4 = aux_ic_input_level4_binary_counter_4bit_0_load_mux0_32_3;
    //Multiplexer
    if ((aux_level4_binary_counter_4bit_0_load_mux0_32_node_2) == 0) {
        aux_level4_binary_counter_4bit_0_load_mux0_32_mux_3 = aux_level4_binary_counter_4bit_0_load_mux0_32_node_0;
    } else if ((aux_level4_binary_counter_4bit_0_load_mux0_32_node_2) == 1) {
        aux_level4_binary_counter_4bit_0_load_mux0_32_mux_3 = aux_level4_binary_counter_4bit_0_load_mux0_32_node_1;
    } else {
        aux_level4_binary_counter_4bit_0_load_mux0_32_mux_3 = LOW;
    }
    //End of Multiplexer
    aux_level4_binary_counter_4bit_0_load_mux0_32_and_5 = aux_level4_binary_counter_4bit_0_load_mux0_32_mux_3 && aux_level4_binary_counter_4bit_0_load_mux0_32_node_4;
    aux_level4_binary_counter_4bit_0_load_mux0_32_node_6 = aux_level4_binary_counter_4bit_0_load_mux0_32_and_5;
    aux_level4_binary_counter_4bit_0_load_mux0_32_output = aux_level4_binary_counter_4bit_0_load_mux0_32_node_6;
    // End IC: load_mux0
    // IC: load_mux1
    aux_ic_input_level4_binary_counter_4bit_0_load_mux1_34_0 = aux_level4_binary_counter_4bit_0_hold_mux1_33_output;
    aux_ic_input_level4_binary_counter_4bit_0_load_mux1_34_1 = aux_level4_binary_counter_4bit_0_node_5;
    aux_ic_input_level4_binary_counter_4bit_0_load_mux1_34_2 = aux_level4_binary_counter_4bit_0_node_2;
    aux_ic_input_level4_binary_counter_4bit_0_load_mux1_34_3 = HIGH;
    aux_level4_binary_counter_4bit_0_load_mux1_34_node_0 = aux_ic_input_level4_binary_counter_4bit_0_load_mux1_34_0;
    aux_level4_binary_counter_4bit_0_load_mux1_34_node_1 = aux_ic_input_level4_binary_counter_4bit_0_load_mux1_34_1;
    aux_level4_binary_counter_4bit_0_load_mux1_34_node_2 = aux_ic_input_level4_binary_counter_4bit_0_load_mux1_34_2;
    aux_level4_binary_counter_4bit_0_load_mux1_34_node_4 = aux_ic_input_level4_binary_counter_4bit_0_load_mux1_34_3;
    //Multiplexer
    if ((aux_level4_binary_counter_4bit_0_load_mux1_34_node_2) == 0) {
        aux_level4_binary_counter_4bit_0_load_mux1_34_mux_3 = aux_level4_binary_counter_4bit_0_load_mux1_34_node_0;
    } else if ((aux_level4_binary_counter_4bit_0_load_mux1_34_node_2) == 1) {
        aux_level4_binary_counter_4bit_0_load_mux1_34_mux_3 = aux_level4_binary_counter_4bit_0_load_mux1_34_node_1;
    } else {
        aux_level4_binary_counter_4bit_0_load_mux1_34_mux_3 = LOW;
    }
    //End of Multiplexer
    aux_level4_binary_counter_4bit_0_load_mux1_34_and_5 = aux_level4_binary_counter_4bit_0_load_mux1_34_mux_3 && aux_level4_binary_counter_4bit_0_load_mux1_34_node_4;
    aux_level4_binary_counter_4bit_0_load_mux1_34_node_6 = aux_level4_binary_counter_4bit_0_load_mux1_34_and_5;
    aux_level4_binary_counter_4bit_0_load_mux1_34_output = aux_level4_binary_counter_4bit_0_load_mux1_34_node_6;
    // End IC: load_mux1
    // IC: load_mux2
    aux_ic_input_level4_binary_counter_4bit_0_load_mux2_36_0 = aux_level4_binary_counter_4bit_0_hold_mux2_35_output;
    aux_ic_input_level4_binary_counter_4bit_0_load_mux2_36_1 = aux_level4_binary_counter_4bit_0_node_6;
    aux_ic_input_level4_binary_counter_4bit_0_load_mux2_36_2 = aux_level4_binary_counter_4bit_0_node_2;
    aux_ic_input_level4_binary_counter_4bit_0_load_mux2_36_3 = HIGH;
    aux_level4_binary_counter_4bit_0_load_mux2_36_node_0 = aux_ic_input_level4_binary_counter_4bit_0_load_mux2_36_0;
    aux_level4_binary_counter_4bit_0_load_mux2_36_node_1 = aux_ic_input_level4_binary_counter_4bit_0_load_mux2_36_1;
    aux_level4_binary_counter_4bit_0_load_mux2_36_node_2 = aux_ic_input_level4_binary_counter_4bit_0_load_mux2_36_2;
    aux_level4_binary_counter_4bit_0_load_mux2_36_node_4 = aux_ic_input_level4_binary_counter_4bit_0_load_mux2_36_3;
    //Multiplexer
    if ((aux_level4_binary_counter_4bit_0_load_mux2_36_node_2) == 0) {
        aux_level4_binary_counter_4bit_0_load_mux2_36_mux_3 = aux_level4_binary_counter_4bit_0_load_mux2_36_node_0;
    } else if ((aux_level4_binary_counter_4bit_0_load_mux2_36_node_2) == 1) {
        aux_level4_binary_counter_4bit_0_load_mux2_36_mux_3 = aux_level4_binary_counter_4bit_0_load_mux2_36_node_1;
    } else {
        aux_level4_binary_counter_4bit_0_load_mux2_36_mux_3 = LOW;
    }
    //End of Multiplexer
    aux_level4_binary_counter_4bit_0_load_mux2_36_and_5 = aux_level4_binary_counter_4bit_0_load_mux2_36_mux_3 && aux_level4_binary_counter_4bit_0_load_mux2_36_node_4;
    aux_level4_binary_counter_4bit_0_load_mux2_36_node_6 = aux_level4_binary_counter_4bit_0_load_mux2_36_and_5;
    aux_level4_binary_counter_4bit_0_load_mux2_36_output = aux_level4_binary_counter_4bit_0_load_mux2_36_node_6;
    // End IC: load_mux2
    // IC: load_mux3
    aux_ic_input_level4_binary_counter_4bit_0_load_mux3_38_0 = aux_level4_binary_counter_4bit_0_hold_mux3_37_output;
    aux_ic_input_level4_binary_counter_4bit_0_load_mux3_38_1 = aux_level4_binary_counter_4bit_0_node_7;
    aux_ic_input_level4_binary_counter_4bit_0_load_mux3_38_2 = aux_level4_binary_counter_4bit_0_node_2;
    aux_ic_input_level4_binary_counter_4bit_0_load_mux3_38_3 = HIGH;
    aux_level4_binary_counter_4bit_0_load_mux3_38_node_0 = aux_ic_input_level4_binary_counter_4bit_0_load_mux3_38_0;
    aux_level4_binary_counter_4bit_0_load_mux3_38_node_1 = aux_ic_input_level4_binary_counter_4bit_0_load_mux3_38_1;
    aux_level4_binary_counter_4bit_0_load_mux3_38_node_2 = aux_ic_input_level4_binary_counter_4bit_0_load_mux3_38_2;
    aux_level4_binary_counter_4bit_0_load_mux3_38_node_4 = aux_ic_input_level4_binary_counter_4bit_0_load_mux3_38_3;
    //Multiplexer
    if ((aux_level4_binary_counter_4bit_0_load_mux3_38_node_2) == 0) {
        aux_level4_binary_counter_4bit_0_load_mux3_38_mux_3 = aux_level4_binary_counter_4bit_0_load_mux3_38_node_0;
    } else if ((aux_level4_binary_counter_4bit_0_load_mux3_38_node_2) == 1) {
        aux_level4_binary_counter_4bit_0_load_mux3_38_mux_3 = aux_level4_binary_counter_4bit_0_load_mux3_38_node_1;
    } else {
        aux_level4_binary_counter_4bit_0_load_mux3_38_mux_3 = LOW;
    }
    //End of Multiplexer
    aux_level4_binary_counter_4bit_0_load_mux3_38_and_5 = aux_level4_binary_counter_4bit_0_load_mux3_38_mux_3 && aux_level4_binary_counter_4bit_0_load_mux3_38_node_4;
    aux_level4_binary_counter_4bit_0_load_mux3_38_node_6 = aux_level4_binary_counter_4bit_0_load_mux3_38_and_5;
    aux_level4_binary_counter_4bit_0_load_mux3_38_output = aux_level4_binary_counter_4bit_0_load_mux3_38_node_6;
    // End IC: load_mux3
    aux_level4_binary_counter_4bit_0_node_39 = aux_level4_binary_counter_4bit_0_ff0_27_q;
    aux_level4_binary_counter_4bit_0_node_40 = aux_level4_binary_counter_4bit_0_ff1_28_q;
    aux_level4_binary_counter_4bit_0_node_41 = aux_level4_binary_counter_4bit_0_ff2_29_q;
    aux_level4_binary_counter_4bit_0_node_42 = aux_level4_binary_counter_4bit_0_ff3_30_q;
    aux_level4_binary_counter_4bit_0_q0 = aux_level4_binary_counter_4bit_0_node_39;
    aux_level4_binary_counter_4bit_0_q1_1 = aux_level4_binary_counter_4bit_0_node_40;
    aux_level4_binary_counter_4bit_0_q2_2 = aux_level4_binary_counter_4bit_0_node_41;
    aux_level4_binary_counter_4bit_0_q3_3 = aux_level4_binary_counter_4bit_0_node_42;
    // End IC: LEVEL4_BINARY_COUNTER_4BIT
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
    digitalWrite(led1_1, aux_level4_binary_counter_4bit_0_q0);
    digitalWrite(led2_1, aux_level4_binary_counter_4bit_0_q1_1);
    digitalWrite(led3_1, aux_level4_binary_counter_4bit_0_q2_2);
    digitalWrite(led4_1, aux_level4_binary_counter_4bit_0_q3_3);
}
