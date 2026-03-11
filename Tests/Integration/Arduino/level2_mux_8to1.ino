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
const int input_switch9 = 4;
const int input_switch10 = 5;
const int input_switch11 = 6;

/* ========= Outputs ========== */
const int led1_0 = 7;

/* ====== Aux. Variables ====== */
bool input_switch1_val = LOW;
bool input_switch2_val = LOW;
bool input_switch3_val = LOW;
bool input_switch4_val = LOW;
bool input_switch5_val = LOW;
bool input_switch6_val = LOW;
bool input_switch7_val = LOW;
bool input_switch8_val = LOW;
bool input_switch9_val = LOW;
bool input_switch10_val = LOW;
bool input_switch11_val = LOW;
// IC: LEVEL2_MUX_8TO1
bool aux_level2_mux_8to1_0_output = LOW;
bool aux_level2_mux_8to1_0_node_0 = LOW;
bool aux_level2_mux_8to1_0_node_1 = LOW;
bool aux_level2_mux_8to1_0_node_2 = LOW;
bool aux_level2_mux_8to1_0_node_3 = LOW;
bool aux_level2_mux_8to1_0_node_4 = LOW;
bool aux_level2_mux_8to1_0_node_5 = LOW;
bool aux_level2_mux_8to1_0_node_6 = LOW;
bool aux_level2_mux_8to1_0_node_7 = LOW;
bool aux_level2_mux_8to1_0_node_8 = LOW;
bool aux_level2_mux_8to1_0_node_9 = LOW;
bool aux_level2_mux_8to1_0_node_10 = LOW;
bool aux_level2_mux_8to1_0_mux_11 = LOW;
bool aux_level2_mux_8to1_0_node_12 = LOW;
bool aux_ic_input_level2_mux_8to1_0 = LOW;
bool aux_ic_input_level2_mux_8to1_1 = LOW;
bool aux_ic_input_level2_mux_8to1_2 = LOW;
bool aux_ic_input_level2_mux_8to1_3 = LOW;
bool aux_ic_input_level2_mux_8to1_4 = LOW;
bool aux_ic_input_level2_mux_8to1_5 = LOW;
bool aux_ic_input_level2_mux_8to1_6 = LOW;
bool aux_ic_input_level2_mux_8to1_7 = LOW;
bool aux_ic_input_level2_mux_8to1_8 = LOW;
bool aux_ic_input_level2_mux_8to1_9 = LOW;
bool aux_ic_input_level2_mux_8to1_10 = LOW;
// End IC: LEVEL2_MUX_8TO1

void setup() {
    pinMode(input_switch1, INPUT);
    pinMode(input_switch2, INPUT);
    pinMode(input_switch3, INPUT);
    pinMode(input_switch4, INPUT);
    pinMode(input_switch5, INPUT);
    pinMode(input_switch6, INPUT);
    pinMode(input_switch7, INPUT);
    pinMode(input_switch8, INPUT);
    pinMode(input_switch9, INPUT);
    pinMode(input_switch10, INPUT);
    pinMode(input_switch11, INPUT);
    pinMode(led1_0, OUTPUT);
}

