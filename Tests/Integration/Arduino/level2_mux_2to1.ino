// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //
//
// Target Board: Arduino UNO R3/R4
// Pin Usage: 4/18 pins
//


#include <elapsedMillis.h>
/* ========= Inputs ========== */
const int input_switch1 = A0;
const int input_switch2 = A1;
const int input_switch3 = A2;

/* ========= Outputs ========== */
const int led1_0 = A3;

/* ====== Aux. Variables ====== */
bool input_switch1_val = LOW;
bool input_switch2_val = LOW;
bool input_switch3_val = LOW;
// IC: LEVEL2_MUX_2TO1
bool aux_level2_mux_2to1_0_output = LOW;
bool aux_level2_mux_2to1_0_node_0 = LOW;
bool aux_level2_mux_2to1_0_node_1 = LOW;
bool aux_level2_mux_2to1_0_node_2 = LOW;
bool aux_level2_mux_2to1_0_mux_3 = LOW;
bool aux_level2_mux_2to1_0_node_4 = LOW;
bool aux_ic_input_level2_mux_2to1_0 = LOW;
bool aux_ic_input_level2_mux_2to1_1 = LOW;
bool aux_ic_input_level2_mux_2to1_2 = LOW;
// End IC: LEVEL2_MUX_2TO1

void setup() {
    pinMode(input_switch1, INPUT);
    pinMode(input_switch2, INPUT);
    pinMode(input_switch3, INPUT);
    pinMode(led1_0, OUTPUT);
}

void computeLogic() {
    // Assigning aux variables. //
    // IC: LEVEL2_MUX_2TO1
    aux_ic_input_level2_mux_2to1_0 = input_switch1_val;
    aux_ic_input_level2_mux_2to1_1 = input_switch2_val;
    aux_ic_input_level2_mux_2to1_2 = input_switch3_val;
    aux_level2_mux_2to1_0_node_0 = aux_ic_input_level2_mux_2to1_0;
    aux_level2_mux_2to1_0_node_1 = aux_ic_input_level2_mux_2to1_1;
    aux_level2_mux_2to1_0_node_2 = aux_ic_input_level2_mux_2to1_2;
    //Multiplexer
    if ((aux_level2_mux_2to1_0_node_2) == 0) {
        aux_level2_mux_2to1_0_mux_3 = aux_level2_mux_2to1_0_node_0;
    } else if ((aux_level2_mux_2to1_0_node_2) == 1) {
        aux_level2_mux_2to1_0_mux_3 = aux_level2_mux_2to1_0_node_1;
    } else {
        aux_level2_mux_2to1_0_mux_3 = LOW;
    }
    //End of Multiplexer
    aux_level2_mux_2to1_0_node_4 = aux_level2_mux_2to1_0_mux_3;
    aux_level2_mux_2to1_0_output = aux_level2_mux_2to1_0_node_4;
    // End IC: LEVEL2_MUX_2TO1
}

void loop() {
    // Reading input data //.
    input_switch1_val = digitalRead(input_switch1);
    input_switch2_val = digitalRead(input_switch2);
    input_switch3_val = digitalRead(input_switch3);

    // Updating clocks. //

    computeLogic();

    // Writing output data. //
    digitalWrite(led1_0, aux_level2_mux_2to1_0_output);
}
