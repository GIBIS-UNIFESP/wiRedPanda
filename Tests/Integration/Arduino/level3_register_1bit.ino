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
const int input_switch3 = A2;
const int input_switch4 = A3;

/* ========= Outputs ========== */
const int led1_1 = A4;
const int led2_1 = A5;

/* ====== Aux. Variables ====== */
bool input_switch1_val = LOW;
bool input_switch2_val = LOW;
bool input_switch3_val = LOW;
bool input_switch4_val = LOW;
// IC: LEVEL3_REGISTER_1BIT
bool aux_level3_register_1bit_0_q = LOW;
bool aux_level3_register_1bit_0_notq_1 = LOW;
bool aux_level3_register_1bit_0_node_0 = LOW;
bool aux_level3_register_1bit_0_node_1 = LOW;
bool aux_level3_register_1bit_0_node_2 = LOW;
bool aux_level3_register_1bit_0_node_3 = LOW;
bool aux_level3_register_1bit_0_not_4 = LOW;
bool aux_level3_register_1bit_0_not_5 = LOW;
bool aux_level3_register_1bit_0_mux_6 = LOW;
bool aux_level3_register_1bit_0_d_flip_flop_7_0_q = LOW;
bool aux_level3_register_1bit_0_d_flip_flop_7_0_q_inclk = LOW;
bool aux_level3_register_1bit_0_d_flip_flop_7_0_q_last = LOW;
bool aux_level3_register_1bit_0_d_flip_flop_7_1_q = HIGH;
bool aux_level3_register_1bit_0_d_flip_flop_7_1_q_inclk = LOW;
bool aux_level3_register_1bit_0_d_flip_flop_7_1_q_last = LOW;
bool aux_level3_register_1bit_0_node_9 = LOW;
bool aux_level3_register_1bit_0_node_10 = LOW;
bool aux_ic_input_level3_register_1bit_0 = LOW;
bool aux_ic_input_level3_register_1bit_1 = LOW;
bool aux_ic_input_level3_register_1bit_2 = LOW;
bool aux_ic_input_level3_register_1bit_3 = LOW;
// End IC: LEVEL3_REGISTER_1BIT

void setup() {
    pinMode(input_switch1, INPUT);
    pinMode(input_switch2, INPUT);
    pinMode(input_switch3, INPUT);
    pinMode(input_switch4, INPUT);
    pinMode(led1_1, OUTPUT);
    pinMode(led2_1, OUTPUT);
}

void computeLogic() {
    // Assigning aux variables. //
    // IC: LEVEL3_REGISTER_1BIT
    aux_ic_input_level3_register_1bit_0 = input_switch1_val;
    aux_ic_input_level3_register_1bit_1 = input_switch2_val;
    aux_ic_input_level3_register_1bit_2 = input_switch3_val;
    aux_ic_input_level3_register_1bit_3 = input_switch4_val;
    aux_level3_register_1bit_0_node_2 = aux_ic_input_level3_register_1bit_2;
    aux_level3_register_1bit_0_node_0 = aux_ic_input_level3_register_1bit_0;
    aux_level3_register_1bit_0_node_3 = aux_ic_input_level3_register_1bit_3;
    aux_level3_register_1bit_0_not_5 = !aux_level3_register_1bit_0_node_2;
    aux_level3_register_1bit_0_node_1 = aux_ic_input_level3_register_1bit_1;
    aux_level3_register_1bit_0_not_4 = !aux_level3_register_1bit_0_node_3;
    //Multiplexer
    if ((aux_level3_register_1bit_0_not_5) == 0) {
        aux_level3_register_1bit_0_mux_6 = aux_level3_register_1bit_0_node_0;
    } else if ((aux_level3_register_1bit_0_not_5) == 1) {
        aux_level3_register_1bit_0_mux_6 = aux_level3_register_1bit_0_d_flip_flop_7_0_q;
    } else {
        aux_level3_register_1bit_0_mux_6 = LOW;
    }
    //End of Multiplexer
    //D FlipFlop
    if (aux_level3_register_1bit_0_node_1 && !aux_level3_register_1bit_0_d_flip_flop_7_0_q_inclk) { 
        aux_level3_register_1bit_0_d_flip_flop_7_0_q = aux_level3_register_1bit_0_d_flip_flop_7_0_q_last;
        aux_level3_register_1bit_0_d_flip_flop_7_1_q = !aux_level3_register_1bit_0_d_flip_flop_7_0_q_last;
    }
    if (!HIGH || !aux_level3_register_1bit_0_not_4) { 
        aux_level3_register_1bit_0_d_flip_flop_7_0_q = !HIGH; //Preset
        aux_level3_register_1bit_0_d_flip_flop_7_1_q = !aux_level3_register_1bit_0_not_4; //Clear
    }
    aux_level3_register_1bit_0_d_flip_flop_7_0_q_inclk = aux_level3_register_1bit_0_node_1;
    //End of D FlipFlop
    aux_level3_register_1bit_0_d_flip_flop_7_0_q_last = aux_level3_register_1bit_0_mux_6;
    aux_level3_register_1bit_0_node_9 = aux_level3_register_1bit_0_d_flip_flop_7_0_q;
    aux_level3_register_1bit_0_node_10 = aux_level3_register_1bit_0_d_flip_flop_7_1_q;
    aux_level3_register_1bit_0_q = aux_level3_register_1bit_0_node_9;
    aux_level3_register_1bit_0_notq_1 = aux_level3_register_1bit_0_node_10;
    // End IC: LEVEL3_REGISTER_1BIT
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
    digitalWrite(led1_1, aux_level3_register_1bit_0_q);
    digitalWrite(led2_1, aux_level3_register_1bit_0_notq_1);
}
