// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //
//
// Target Board: Arduino UNO R3/R4
// Pin Usage: 10/18 pins
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
const int led1_0 = 2;
const int led2_0 = 3;
const int led3_0 = 4;
const int led4_0 = 5;

/* ====== Aux. Variables ====== */
bool input_switch1_val = LOW;
bool input_switch2_val = LOW;
bool input_switch3_val = LOW;
bool input_switch4_val = LOW;
bool input_switch5_val = LOW;
bool input_switch6_val = LOW;
// IC: LEVEL4_REGISTER_4BIT
bool aux_level4_register_4bit_0_q0 = LOW;
bool aux_level4_register_4bit_0_q1_1 = LOW;
bool aux_level4_register_4bit_0_q2_2 = LOW;
bool aux_level4_register_4bit_0_q3_3 = LOW;
bool aux_level4_register_4bit_0_node_0 = LOW;
bool aux_level4_register_4bit_0_node_1 = LOW;
bool aux_level4_register_4bit_0_node_2 = LOW;
bool aux_level4_register_4bit_0_node_3 = LOW;
bool aux_level4_register_4bit_0_node_4 = LOW;
bool aux_level4_register_4bit_0_node_5 = LOW;
// IC: BusMux_LoadHold
bool aux_busmux_loadhold_6_out0 = LOW;
bool aux_busmux_loadhold_6_out1_1 = LOW;
bool aux_busmux_loadhold_6_out2_2 = LOW;
bool aux_busmux_loadhold_6_out3_3 = LOW;
bool aux_busmux_loadhold_6_node_0 = LOW;
bool aux_busmux_loadhold_6_node_1 = LOW;
bool aux_busmux_loadhold_6_node_2 = LOW;
bool aux_busmux_loadhold_6_node_3 = LOW;
bool aux_busmux_loadhold_6_node_4 = LOW;
bool aux_busmux_loadhold_6_node_5 = LOW;
bool aux_busmux_loadhold_6_node_6 = LOW;
bool aux_busmux_loadhold_6_node_7 = LOW;
bool aux_busmux_loadhold_6_node_8 = LOW;
// IC: Mux[0]
bool aux_mux0_9_output = LOW;
bool aux_mux0_9_node_0 = LOW;
bool aux_mux0_9_node_1 = LOW;
bool aux_mux0_9_node_2 = LOW;
bool aux_mux0_9_mux_3 = LOW;
bool aux_mux0_9_node_4 = LOW;
bool aux_ic_input_mux0_0 = LOW;
bool aux_ic_input_mux0_1 = LOW;
bool aux_ic_input_mux0_2 = LOW;
// End IC: Mux[0]
// IC: Mux[1]
bool aux_mux1_10_output = LOW;
bool aux_mux1_10_node_0 = LOW;
bool aux_mux1_10_node_1 = LOW;
bool aux_mux1_10_node_2 = LOW;
bool aux_mux1_10_mux_3 = LOW;
bool aux_mux1_10_node_4 = LOW;
bool aux_ic_input_mux1_0 = LOW;
bool aux_ic_input_mux1_1 = LOW;
bool aux_ic_input_mux1_2 = LOW;
// End IC: Mux[1]
// IC: Mux[2]
bool aux_mux2_11_output = LOW;
bool aux_mux2_11_node_0 = LOW;
bool aux_mux2_11_node_1 = LOW;
bool aux_mux2_11_node_2 = LOW;
bool aux_mux2_11_mux_3 = LOW;
bool aux_mux2_11_node_4 = LOW;
bool aux_ic_input_mux2_0 = LOW;
bool aux_ic_input_mux2_1 = LOW;
bool aux_ic_input_mux2_2 = LOW;
// End IC: Mux[2]
// IC: Mux[3]
bool aux_mux3_12_output = LOW;
bool aux_mux3_12_node_0 = LOW;
bool aux_mux3_12_node_1 = LOW;
bool aux_mux3_12_node_2 = LOW;
bool aux_mux3_12_mux_3 = LOW;
bool aux_mux3_12_node_4 = LOW;
bool aux_ic_input_mux3_0 = LOW;
bool aux_ic_input_mux3_1 = LOW;
bool aux_ic_input_mux3_2 = LOW;
// End IC: Mux[3]
bool aux_busmux_loadhold_6_node_13 = LOW;
bool aux_busmux_loadhold_6_node_14 = LOW;
bool aux_busmux_loadhold_6_node_15 = LOW;
bool aux_busmux_loadhold_6_node_16 = LOW;
bool aux_ic_input_busmux_loadhold_0 = LOW;
bool aux_ic_input_busmux_loadhold_1 = LOW;
bool aux_ic_input_busmux_loadhold_2 = LOW;
bool aux_ic_input_busmux_loadhold_3 = LOW;
bool aux_ic_input_busmux_loadhold_4 = LOW;
bool aux_ic_input_busmux_loadhold_5 = LOW;
bool aux_ic_input_busmux_loadhold_6 = LOW;
bool aux_ic_input_busmux_loadhold_7 = LOW;
bool aux_ic_input_busmux_loadhold_8 = LOW;
// End IC: BusMux_LoadHold
bool aux_level4_register_4bit_0_d_flip_flop_7_0_q = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_7_0_q_inclk = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_7_0_q_last = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_7_1_q = HIGH;
bool aux_level4_register_4bit_0_d_flip_flop_7_1_q_inclk = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_7_1_q_last = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_8_0_q = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_8_0_q_inclk = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_8_0_q_last = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_8_1_q = HIGH;
bool aux_level4_register_4bit_0_d_flip_flop_8_1_q_inclk = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_8_1_q_last = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_9_0_q = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_9_0_q_inclk = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_9_0_q_last = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_9_1_q = HIGH;
bool aux_level4_register_4bit_0_d_flip_flop_9_1_q_inclk = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_9_1_q_last = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_10_0_q = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_10_0_q_inclk = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_10_0_q_last = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_10_1_q = HIGH;
bool aux_level4_register_4bit_0_d_flip_flop_10_1_q_inclk = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_10_1_q_last = LOW;
bool aux_level4_register_4bit_0_node_11 = LOW;
bool aux_level4_register_4bit_0_node_12 = LOW;
bool aux_level4_register_4bit_0_node_13 = LOW;
bool aux_level4_register_4bit_0_node_14 = LOW;
bool aux_level4_register_4bit_0_not_15 = LOW;
bool aux_ic_input_level4_register_4bit_0 = LOW;
bool aux_ic_input_level4_register_4bit_1 = LOW;
bool aux_ic_input_level4_register_4bit_2 = LOW;
bool aux_ic_input_level4_register_4bit_3 = LOW;
bool aux_ic_input_level4_register_4bit_4 = LOW;
bool aux_ic_input_level4_register_4bit_5 = LOW;
// End IC: LEVEL4_REGISTER_4BIT

