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
// IC: LEVEL3_ALU_SELECTOR_5WAY
bool aux_level3_alu_selector_5way_0_out = LOW;
bool aux_level3_alu_selector_5way_0_node_0 = LOW;
bool aux_level3_alu_selector_5way_0_node_1 = LOW;
bool aux_level3_alu_selector_5way_0_node_2 = LOW;
bool aux_level3_alu_selector_5way_0_node_3 = LOW;
bool aux_level3_alu_selector_5way_0_node_4 = LOW;
bool aux_level3_alu_selector_5way_0_node_5 = LOW;
bool aux_level3_alu_selector_5way_0_node_6 = LOW;
bool aux_level3_alu_selector_5way_0_node_7 = LOW;
bool aux_level3_alu_selector_5way_0_mux_8 = LOW;
bool aux_level3_alu_selector_5way_0_mux_9 = LOW;
bool aux_level3_alu_selector_5way_0_mux_10 = LOW;
bool aux_level3_alu_selector_5way_0_mux_11 = LOW;
bool aux_level3_alu_selector_5way_0_node_12 = LOW;
bool aux_ic_input_level3_alu_selector_5way_0 = LOW;
bool aux_ic_input_level3_alu_selector_5way_1 = LOW;
bool aux_ic_input_level3_alu_selector_5way_2 = LOW;
bool aux_ic_input_level3_alu_selector_5way_3 = LOW;
bool aux_ic_input_level3_alu_selector_5way_4 = LOW;
bool aux_ic_input_level3_alu_selector_5way_5 = LOW;
bool aux_ic_input_level3_alu_selector_5way_6 = LOW;
bool aux_ic_input_level3_alu_selector_5way_7 = LOW;
// End IC: LEVEL3_ALU_SELECTOR_5WAY

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
    // IC: LEVEL3_ALU_SELECTOR_5WAY
    aux_ic_input_level3_alu_selector_5way_0 = input_switch1_val;
    aux_ic_input_level3_alu_selector_5way_1 = input_switch2_val;
    aux_ic_input_level3_alu_selector_5way_2 = input_switch3_val;
    aux_ic_input_level3_alu_selector_5way_3 = input_switch4_val;
    aux_ic_input_level3_alu_selector_5way_4 = input_switch5_val;
    aux_ic_input_level3_alu_selector_5way_5 = input_switch6_val;
    aux_ic_input_level3_alu_selector_5way_6 = input_switch7_val;
    aux_ic_input_level3_alu_selector_5way_7 = input_switch8_val;
    aux_level3_alu_selector_5way_0_node_0 = aux_ic_input_level3_alu_selector_5way_0;
    aux_level3_alu_selector_5way_0_node_1 = aux_ic_input_level3_alu_selector_5way_1;
    aux_level3_alu_selector_5way_0_node_2 = aux_ic_input_level3_alu_selector_5way_2;
    aux_level3_alu_selector_5way_0_node_3 = aux_ic_input_level3_alu_selector_5way_3;
    aux_level3_alu_selector_5way_0_node_5 = aux_ic_input_level3_alu_selector_5way_5;
    aux_level3_alu_selector_5way_0_node_6 = aux_ic_input_level3_alu_selector_5way_6;
    //Multiplexer
    if ((aux_level3_alu_selector_5way_0_node_5) == 0) {
        aux_level3_alu_selector_5way_0_mux_8 = aux_level3_alu_selector_5way_0_node_0;
    } else if ((aux_level3_alu_selector_5way_0_node_5) == 1) {
        aux_level3_alu_selector_5way_0_mux_8 = aux_level3_alu_selector_5way_0_node_1;
    } else {
        aux_level3_alu_selector_5way_0_mux_8 = LOW;
    }
    //End of Multiplexer
    //Multiplexer
    if ((aux_level3_alu_selector_5way_0_node_5) == 0) {
        aux_level3_alu_selector_5way_0_mux_9 = aux_level3_alu_selector_5way_0_node_2;
    } else if ((aux_level3_alu_selector_5way_0_node_5) == 1) {
        aux_level3_alu_selector_5way_0_mux_9 = aux_level3_alu_selector_5way_0_node_3;
    } else {
        aux_level3_alu_selector_5way_0_mux_9 = LOW;
    }
    //End of Multiplexer
    aux_level3_alu_selector_5way_0_node_4 = aux_ic_input_level3_alu_selector_5way_4;
    aux_level3_alu_selector_5way_0_node_7 = aux_ic_input_level3_alu_selector_5way_7;
    //Multiplexer
    if ((aux_level3_alu_selector_5way_0_node_6) == 0) {
        aux_level3_alu_selector_5way_0_mux_10 = aux_level3_alu_selector_5way_0_mux_8;
    } else if ((aux_level3_alu_selector_5way_0_node_6) == 1) {
        aux_level3_alu_selector_5way_0_mux_10 = aux_level3_alu_selector_5way_0_mux_9;
    } else {
        aux_level3_alu_selector_5way_0_mux_10 = LOW;
    }
    //End of Multiplexer
    //Multiplexer
    if ((aux_level3_alu_selector_5way_0_node_7) == 0) {
        aux_level3_alu_selector_5way_0_mux_11 = aux_level3_alu_selector_5way_0_mux_10;
    } else if ((aux_level3_alu_selector_5way_0_node_7) == 1) {
        aux_level3_alu_selector_5way_0_mux_11 = aux_level3_alu_selector_5way_0_node_4;
    } else {
        aux_level3_alu_selector_5way_0_mux_11 = LOW;
    }
    //End of Multiplexer
    aux_level3_alu_selector_5way_0_node_12 = aux_level3_alu_selector_5way_0_mux_11;
    aux_level3_alu_selector_5way_0_out = aux_level3_alu_selector_5way_0_node_12;
    // End IC: LEVEL3_ALU_SELECTOR_5WAY
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
    digitalWrite(led1_0, aux_level3_alu_selector_5way_0_out);
}
