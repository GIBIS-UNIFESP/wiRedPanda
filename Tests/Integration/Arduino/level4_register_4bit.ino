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

/* ========= Outputs ========== */
const int led1_1 = 3;
const int led2_1 = 4;
const int led3_1 = 5;
const int led4_1 = 6;

/* ====== Aux. Variables ====== */
bool input_switch1_val = LOW;
bool input_switch2_val = LOW;
bool input_switch3_val = LOW;
bool input_switch4_val = LOW;
bool input_switch5_val = LOW;
bool input_switch6_val = LOW;
bool input_switch7_val = LOW;
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
bool aux_level4_register_4bit_0_node_6 = LOW;
// IC: BusMux_LoadHold
bool aux_level4_register_4bit_0_busmux_loadhold_7_out0 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_out1_1 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_out2_2 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_out3_3 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_node_0 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_node_1 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_node_2 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_node_3 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_node_4 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_node_5 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_node_6 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_node_7 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_node_8 = LOW;
// IC: Mux[0]
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux0_9_output = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux0_9_node_0 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux0_9_node_1 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux0_9_node_2 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux0_9_mux_3 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux0_9_node_4 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux0_9_and_5 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux0_9_node_6 = LOW;
bool aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux0_9_0 = LOW;
bool aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux0_9_1 = LOW;
bool aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux0_9_2 = LOW;
bool aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux0_9_3 = LOW;
// End IC: Mux[0]
// IC: Mux[1]
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux1_10_output = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux1_10_node_0 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux1_10_node_1 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux1_10_node_2 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux1_10_mux_3 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux1_10_node_4 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux1_10_and_5 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux1_10_node_6 = LOW;
bool aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux1_10_0 = LOW;
bool aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux1_10_1 = LOW;
bool aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux1_10_2 = LOW;
bool aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux1_10_3 = LOW;
// End IC: Mux[1]
// IC: Mux[2]
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux2_11_output = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux2_11_node_0 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux2_11_node_1 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux2_11_node_2 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux2_11_mux_3 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux2_11_node_4 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux2_11_and_5 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux2_11_node_6 = LOW;
bool aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux2_11_0 = LOW;
bool aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux2_11_1 = LOW;
bool aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux2_11_2 = LOW;
bool aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux2_11_3 = LOW;
// End IC: Mux[2]
// IC: Mux[3]
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux3_12_output = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux3_12_node_0 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux3_12_node_1 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux3_12_node_2 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux3_12_mux_3 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux3_12_node_4 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux3_12_and_5 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_mux3_12_node_6 = LOW;
bool aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux3_12_0 = LOW;
bool aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux3_12_1 = LOW;
bool aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux3_12_2 = LOW;
bool aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux3_12_3 = LOW;
// End IC: Mux[3]
bool aux_level4_register_4bit_0_busmux_loadhold_7_node_14 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_node_15 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_node_16 = LOW;
bool aux_level4_register_4bit_0_busmux_loadhold_7_node_17 = LOW;
bool aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_0 = LOW;
bool aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_1 = LOW;
bool aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_2 = LOW;
bool aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_3 = LOW;
bool aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_4 = LOW;
bool aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_5 = LOW;
bool aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_6 = LOW;
bool aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_7 = LOW;
bool aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_8 = LOW;
// End IC: BusMux_LoadHold
bool aux_level4_register_4bit_0_d_flip_flop_8_0_q = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_8_0_q_next = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_8_0_q_inclk = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_8_0_q_last = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_8_1_q = HIGH;
bool aux_level4_register_4bit_0_d_flip_flop_8_1_q_next = HIGH;
bool aux_level4_register_4bit_0_d_flip_flop_8_1_q_inclk = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_8_1_q_last = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_9_0_q = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_9_0_q_next = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_9_0_q_inclk = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_9_0_q_last = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_9_1_q = HIGH;
bool aux_level4_register_4bit_0_d_flip_flop_9_1_q_next = HIGH;
bool aux_level4_register_4bit_0_d_flip_flop_9_1_q_inclk = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_9_1_q_last = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_10_0_q = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_10_0_q_next = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_10_0_q_inclk = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_10_0_q_last = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_10_1_q = HIGH;
bool aux_level4_register_4bit_0_d_flip_flop_10_1_q_next = HIGH;
bool aux_level4_register_4bit_0_d_flip_flop_10_1_q_inclk = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_10_1_q_last = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_11_0_q = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_11_0_q_next = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_11_0_q_inclk = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_11_0_q_last = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_11_1_q = HIGH;
bool aux_level4_register_4bit_0_d_flip_flop_11_1_q_next = HIGH;
bool aux_level4_register_4bit_0_d_flip_flop_11_1_q_inclk = LOW;
bool aux_level4_register_4bit_0_d_flip_flop_11_1_q_last = LOW;
bool aux_level4_register_4bit_0_node_12 = LOW;
bool aux_level4_register_4bit_0_node_13 = LOW;
bool aux_level4_register_4bit_0_node_14 = LOW;
bool aux_level4_register_4bit_0_node_15 = LOW;
bool aux_level4_register_4bit_0_not_16 = LOW;
bool aux_level4_register_4bit_0_not_17 = LOW;
bool aux_ic_input_level4_register_4bit_0_0 = LOW;
bool aux_ic_input_level4_register_4bit_0_1 = LOW;
bool aux_ic_input_level4_register_4bit_0_2 = LOW;
bool aux_ic_input_level4_register_4bit_0_3 = LOW;
bool aux_ic_input_level4_register_4bit_0_4 = LOW;
bool aux_ic_input_level4_register_4bit_0_5 = LOW;
bool aux_ic_input_level4_register_4bit_0_6 = LOW;
// End IC: LEVEL4_REGISTER_4BIT

