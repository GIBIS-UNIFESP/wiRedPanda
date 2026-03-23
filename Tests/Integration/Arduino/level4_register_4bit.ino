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
const int led1_1 = 2;
const int led2_1 = 3;
const int led3_1 = 4;
const int led4_1 = 5;

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
bool aux_ic_input_mux0_9_0 = LOW;
bool aux_ic_input_mux0_9_1 = LOW;
bool aux_ic_input_mux0_9_2 = LOW;
// End IC: Mux[0]
// IC: Mux[1]
bool aux_mux1_10_output = LOW;
bool aux_mux1_10_node_0 = LOW;
bool aux_mux1_10_node_1 = LOW;
bool aux_mux1_10_node_2 = LOW;
bool aux_mux1_10_mux_3 = LOW;
bool aux_mux1_10_node_4 = LOW;
bool aux_ic_input_mux1_10_0 = LOW;
bool aux_ic_input_mux1_10_1 = LOW;
bool aux_ic_input_mux1_10_2 = LOW;
// End IC: Mux[1]
// IC: Mux[2]
bool aux_mux2_11_output = LOW;
bool aux_mux2_11_node_0 = LOW;
bool aux_mux2_11_node_1 = LOW;
bool aux_mux2_11_node_2 = LOW;
bool aux_mux2_11_mux_3 = LOW;
bool aux_mux2_11_node_4 = LOW;
bool aux_ic_input_mux2_11_0 = LOW;
bool aux_ic_input_mux2_11_1 = LOW;
bool aux_ic_input_mux2_11_2 = LOW;
// End IC: Mux[2]
// IC: Mux[3]
bool aux_mux3_12_output = LOW;
bool aux_mux3_12_node_0 = LOW;
bool aux_mux3_12_node_1 = LOW;
bool aux_mux3_12_node_2 = LOW;
bool aux_mux3_12_mux_3 = LOW;
bool aux_mux3_12_node_4 = LOW;
bool aux_ic_input_mux3_12_0 = LOW;
bool aux_ic_input_mux3_12_1 = LOW;
bool aux_ic_input_mux3_12_2 = LOW;
// End IC: Mux[3]
bool aux_busmux_loadhold_6_node_13 = LOW;
bool aux_busmux_loadhold_6_node_14 = LOW;
bool aux_busmux_loadhold_6_node_15 = LOW;
bool aux_busmux_loadhold_6_node_16 = LOW;
bool aux_ic_input_busmux_loadhold_6_0 = LOW;
bool aux_ic_input_busmux_loadhold_6_1 = LOW;
bool aux_ic_input_busmux_loadhold_6_2 = LOW;
bool aux_ic_input_busmux_loadhold_6_3 = LOW;
bool aux_ic_input_busmux_loadhold_6_4 = LOW;
bool aux_ic_input_busmux_loadhold_6_5 = LOW;
bool aux_ic_input_busmux_loadhold_6_6 = LOW;
bool aux_ic_input_busmux_loadhold_6_7 = LOW;
bool aux_ic_input_busmux_loadhold_6_8 = LOW;
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
bool aux_ic_input_level4_register_4bit_0_0 = LOW;
bool aux_ic_input_level4_register_4bit_0_1 = LOW;
bool aux_ic_input_level4_register_4bit_0_2 = LOW;
bool aux_ic_input_level4_register_4bit_0_3 = LOW;
bool aux_ic_input_level4_register_4bit_0_4 = LOW;
bool aux_ic_input_level4_register_4bit_0_5 = LOW;
// End IC: LEVEL4_REGISTER_4BIT

