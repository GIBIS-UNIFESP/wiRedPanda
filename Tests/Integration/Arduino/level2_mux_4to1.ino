// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //
//
// Target Board: Arduino UNO R3/R4
// Pin Usage: 7/18 pins
//


#include <elapsedMillis.h>
/* ========= Inputs ========== */
const int input_switch1 = A0;
const int input_switch2 = A1;
const int input_switch3 = A2;
const int input_switch4 = A3;
const int input_switch5 = A4;
const int input_switch6 = A5;

/* ========= Outputs ========== */
const int led1_1 = 2;

/* ====== Aux. Variables ====== */
bool input_switch1_val = LOW;
bool input_switch2_val = LOW;
bool input_switch3_val = LOW;
bool input_switch4_val = LOW;
bool input_switch5_val = LOW;
bool input_switch6_val = LOW;
// IC: LEVEL2_MUX_4TO1
bool aux_level2_mux_4to1_0_output = LOW;
bool aux_level2_mux_4to1_0_node_0 = LOW;
bool aux_level2_mux_4to1_0_node_1 = LOW;
bool aux_level2_mux_4to1_0_node_2 = LOW;
bool aux_level2_mux_4to1_0_node_3 = LOW;
bool aux_level2_mux_4to1_0_node_4 = LOW;
bool aux_level2_mux_4to1_0_node_5 = LOW;
bool aux_level2_mux_4to1_0_mux_6 = LOW;
bool aux_level2_mux_4to1_0_node_7 = LOW;
bool aux_ic_input_level2_mux_4to1_0 = LOW;
bool aux_ic_input_level2_mux_4to1_1 = LOW;
bool aux_ic_input_level2_mux_4to1_2 = LOW;
bool aux_ic_input_level2_mux_4to1_3 = LOW;
bool aux_ic_input_level2_mux_4to1_4 = LOW;
bool aux_ic_input_level2_mux_4to1_5 = LOW;
// End IC: LEVEL2_MUX_4TO1

void setup() {
    pinMode(input_switch1, INPUT);
    pinMode(input_switch2, INPUT);
    pinMode(input_switch3, INPUT);
    pinMode(input_switch4, INPUT);
    pinMode(input_switch5, INPUT);
    pinMode(input_switch6, INPUT);
    pinMode(led1_1, OUTPUT);
}

void computeLogic() {
    // Assigning aux variables. //
    // IC: LEVEL2_MUX_4TO1
    aux_ic_input_level2_mux_4to1_0 = input_switch1_val;
    aux_ic_input_level2_mux_4to1_1 = input_switch2_val;
    aux_ic_input_level2_mux_4to1_2 = input_switch3_val;
    aux_ic_input_level2_mux_4to1_3 = input_switch4_val;
    aux_ic_input_level2_mux_4to1_4 = input_switch5_val;
    aux_ic_input_level2_mux_4to1_5 = input_switch6_val;
    aux_level2_mux_4to1_0_node_0 = aux_ic_input_level2_mux_4to1_0;
    aux_level2_mux_4to1_0_node_1 = aux_ic_input_level2_mux_4to1_1;
    aux_level2_mux_4to1_0_node_2 = aux_ic_input_level2_mux_4to1_2;
    aux_level2_mux_4to1_0_node_3 = aux_ic_input_level2_mux_4to1_3;
    aux_level2_mux_4to1_0_node_4 = aux_ic_input_level2_mux_4to1_4;
    aux_level2_mux_4to1_0_node_5 = aux_ic_input_level2_mux_4to1_5;
    //Multiplexer
    if ((aux_level2_mux_4to1_0_node_4 + (aux_level2_mux_4to1_0_node_5 << 1)) == 0) {
        aux_level2_mux_4to1_0_mux_6 = aux_level2_mux_4to1_0_node_0;
    } else if ((aux_level2_mux_4to1_0_node_4 + (aux_level2_mux_4to1_0_node_5 << 1)) == 1) {
        aux_level2_mux_4to1_0_mux_6 = aux_level2_mux_4to1_0_node_1;
    } else if ((aux_level2_mux_4to1_0_node_4 + (aux_level2_mux_4to1_0_node_5 << 1)) == 2) {
        aux_level2_mux_4to1_0_mux_6 = aux_level2_mux_4to1_0_node_2;
    } else if ((aux_level2_mux_4to1_0_node_4 + (aux_level2_mux_4to1_0_node_5 << 1)) == 3) {
        aux_level2_mux_4to1_0_mux_6 = aux_level2_mux_4to1_0_node_3;
    } else {
        aux_level2_mux_4to1_0_mux_6 = LOW;
    }
    //End of Multiplexer
    aux_level2_mux_4to1_0_node_7 = aux_level2_mux_4to1_0_mux_6;
    aux_level2_mux_4to1_0_output = aux_level2_mux_4to1_0_node_7;
    // End IC: LEVEL2_MUX_4TO1
}

void loop() {
    // Reading input data //.
    input_switch1_val = digitalRead(input_switch1);
    input_switch2_val = digitalRead(input_switch2);
    input_switch3_val = digitalRead(input_switch3);
    input_switch4_val = digitalRead(input_switch4);
    input_switch5_val = digitalRead(input_switch5);
    input_switch6_val = digitalRead(input_switch6);

    // Updating clocks. //

    computeLogic();

    // Writing output data. //
    digitalWrite(led1_1, aux_level2_mux_4to1_0_output);
}