void setup() {
    pinMode(input_switch1, INPUT);
    pinMode(input_switch2, INPUT);
    pinMode(input_switch3, INPUT);
    pinMode(input_switch4, INPUT);
    pinMode(input_switch5, INPUT);
    pinMode(input_switch6, INPUT);
    pinMode(led1_0, OUTPUT);
    pinMode(led2_0, OUTPUT);
    pinMode(led3_0, OUTPUT);
    pinMode(led4_0, OUTPUT);
}

void computeLogic() {
    // Assigning aux variables. //
    // IC: LEVEL4_REGISTER_4BIT
    aux_ic_input_level4_register_4bit_0 = input_switch1_val;
    aux_ic_input_level4_register_4bit_1 = input_switch2_val;
    aux_ic_input_level4_register_4bit_2 = input_switch3_val;
    aux_ic_input_level4_register_4bit_3 = input_switch4_val;
    aux_ic_input_level4_register_4bit_4 = input_switch5_val;
    aux_ic_input_level4_register_4bit_5 = input_switch6_val;
    aux_level4_register_4bit_0_node_0 = aux_ic_input_level4_register_4bit_0;
    aux_level4_register_4bit_0_node_1 = aux_ic_input_level4_register_4bit_1;
    aux_level4_register_4bit_0_node_2 = aux_ic_input_level4_register_4bit_2;
    aux_level4_register_4bit_0_node_3 = aux_ic_input_level4_register_4bit_3;
    aux_level4_register_4bit_0_node_4 = aux_ic_input_level4_register_4bit_4;
    aux_level4_register_4bit_0_node_5 = aux_ic_input_level4_register_4bit_5;
    //D FlipFlop
    if (aux_level4_register_4bit_0_node_0 && !aux_level4_register_4bit_0_d_flip_flop_7_0_q_inclk) { 
        aux_level4_register_4bit_0_d_flip_flop_7_0_q = aux_level4_register_4bit_0_d_flip_flop_7_0_q_last;
        aux_level4_register_4bit_0_d_flip_flop_7_1_q = !aux_level4_register_4bit_0_d_flip_flop_7_0_q_last;
    }
    if (!HIGH || !HIGH) { 
        aux_level4_register_4bit_0_d_flip_flop_7_0_q = !HIGH; //Preset
        aux_level4_register_4bit_0_d_flip_flop_7_1_q = !HIGH; //Clear
    }
    aux_level4_register_4bit_0_d_flip_flop_7_0_q_inclk = aux_level4_register_4bit_0_node_0;
    //End of D FlipFlop
    aux_level4_register_4bit_0_d_flip_flop_7_0_q_last = aux_busmux_loadhold_6_out0;
    aux_level4_register_4bit_0_not_15 = !aux_level4_register_4bit_0_node_1;
    // IC: BusMux_LoadHold
    aux_ic_input_busmux_loadhold_0 = aux_level4_register_4bit_0_node_2;
    aux_ic_input_busmux_loadhold_1 = aux_level4_register_4bit_0_node_3;
    aux_ic_input_busmux_loadhold_2 = aux_level4_register_4bit_0_node_4;
    aux_ic_input_busmux_loadhold_3 = aux_level4_register_4bit_0_node_5;
    aux_ic_input_busmux_loadhold_4 = aux_level4_register_4bit_0_d_flip_flop_7_0_q;
    aux_ic_input_busmux_loadhold_5 = aux_level4_register_4bit_0_d_flip_flop_8_0_q;
    aux_ic_input_busmux_loadhold_6 = aux_level4_register_4bit_0_d_flip_flop_9_0_q;
    aux_ic_input_busmux_loadhold_7 = aux_level4_register_4bit_0_d_flip_flop_10_0_q;
    aux_ic_input_busmux_loadhold_8 = aux_level4_register_4bit_0_not_15;
    aux_busmux_loadhold_6_node_0 = aux_ic_input_busmux_loadhold_0;
    aux_busmux_loadhold_6_node_1 = aux_ic_input_busmux_loadhold_1;
    aux_busmux_loadhold_6_node_2 = aux_ic_input_busmux_loadhold_2;
    aux_busmux_loadhold_6_node_3 = aux_ic_input_busmux_loadhold_3;
    aux_busmux_loadhold_6_node_4 = aux_ic_input_busmux_loadhold_4;
    aux_busmux_loadhold_6_node_5 = aux_ic_input_busmux_loadhold_5;
    aux_busmux_loadhold_6_node_6 = aux_ic_input_busmux_loadhold_6;
    aux_busmux_loadhold_6_node_7 = aux_ic_input_busmux_loadhold_7;
    aux_busmux_loadhold_6_node_8 = aux_ic_input_busmux_loadhold_8;
    // IC: Mux[0]
    aux_ic_input_mux0_0 = aux_busmux_loadhold_6_node_0;
    aux_ic_input_mux0_1 = aux_busmux_loadhold_6_node_4;
    aux_ic_input_mux0_2 = aux_busmux_loadhold_6_node_8;
    aux_mux0_9_node_0 = aux_ic_input_mux0_0;
    aux_mux0_9_node_1 = aux_ic_input_mux0_1;
    aux_mux0_9_node_2 = aux_ic_input_mux0_2;
    //Multiplexer
    if ((aux_mux0_9_node_2) == 0) {
        aux_mux0_9_mux_3 = aux_mux0_9_node_0;
    } else if ((aux_mux0_9_node_2) == 1) {
        aux_mux0_9_mux_3 = aux_mux0_9_node_1;
    } else {
        aux_mux0_9_mux_3 = LOW;
    }
    //End of Multiplexer
    aux_mux0_9_node_4 = aux_mux0_9_mux_3;
    aux_mux0_9_output = aux_mux0_9_node_4;
    // End IC: Mux[0]
    // IC: Mux[1]
    aux_ic_input_mux1_0 = aux_busmux_loadhold_6_node_1;
    aux_ic_input_mux1_1 = aux_busmux_loadhold_6_node_5;
    aux_ic_input_mux1_2 = aux_busmux_loadhold_6_node_8;
    aux_mux1_10_node_0 = aux_ic_input_mux1_0;
    aux_mux1_10_node_1 = aux_ic_input_mux1_1;
    aux_mux1_10_node_2 = aux_ic_input_mux1_2;
    //Multiplexer
    if ((aux_mux1_10_node_2) == 0) {
        aux_mux1_10_mux_3 = aux_mux1_10_node_0;
    } else if ((aux_mux1_10_node_2) == 1) {
        aux_mux1_10_mux_3 = aux_mux1_10_node_1;
    } else {
        aux_mux1_10_mux_3 = LOW;
    }
    //End of Multiplexer
    aux_mux1_10_node_4 = aux_mux1_10_mux_3;
    aux_mux1_10_output = aux_mux1_10_node_4;
    // End IC: Mux[1]
    // IC: Mux[2]
    aux_ic_input_mux2_0 = aux_busmux_loadhold_6_node_2;
    aux_ic_input_mux2_1 = aux_busmux_loadhold_6_node_6;
    aux_ic_input_mux2_2 = aux_busmux_loadhold_6_node_8;
    aux_mux2_11_node_0 = aux_ic_input_mux2_0;
    aux_mux2_11_node_1 = aux_ic_input_mux2_1;
    aux_mux2_11_node_2 = aux_ic_input_mux2_2;
    //Multiplexer
    if ((aux_mux2_11_node_2) == 0) {
        aux_mux2_11_mux_3 = aux_mux2_11_node_0;
    } else if ((aux_mux2_11_node_2) == 1) {
        aux_mux2_11_mux_3 = aux_mux2_11_node_1;
    } else {
        aux_mux2_11_mux_3 = LOW;
    }
    //End of Multiplexer
    aux_mux2_11_node_4 = aux_mux2_11_mux_3;
    aux_mux2_11_output = aux_mux2_11_node_4;
    // End IC: Mux[2]
    // IC: Mux[3]
    aux_ic_input_mux3_0 = aux_busmux_loadhold_6_node_3;
    aux_ic_input_mux3_1 = aux_busmux_loadhold_6_node_7;
    aux_ic_input_mux3_2 = aux_busmux_loadhold_6_node_8;
    aux_mux3_12_node_0 = aux_ic_input_mux3_0;
    aux_mux3_12_node_1 = aux_ic_input_mux3_1;
    aux_mux3_12_node_2 = aux_ic_input_mux3_2;
    //Multiplexer
    if ((aux_mux3_12_node_2) == 0) {
        aux_mux3_12_mux_3 = aux_mux3_12_node_0;
    } else if ((aux_mux3_12_node_2) == 1) {
        aux_mux3_12_mux_3 = aux_mux3_12_node_1;
    } else {
        aux_mux3_12_mux_3 = LOW;
    }
    //End of Multiplexer
    aux_mux3_12_node_4 = aux_mux3_12_mux_3;
    aux_mux3_12_output = aux_mux3_12_node_4;
    // End IC: Mux[3]
    aux_busmux_loadhold_6_node_13 = aux_mux0_9_output;
    aux_busmux_loadhold_6_node_14 = aux_mux1_10_output;
    aux_busmux_loadhold_6_node_15 = aux_mux2_11_output;
    aux_busmux_loadhold_6_node_16 = aux_mux3_12_output;
    aux_busmux_loadhold_6_out0 = aux_busmux_loadhold_6_node_13;
    aux_busmux_loadhold_6_out1_1 = aux_busmux_loadhold_6_node_14;
    aux_busmux_loadhold_6_out2_2 = aux_busmux_loadhold_6_node_15;
    aux_busmux_loadhold_6_out3_3 = aux_busmux_loadhold_6_node_16;
    // End IC: BusMux_LoadHold
    //D FlipFlop
    if (aux_level4_register_4bit_0_node_0 && !aux_level4_register_4bit_0_d_flip_flop_8_0_q_inclk) { 
        aux_level4_register_4bit_0_d_flip_flop_8_0_q = aux_level4_register_4bit_0_d_flip_flop_8_0_q_last;
        aux_level4_register_4bit_0_d_flip_flop_8_1_q = !aux_level4_register_4bit_0_d_flip_flop_8_0_q_last;
    }
    if (!HIGH || !HIGH) { 
        aux_level4_register_4bit_0_d_flip_flop_8_0_q = !HIGH; //Preset
        aux_level4_register_4bit_0_d_flip_flop_8_1_q = !HIGH; //Clear
    }
    aux_level4_register_4bit_0_d_flip_flop_8_0_q_inclk = aux_level4_register_4bit_0_node_0;
    //End of D FlipFlop
    aux_level4_register_4bit_0_d_flip_flop_8_0_q_last = aux_busmux_loadhold_6_out1_1;
    //D FlipFlop
    if (aux_level4_register_4bit_0_node_0 && !aux_level4_register_4bit_0_d_flip_flop_9_0_q_inclk) { 
        aux_level4_register_4bit_0_d_flip_flop_9_0_q = aux_level4_register_4bit_0_d_flip_flop_9_0_q_last;
        aux_level4_register_4bit_0_d_flip_flop_9_1_q = !aux_level4_register_4bit_0_d_flip_flop_9_0_q_last;
    }
    if (!HIGH || !HIGH) { 
        aux_level4_register_4bit_0_d_flip_flop_9_0_q = !HIGH; //Preset
        aux_level4_register_4bit_0_d_flip_flop_9_1_q = !HIGH; //Clear
    }
    aux_level4_register_4bit_0_d_flip_flop_9_0_q_inclk = aux_level4_register_4bit_0_node_0;
    //End of D FlipFlop
    aux_level4_register_4bit_0_d_flip_flop_9_0_q_last = aux_busmux_loadhold_6_out2_2;
    //D FlipFlop
    if (aux_level4_register_4bit_0_node_0 && !aux_level4_register_4bit_0_d_flip_flop_10_0_q_inclk) { 
        aux_level4_register_4bit_0_d_flip_flop_10_0_q = aux_level4_register_4bit_0_d_flip_flop_10_0_q_last;
        aux_level4_register_4bit_0_d_flip_flop_10_1_q = !aux_level4_register_4bit_0_d_flip_flop_10_0_q_last;
    }
    if (!HIGH || !HIGH) { 
        aux_level4_register_4bit_0_d_flip_flop_10_0_q = !HIGH; //Preset
        aux_level4_register_4bit_0_d_flip_flop_10_1_q = !HIGH; //Clear
    }
    aux_level4_register_4bit_0_d_flip_flop_10_0_q_inclk = aux_level4_register_4bit_0_node_0;
    //End of D FlipFlop
    aux_level4_register_4bit_0_d_flip_flop_10_0_q_last = aux_busmux_loadhold_6_out3_3;
    aux_level4_register_4bit_0_node_11 = aux_level4_register_4bit_0_d_flip_flop_7_0_q;
    aux_level4_register_4bit_0_node_12 = aux_level4_register_4bit_0_d_flip_flop_8_0_q;
    aux_level4_register_4bit_0_node_13 = aux_level4_register_4bit_0_d_flip_flop_9_0_q;
    aux_level4_register_4bit_0_node_14 = aux_level4_register_4bit_0_d_flip_flop_10_0_q;
    aux_level4_register_4bit_0_q0 = aux_level4_register_4bit_0_node_11;
    aux_level4_register_4bit_0_q1_1 = aux_level4_register_4bit_0_node_12;
    aux_level4_register_4bit_0_q2_2 = aux_level4_register_4bit_0_node_13;
    aux_level4_register_4bit_0_q3_3 = aux_level4_register_4bit_0_node_14;
    // End IC: LEVEL4_REGISTER_4BIT
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
    digitalWrite(led1_0, aux_level4_register_4bit_0_q0);
    digitalWrite(led2_0, aux_level4_register_4bit_0_q1_1);
    digitalWrite(led3_0, aux_level4_register_4bit_0_q2_2);
    digitalWrite(led4_0, aux_level4_register_4bit_0_q3_3);
}