void computeLogic() {
    // Assigning aux variables. //
    // IC: LEVEL2_MUX_8TO1
    aux_ic_input_level2_mux_8to1_0 = input_switch1_val;
    aux_ic_input_level2_mux_8to1_1 = input_switch2_val;
    aux_ic_input_level2_mux_8to1_2 = input_switch3_val;
    aux_ic_input_level2_mux_8to1_3 = input_switch4_val;
    aux_ic_input_level2_mux_8to1_4 = input_switch5_val;
    aux_ic_input_level2_mux_8to1_5 = input_switch6_val;
    aux_ic_input_level2_mux_8to1_6 = input_switch7_val;
    aux_ic_input_level2_mux_8to1_7 = input_switch8_val;
    aux_ic_input_level2_mux_8to1_8 = input_switch9_val;
    aux_ic_input_level2_mux_8to1_9 = input_switch10_val;
    aux_ic_input_level2_mux_8to1_10 = input_switch11_val;
    aux_level2_mux_8to1_0_node_0 = aux_ic_input_level2_mux_8to1_0;
    aux_level2_mux_8to1_0_node_1 = aux_ic_input_level2_mux_8to1_1;
    aux_level2_mux_8to1_0_node_2 = aux_ic_input_level2_mux_8to1_2;
    aux_level2_mux_8to1_0_node_3 = aux_ic_input_level2_mux_8to1_3;
    aux_level2_mux_8to1_0_node_4 = aux_ic_input_level2_mux_8to1_4;
    aux_level2_mux_8to1_0_node_5 = aux_ic_input_level2_mux_8to1_5;
    aux_level2_mux_8to1_0_node_6 = aux_ic_input_level2_mux_8to1_6;
    aux_level2_mux_8to1_0_node_7 = aux_ic_input_level2_mux_8to1_7;
    aux_level2_mux_8to1_0_node_8 = aux_ic_input_level2_mux_8to1_8;
    aux_level2_mux_8to1_0_node_9 = aux_ic_input_level2_mux_8to1_9;
    aux_level2_mux_8to1_0_node_10 = aux_ic_input_level2_mux_8to1_10;
    //Multiplexer
    if ((aux_level2_mux_8to1_0_node_8 + (aux_level2_mux_8to1_0_node_9 + (aux_level2_mux_8to1_0_node_10 << 1) << 1)) == 0) {
        aux_level2_mux_8to1_0_mux_11 = aux_level2_mux_8to1_0_node_0;
    } else if ((aux_level2_mux_8to1_0_node_8 + (aux_level2_mux_8to1_0_node_9 + (aux_level2_mux_8to1_0_node_10 << 1) << 1)) == 1) {
        aux_level2_mux_8to1_0_mux_11 = aux_level2_mux_8to1_0_node_1;
    } else if ((aux_level2_mux_8to1_0_node_8 + (aux_level2_mux_8to1_0_node_9 + (aux_level2_mux_8to1_0_node_10 << 1) << 1)) == 2) {
        aux_level2_mux_8to1_0_mux_11 = aux_level2_mux_8to1_0_node_2;
    } else if ((aux_level2_mux_8to1_0_node_8 + (aux_level2_mux_8to1_0_node_9 + (aux_level2_mux_8to1_0_node_10 << 1) << 1)) == 3) {
        aux_level2_mux_8to1_0_mux_11 = aux_level2_mux_8to1_0_node_3;
    } else if ((aux_level2_mux_8to1_0_node_8 + (aux_level2_mux_8to1_0_node_9 + (aux_level2_mux_8to1_0_node_10 << 1) << 1)) == 4) {
        aux_level2_mux_8to1_0_mux_11 = aux_level2_mux_8to1_0_node_4;
    } else if ((aux_level2_mux_8to1_0_node_8 + (aux_level2_mux_8to1_0_node_9 + (aux_level2_mux_8to1_0_node_10 << 1) << 1)) == 5) {
        aux_level2_mux_8to1_0_mux_11 = aux_level2_mux_8to1_0_node_5;
    } else if ((aux_level2_mux_8to1_0_node_8 + (aux_level2_mux_8to1_0_node_9 + (aux_level2_mux_8to1_0_node_10 << 1) << 1)) == 6) {
        aux_level2_mux_8to1_0_mux_11 = aux_level2_mux_8to1_0_node_6;
    } else if ((aux_level2_mux_8to1_0_node_8 + (aux_level2_mux_8to1_0_node_9 + (aux_level2_mux_8to1_0_node_10 << 1) << 1)) == 7) {
        aux_level2_mux_8to1_0_mux_11 = aux_level2_mux_8to1_0_node_7;
    } else {
        aux_level2_mux_8to1_0_mux_11 = LOW;
    }
    //End of Multiplexer
    aux_level2_mux_8to1_0_node_12 = aux_level2_mux_8to1_0_mux_11;
    aux_level2_mux_8to1_0_output = aux_level2_mux_8to1_0_node_12;
    // End IC: LEVEL2_MUX_8TO1
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
    input_switch9_val = digitalRead(input_switch9);
    input_switch10_val = digitalRead(input_switch10);
    input_switch11_val = digitalRead(input_switch11);

    // Updating clocks. //

    computeLogic();

    // Writing output data. //
    digitalWrite(led1_0, aux_level2_mux_8to1_0_output);
}