bool g_sample = true;

void setup() {
    pinMode(input_switch1, INPUT);
    pinMode(input_switch2, INPUT);
    pinMode(input_switch3, INPUT);
    pinMode(input_switch4, INPUT);
    pinMode(input_switch5, INPUT);
    pinMode(input_switch6, INPUT);
    pinMode(input_switch7, INPUT);
    pinMode(led1_1, OUTPUT);
    pinMode(led2_1, OUTPUT);
    pinMode(led3_1, OUTPUT);
    pinMode(led4_1, OUTPUT);
}

void computeLogic() {
    // Assigning aux variables. //
    // IC: LEVEL4_REGISTER_4BIT
    aux_ic_input_level4_register_4bit_0_0 = input_switch1_val;
    aux_ic_input_level4_register_4bit_0_1 = input_switch2_val;
    aux_ic_input_level4_register_4bit_0_2 = input_switch3_val;
    aux_ic_input_level4_register_4bit_0_3 = input_switch4_val;
    aux_ic_input_level4_register_4bit_0_4 = input_switch5_val;
    aux_ic_input_level4_register_4bit_0_5 = input_switch6_val;
    aux_ic_input_level4_register_4bit_0_6 = input_switch7_val;
    aux_level4_register_4bit_0_node_2 = aux_ic_input_level4_register_4bit_0_2;
    aux_level4_register_4bit_0_node_0 = aux_ic_input_level4_register_4bit_0_0;
    aux_level4_register_4bit_0_node_1 = aux_ic_input_level4_register_4bit_0_1;
    aux_level4_register_4bit_0_node_3 = aux_ic_input_level4_register_4bit_0_3;
    aux_level4_register_4bit_0_node_4 = aux_ic_input_level4_register_4bit_0_4;
    aux_level4_register_4bit_0_node_5 = aux_ic_input_level4_register_4bit_0_5;
    aux_level4_register_4bit_0_node_6 = aux_ic_input_level4_register_4bit_0_6;
    aux_level4_register_4bit_0_not_16 = !aux_level4_register_4bit_0_node_2;
    //D FlipFlop
    if (g_sample) {
    if (aux_level4_register_4bit_0_node_0 && !aux_level4_register_4bit_0_d_flip_flop_8_0_q_inclk) { 
        aux_level4_register_4bit_0_d_flip_flop_8_0_q_next = aux_level4_register_4bit_0_d_flip_flop_8_0_q_last;
        aux_level4_register_4bit_0_d_flip_flop_8_1_q_next = !aux_level4_register_4bit_0_d_flip_flop_8_0_q_last;
    }
    if (!HIGH || !aux_level4_register_4bit_0_not_16) { 
        aux_level4_register_4bit_0_d_flip_flop_8_0_q_next = !HIGH; //Preset
        aux_level4_register_4bit_0_d_flip_flop_8_1_q_next = !aux_level4_register_4bit_0_not_16; //Clear
    }
    aux_level4_register_4bit_0_d_flip_flop_8_0_q_inclk = aux_level4_register_4bit_0_node_0;
        aux_level4_register_4bit_0_d_flip_flop_8_0_q_last = aux_level4_register_4bit_0_busmux_loadhold_7_out0;
    }
    //End of D FlipFlop
    //D FlipFlop
    if (g_sample) {
    if (aux_level4_register_4bit_0_node_0 && !aux_level4_register_4bit_0_d_flip_flop_9_0_q_inclk) { 
        aux_level4_register_4bit_0_d_flip_flop_9_0_q_next = aux_level4_register_4bit_0_d_flip_flop_9_0_q_last;
        aux_level4_register_4bit_0_d_flip_flop_9_1_q_next = !aux_level4_register_4bit_0_d_flip_flop_9_0_q_last;
    }
    if (!HIGH || !aux_level4_register_4bit_0_not_16) { 
        aux_level4_register_4bit_0_d_flip_flop_9_0_q_next = !HIGH; //Preset
        aux_level4_register_4bit_0_d_flip_flop_9_1_q_next = !aux_level4_register_4bit_0_not_16; //Clear
    }
    aux_level4_register_4bit_0_d_flip_flop_9_0_q_inclk = aux_level4_register_4bit_0_node_0;
        aux_level4_register_4bit_0_d_flip_flop_9_0_q_last = aux_level4_register_4bit_0_busmux_loadhold_7_out1_1;
    }
    //End of D FlipFlop
    //D FlipFlop
    if (g_sample) {
    if (aux_level4_register_4bit_0_node_0 && !aux_level4_register_4bit_0_d_flip_flop_10_0_q_inclk) { 
        aux_level4_register_4bit_0_d_flip_flop_10_0_q_next = aux_level4_register_4bit_0_d_flip_flop_10_0_q_last;
        aux_level4_register_4bit_0_d_flip_flop_10_1_q_next = !aux_level4_register_4bit_0_d_flip_flop_10_0_q_last;
    }
    if (!HIGH || !aux_level4_register_4bit_0_not_16) { 
        aux_level4_register_4bit_0_d_flip_flop_10_0_q_next = !HIGH; //Preset
        aux_level4_register_4bit_0_d_flip_flop_10_1_q_next = !aux_level4_register_4bit_0_not_16; //Clear
    }
    aux_level4_register_4bit_0_d_flip_flop_10_0_q_inclk = aux_level4_register_4bit_0_node_0;
        aux_level4_register_4bit_0_d_flip_flop_10_0_q_last = aux_level4_register_4bit_0_busmux_loadhold_7_out2_2;
    }
    //End of D FlipFlop
    //D FlipFlop
    if (g_sample) {
    if (aux_level4_register_4bit_0_node_0 && !aux_level4_register_4bit_0_d_flip_flop_11_0_q_inclk) { 
        aux_level4_register_4bit_0_d_flip_flop_11_0_q_next = aux_level4_register_4bit_0_d_flip_flop_11_0_q_last;
        aux_level4_register_4bit_0_d_flip_flop_11_1_q_next = !aux_level4_register_4bit_0_d_flip_flop_11_0_q_last;
    }
    if (!HIGH || !aux_level4_register_4bit_0_not_16) { 
        aux_level4_register_4bit_0_d_flip_flop_11_0_q_next = !HIGH; //Preset
        aux_level4_register_4bit_0_d_flip_flop_11_1_q_next = !aux_level4_register_4bit_0_not_16; //Clear
    }
    aux_level4_register_4bit_0_d_flip_flop_11_0_q_inclk = aux_level4_register_4bit_0_node_0;
        aux_level4_register_4bit_0_d_flip_flop_11_0_q_last = aux_level4_register_4bit_0_busmux_loadhold_7_out3_3;
    }
    //End of D FlipFlop
    aux_level4_register_4bit_0_not_17 = !aux_level4_register_4bit_0_node_1;
    // IC: BusMux_LoadHold
    aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_0 = aux_level4_register_4bit_0_node_3;
    aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_1 = aux_level4_register_4bit_0_node_4;
    aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_2 = aux_level4_register_4bit_0_node_5;
    aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_3 = aux_level4_register_4bit_0_node_6;
    aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_4 = aux_level4_register_4bit_0_d_flip_flop_8_0_q;
    aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_5 = aux_level4_register_4bit_0_d_flip_flop_9_0_q;
    aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_6 = aux_level4_register_4bit_0_d_flip_flop_10_0_q;
    aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_7 = aux_level4_register_4bit_0_d_flip_flop_11_0_q;
    aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_8 = aux_level4_register_4bit_0_not_17;
    aux_level4_register_4bit_0_busmux_loadhold_7_node_0 = aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_0;
    aux_level4_register_4bit_0_busmux_loadhold_7_node_1 = aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_1;
    aux_level4_register_4bit_0_busmux_loadhold_7_node_2 = aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_2;
    aux_level4_register_4bit_0_busmux_loadhold_7_node_3 = aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_3;
    aux_level4_register_4bit_0_busmux_loadhold_7_node_4 = aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_4;
    aux_level4_register_4bit_0_busmux_loadhold_7_node_5 = aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_5;
    aux_level4_register_4bit_0_busmux_loadhold_7_node_6 = aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_6;
    aux_level4_register_4bit_0_busmux_loadhold_7_node_7 = aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_7;
    aux_level4_register_4bit_0_busmux_loadhold_7_node_8 = aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_8;
    // IC: Mux[0]
    aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux0_9_0 = aux_level4_register_4bit_0_busmux_loadhold_7_node_0;
    aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux0_9_1 = aux_level4_register_4bit_0_busmux_loadhold_7_node_4;
    aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux0_9_2 = aux_level4_register_4bit_0_busmux_loadhold_7_node_8;
    aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux0_9_3 = HIGH;
    aux_level4_register_4bit_0_busmux_loadhold_7_mux0_9_node_0 = aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux0_9_0;
    aux_level4_register_4bit_0_busmux_loadhold_7_mux0_9_node_1 = aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux0_9_1;
    aux_level4_register_4bit_0_busmux_loadhold_7_mux0_9_node_2 = aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux0_9_2;
    aux_level4_register_4bit_0_busmux_loadhold_7_mux0_9_node_4 = aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux0_9_3;
    //Multiplexer
    if ((aux_level4_register_4bit_0_busmux_loadhold_7_mux0_9_node_2) == 0) {
        aux_level4_register_4bit_0_busmux_loadhold_7_mux0_9_mux_3 = aux_level4_register_4bit_0_busmux_loadhold_7_mux0_9_node_0;
    } else if ((aux_level4_register_4bit_0_busmux_loadhold_7_mux0_9_node_2) == 1) {
        aux_level4_register_4bit_0_busmux_loadhold_7_mux0_9_mux_3 = aux_level4_register_4bit_0_busmux_loadhold_7_mux0_9_node_1;
    } else {
        aux_level4_register_4bit_0_busmux_loadhold_7_mux0_9_mux_3 = LOW;
    }
    //End of Multiplexer
    aux_level4_register_4bit_0_busmux_loadhold_7_mux0_9_and_5 = aux_level4_register_4bit_0_busmux_loadhold_7_mux0_9_mux_3 && aux_level4_register_4bit_0_busmux_loadhold_7_mux0_9_node_4;
    aux_level4_register_4bit_0_busmux_loadhold_7_mux0_9_node_6 = aux_level4_register_4bit_0_busmux_loadhold_7_mux0_9_and_5;
    aux_level4_register_4bit_0_busmux_loadhold_7_mux0_9_output = aux_level4_register_4bit_0_busmux_loadhold_7_mux0_9_node_6;
    // End IC: Mux[0]
    // IC: Mux[1]
    aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux1_10_0 = aux_level4_register_4bit_0_busmux_loadhold_7_node_1;
    aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux1_10_1 = aux_level4_register_4bit_0_busmux_loadhold_7_node_5;
    aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux1_10_2 = aux_level4_register_4bit_0_busmux_loadhold_7_node_8;
    aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux1_10_3 = HIGH;
    aux_level4_register_4bit_0_busmux_loadhold_7_mux1_10_node_0 = aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux1_10_0;
    aux_level4_register_4bit_0_busmux_loadhold_7_mux1_10_node_1 = aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux1_10_1;
    aux_level4_register_4bit_0_busmux_loadhold_7_mux1_10_node_2 = aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux1_10_2;
    aux_level4_register_4bit_0_busmux_loadhold_7_mux1_10_node_4 = aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux1_10_3;
    //Multiplexer
    if ((aux_level4_register_4bit_0_busmux_loadhold_7_mux1_10_node_2) == 0) {
        aux_level4_register_4bit_0_busmux_loadhold_7_mux1_10_mux_3 = aux_level4_register_4bit_0_busmux_loadhold_7_mux1_10_node_0;
    } else if ((aux_level4_register_4bit_0_busmux_loadhold_7_mux1_10_node_2) == 1) {
        aux_level4_register_4bit_0_busmux_loadhold_7_mux1_10_mux_3 = aux_level4_register_4bit_0_busmux_loadhold_7_mux1_10_node_1;
    } else {
        aux_level4_register_4bit_0_busmux_loadhold_7_mux1_10_mux_3 = LOW;
    }
    //End of Multiplexer
    aux_level4_register_4bit_0_busmux_loadhold_7_mux1_10_and_5 = aux_level4_register_4bit_0_busmux_loadhold_7_mux1_10_mux_3 && aux_level4_register_4bit_0_busmux_loadhold_7_mux1_10_node_4;
    aux_level4_register_4bit_0_busmux_loadhold_7_mux1_10_node_6 = aux_level4_register_4bit_0_busmux_loadhold_7_mux1_10_and_5;
    aux_level4_register_4bit_0_busmux_loadhold_7_mux1_10_output = aux_level4_register_4bit_0_busmux_loadhold_7_mux1_10_node_6;
    // End IC: Mux[1]
    // IC: Mux[2]
    aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux2_11_0 = aux_level4_register_4bit_0_busmux_loadhold_7_node_2;
    aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux2_11_1 = aux_level4_register_4bit_0_busmux_loadhold_7_node_6;
    aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux2_11_2 = aux_level4_register_4bit_0_busmux_loadhold_7_node_8;
    aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux2_11_3 = HIGH;
    aux_level4_register_4bit_0_busmux_loadhold_7_mux2_11_node_0 = aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux2_11_0;
    aux_level4_register_4bit_0_busmux_loadhold_7_mux2_11_node_1 = aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux2_11_1;
    aux_level4_register_4bit_0_busmux_loadhold_7_mux2_11_node_2 = aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux2_11_2;
    aux_level4_register_4bit_0_busmux_loadhold_7_mux2_11_node_4 = aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux2_11_3;
    //Multiplexer
    if ((aux_level4_register_4bit_0_busmux_loadhold_7_mux2_11_node_2) == 0) {
        aux_level4_register_4bit_0_busmux_loadhold_7_mux2_11_mux_3 = aux_level4_register_4bit_0_busmux_loadhold_7_mux2_11_node_0;
    } else if ((aux_level4_register_4bit_0_busmux_loadhold_7_mux2_11_node_2) == 1) {
        aux_level4_register_4bit_0_busmux_loadhold_7_mux2_11_mux_3 = aux_level4_register_4bit_0_busmux_loadhold_7_mux2_11_node_1;
    } else {
        aux_level4_register_4bit_0_busmux_loadhold_7_mux2_11_mux_3 = LOW;
    }
    //End of Multiplexer
    aux_level4_register_4bit_0_busmux_loadhold_7_mux2_11_and_5 = aux_level4_register_4bit_0_busmux_loadhold_7_mux2_11_mux_3 && aux_level4_register_4bit_0_busmux_loadhold_7_mux2_11_node_4;
    aux_level4_register_4bit_0_busmux_loadhold_7_mux2_11_node_6 = aux_level4_register_4bit_0_busmux_loadhold_7_mux2_11_and_5;
    aux_level4_register_4bit_0_busmux_loadhold_7_mux2_11_output = aux_level4_register_4bit_0_busmux_loadhold_7_mux2_11_node_6;
    // End IC: Mux[2]
    // IC: Mux[3]
    aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux3_12_0 = aux_level4_register_4bit_0_busmux_loadhold_7_node_3;
    aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux3_12_1 = aux_level4_register_4bit_0_busmux_loadhold_7_node_7;
    aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux3_12_2 = aux_level4_register_4bit_0_busmux_loadhold_7_node_8;
    aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux3_12_3 = HIGH;
    aux_level4_register_4bit_0_busmux_loadhold_7_mux3_12_node_0 = aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux3_12_0;
    aux_level4_register_4bit_0_busmux_loadhold_7_mux3_12_node_1 = aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux3_12_1;
    aux_level4_register_4bit_0_busmux_loadhold_7_mux3_12_node_2 = aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux3_12_2;
    aux_level4_register_4bit_0_busmux_loadhold_7_mux3_12_node_4 = aux_ic_input_level4_register_4bit_0_busmux_loadhold_7_mux3_12_3;
    //Multiplexer
    if ((aux_level4_register_4bit_0_busmux_loadhold_7_mux3_12_node_2) == 0) {
        aux_level4_register_4bit_0_busmux_loadhold_7_mux3_12_mux_3 = aux_level4_register_4bit_0_busmux_loadhold_7_mux3_12_node_0;
    } else if ((aux_level4_register_4bit_0_busmux_loadhold_7_mux3_12_node_2) == 1) {
        aux_level4_register_4bit_0_busmux_loadhold_7_mux3_12_mux_3 = aux_level4_register_4bit_0_busmux_loadhold_7_mux3_12_node_1;
    } else {
        aux_level4_register_4bit_0_busmux_loadhold_7_mux3_12_mux_3 = LOW;
    }
    //End of Multiplexer
    aux_level4_register_4bit_0_busmux_loadhold_7_mux3_12_and_5 = aux_level4_register_4bit_0_busmux_loadhold_7_mux3_12_mux_3 && aux_level4_register_4bit_0_busmux_loadhold_7_mux3_12_node_4;
    aux_level4_register_4bit_0_busmux_loadhold_7_mux3_12_node_6 = aux_level4_register_4bit_0_busmux_loadhold_7_mux3_12_and_5;
    aux_level4_register_4bit_0_busmux_loadhold_7_mux3_12_output = aux_level4_register_4bit_0_busmux_loadhold_7_mux3_12_node_6;
    // End IC: Mux[3]
    aux_level4_register_4bit_0_busmux_loadhold_7_node_14 = aux_level4_register_4bit_0_busmux_loadhold_7_mux0_9_output;
    aux_level4_register_4bit_0_busmux_loadhold_7_node_15 = aux_level4_register_4bit_0_busmux_loadhold_7_mux1_10_output;
    aux_level4_register_4bit_0_busmux_loadhold_7_node_16 = aux_level4_register_4bit_0_busmux_loadhold_7_mux2_11_output;
    aux_level4_register_4bit_0_busmux_loadhold_7_node_17 = aux_level4_register_4bit_0_busmux_loadhold_7_mux3_12_output;
    aux_level4_register_4bit_0_busmux_loadhold_7_out0 = aux_level4_register_4bit_0_busmux_loadhold_7_node_14;
    aux_level4_register_4bit_0_busmux_loadhold_7_out1_1 = aux_level4_register_4bit_0_busmux_loadhold_7_node_15;
    aux_level4_register_4bit_0_busmux_loadhold_7_out2_2 = aux_level4_register_4bit_0_busmux_loadhold_7_node_16;
    aux_level4_register_4bit_0_busmux_loadhold_7_out3_3 = aux_level4_register_4bit_0_busmux_loadhold_7_node_17;
    // End IC: BusMux_LoadHold
    aux_level4_register_4bit_0_node_12 = aux_level4_register_4bit_0_d_flip_flop_8_0_q;
    aux_level4_register_4bit_0_node_13 = aux_level4_register_4bit_0_d_flip_flop_9_0_q;
    aux_level4_register_4bit_0_node_14 = aux_level4_register_4bit_0_d_flip_flop_10_0_q;
    aux_level4_register_4bit_0_node_15 = aux_level4_register_4bit_0_d_flip_flop_11_0_q;
    aux_level4_register_4bit_0_q0 = aux_level4_register_4bit_0_node_12;
    aux_level4_register_4bit_0_q1_1 = aux_level4_register_4bit_0_node_13;
    aux_level4_register_4bit_0_q2_2 = aux_level4_register_4bit_0_node_14;
    aux_level4_register_4bit_0_q3_3 = aux_level4_register_4bit_0_node_15;
    // End IC: LEVEL4_REGISTER_4BIT
}