void setup() {
    pinMode(input_switch1, INPUT);
    pinMode(input_switch2, INPUT);
    pinMode(input_switch3, INPUT);
    pinMode(input_switch4, INPUT);
    pinMode(input_switch5, INPUT);
    pinMode(input_switch6, INPUT);
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
    aux_level4_register_4bit_0_node_0 = aux_ic_input_level4_register_4bit_0_0;
    aux_level4_register_4bit_0_node_1 = aux_ic_input_level4_register_4bit_0_1;
    aux_level4_register_4bit_0_node_2 = aux_ic_input_level4_register_4bit_0_2;
    aux_level4_register_4bit_0_node_3 = aux_ic_input_level4_register_4bit_0_3;
    aux_level4_register_4bit_0_node_4 = aux_ic_input_level4_register_4bit_0_4;
    aux_level4_register_4bit_0_node_5 = aux_ic_input_level4_register_4bit_0_5;
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
    aux_level4_register_4bit_0_not_15 = !aux_level4_register_4bit_0_node_1;
    // IC: BusMux_LoadHold
    aux_ic_input_busmux_loadhold_6_0 = aux_level4_register_4bit_0_node_2;
    aux_ic_input_busmux_loadhold_6_1 = aux_level4_register_4bit_0_node_3;
    aux_ic_input_busmux_loadhold_6_2 = aux_level4_register_4bit_0_node_4;
    aux_ic_input_busmux_loadhold_6_3 = aux_level4_register_4bit_0_node_5;
    aux_ic_input_busmux_loadhold_6_4 = aux_level4_register_4bit_0_d_flip_flop_7_0_q;
    aux_ic_input_busmux_loadhold_6_5 = aux_level4_register_4bit_0_d_flip_flop_8_0_q;
    aux_ic_input_busmux_loadhold_6_6 = aux_level4_register_4bit_0_d_flip_flop_9_0_q;
    aux_ic_input_busmux_loadhold_6_7 = aux_level4_register_4bit_0_d_flip_flop_10_0_q;
    aux_ic_input_busmux_loadhold_6_8 = aux_level4_register_4bit_0_not_15;
    aux_busmux_loadhold_6_node_0 = aux_ic_input_busmux_loadhold_6_0;
    aux_busmux_loadhold_6_node_1 = aux_ic_input_busmux_loadhold_6_1;
    aux_busmux_loadhold_6_node_2 = aux_ic_input_busmux_loadhold_6_2;
    aux_busmux_loadhold_6_node_3 = aux_ic_input_busmux_loadhold_6_3;
    aux_busmux_loadhold_6_node_4 = aux_ic_input_busmux_loadhold_6_4;
    aux_busmux_loadhold_6_node_5 = aux_ic_input_busmux_loadhold_6_5;
    aux_busmux_loadhold_6_node_6 = aux_ic_input_busmux_loadhold_6_6;
    aux_busmux_loadhold_6_node_7 = aux_ic_input_busmux_loadhold_6_7;
    aux_busmux_loadhold_6_node_8 = aux_ic_input_busmux_loadhold_6_8;
    // IC: Mux[0]
    aux_ic_input_mux0_9_0 = aux_busmux_loadhold_6_node_0;
    aux_ic_input_mux0_9_1 = aux_busmux_loadhold_6_node_4;
    aux_ic_input_mux0_9_2 = aux_busmux_loadhold_6_node_8;
    aux_mux0_9_node_0 = aux_ic_input_mux0_9_0;
    aux_mux0_9_node_1 = aux_ic_input_mux0_9_1;
    aux_mux0_9_node_2 = aux_ic_input_mux0_9_2;
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
    aux_ic_input_mux1_10_0 = aux_busmux_loadhold_6_node_1;
    aux_ic_input_mux1_10_1 = aux_busmux_loadhold_6_node_5;
    aux_ic_input_mux1_10_2 = aux_busmux_loadhold_6_node_8;
    aux_mux1_10_node_0 = aux_ic_input_mux1_10_0;
    aux_mux1_10_node_1 = aux_ic_input_mux1_10_1;
    aux_mux1_10_node_2 = aux_ic_input_mux1_10_2;
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
    aux_ic_input_mux2_11_0 = aux_busmux_loadhold_6_node_2;
    aux_ic_input_mux2_11_1 = aux_busmux_loadhold_6_node_6;
    aux_ic_input_mux2_11_2 = aux_busmux_loadhold_6_node_8;
    aux_mux2_11_node_0 = aux_ic_input_mux2_11_0;
    aux_mux2_11_node_1 = aux_ic_input_mux2_11_1;
    aux_mux2_11_node_2 = aux_ic_input_mux2_11_2;
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
    aux_ic_input_mux3_12_0 = aux_busmux_loadhold_6_node_3;
    aux_ic_input_mux3_12_1 = aux_busmux_loadhold_6_node_7;
    aux_ic_input_mux3_12_2 = aux_busmux_loadhold_6_node_8;
    aux_mux3_12_node_0 = aux_ic_input_mux3_12_0;
    aux_mux3_12_node_1 = aux_ic_input_mux3_12_1;
    aux_mux3_12_node_2 = aux_ic_input_mux3_12_2;
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
    digitalWrite(led1_1, aux_level4_register_4bit_0_q0);
    digitalWrite(led2_1, aux_level4_register_4bit_0_q1_1);
    digitalWrite(led3_1, aux_level4_register_4bit_0_q2_2);
    digitalWrite(led4_1, aux_level4_register_4bit_0_q3_3);
}