void commitFlipFlops() {
    aux_level4_register_4bit_0_d_flip_flop_8_0_q = aux_level4_register_4bit_0_d_flip_flop_8_0_q_next;
    aux_level4_register_4bit_0_d_flip_flop_8_1_q = aux_level4_register_4bit_0_d_flip_flop_8_1_q_next;
    aux_level4_register_4bit_0_d_flip_flop_9_0_q = aux_level4_register_4bit_0_d_flip_flop_9_0_q_next;
    aux_level4_register_4bit_0_d_flip_flop_9_1_q = aux_level4_register_4bit_0_d_flip_flop_9_1_q_next;
    aux_level4_register_4bit_0_d_flip_flop_10_0_q = aux_level4_register_4bit_0_d_flip_flop_10_0_q_next;
    aux_level4_register_4bit_0_d_flip_flop_10_1_q = aux_level4_register_4bit_0_d_flip_flop_10_1_q_next;
    aux_level4_register_4bit_0_d_flip_flop_11_0_q = aux_level4_register_4bit_0_d_flip_flop_11_0_q_next;
    aux_level4_register_4bit_0_d_flip_flop_11_1_q = aux_level4_register_4bit_0_d_flip_flop_11_1_q_next;
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

    // Updating clocks. //

    g_sample = true;
    for (int s = 0; s < 10; s++) { computeLogic(); }
    commitFlipFlops();
    g_sample = false;
    for (int s = 0; s < 10; s++) { computeLogic(); }

    // Writing output data. //
    digitalWrite(led1_1, aux_level4_register_4bit_0_q0);
    digitalWrite(led2_1, aux_level4_register_4bit_0_q1_1);
    digitalWrite(led3_1, aux_level4_register_4bit_0_q2_2);
    digitalWrite(led4_1, aux_level4_register_4bit_0_q3_3);
}
