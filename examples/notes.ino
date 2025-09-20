// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


/* ========= Inputs ========== */

/* ========= Outputs ========== */
const int buzzer7_g6 = A0;
const int buzzer8_f6 = A1;
const int buzzer9_d6 = A2;
const int buzzer10_b7 = A3;
const int buzzer11_c6 = A4;
const int buzzer12_a7 = A5;
const int buzzer13_e6 = 2;
const int buzzer14_c7 = 3;

/* ====== Aux. Variables ====== */
bool aux_clock_0 = false;
unsigned long aux_clock_0_lastTime = 0;
const unsigned long aux_clock_0_interval = 250;
// IC: JKFLIPFLOP
bool aux_ic_1_q_0 = false;
bool aux_ic_1_q_0_1 = false;
// IC: DFLIPFLOP
bool aux_ic_0_led_0 = false;
bool aux_ic_0_led_0_1 = false;
bool aux_ic_0_node_0 = false;
bool aux_ic_0_node_1 = false;
bool aux_ic_0_node_2 = false;
bool aux_ic_0_node_3 = false;
bool aux_ic_0_nand_4 = false;
bool aux_ic_0_node_5 = false;
bool aux_ic_0_nand_6 = false;
bool aux_ic_0_not_7 = false;
bool aux_ic_0_nand_8 = false;
bool aux_ic_0_nand_9 = false;
bool aux_ic_0_nand_10 = false;
bool aux_ic_0_nand_11 = false;
bool aux_ic_0_node_12 = false;
bool aux_ic_0_not_13 = false;
bool aux_ic_0_nand_14 = false;
bool aux_ic_0_nand_15 = false;
bool aux_ic_0_node_16 = false;
bool aux_ic_0_node_17 = false;
bool aux_ic_0_node_18 = false;
bool aux_ic_0_not_19 = false;
bool aux_ic_0_node_20 = false;
bool aux_ic_0_node_21 = false;
bool aux_ic_0_node_22 = false;
bool aux_ic_0_node_23 = false;
bool aux_ic_0_node_24 = false;
bool aux_ic_input_ic_0 = false;
bool aux_ic_input_ic_1 = false;
bool aux_ic_input_ic_2 = false;
bool aux_ic_input_ic_3 = false;
// End IC: DFLIPFLOP
bool aux_ic_1_node_1 = false;
bool aux_ic_1_node_2 = false;
bool aux_ic_1_node_3 = false;
bool aux_ic_1_node_4 = false;
bool aux_ic_1_node_5 = false;
bool aux_ic_1_node_6 = false;
bool aux_ic_1_node_7 = false;
bool aux_ic_1_node_8 = false;
bool aux_ic_1_not_9 = false;
bool aux_ic_1_and_10 = false;
bool aux_ic_1_node_11 = false;
bool aux_ic_1_or_12 = false;
bool aux_ic_1_node_13 = false;
bool aux_ic_1_node_14 = false;
bool aux_ic_1_node_15 = false;
bool aux_ic_1_node_16 = false;
bool aux_ic_1_and_17 = false;
bool aux_ic_1_node_18 = false;
bool aux_ic_1_node_19 = false;
bool aux_ic_input_ic_4 = false;
// End IC: JKFLIPFLOP
bool aux_jk_flip_flop_2_0_q = false;
bool aux_jk_flip_flop_2_0_q_inclk = false;
bool aux_jk_flip_flop_2_1_q = true;
bool aux_jk_flip_flop_2_1_q_inclk = false;
bool aux_jk_flip_flop_3_0_q = false;
bool aux_jk_flip_flop_3_0_q_inclk = false;
bool aux_jk_flip_flop_3_1_q = true;
bool aux_jk_flip_flop_3_1_q_inclk = false;
bool aux_jk_flip_flop_4_0_q = false;
bool aux_jk_flip_flop_4_0_q_inclk = false;
bool aux_jk_flip_flop_4_1_q = true;
bool aux_jk_flip_flop_4_1_q_inclk = false;
// IC: DECODER
bool aux_ic_5_led_0 = false;
bool aux_ic_5_led_0_1 = false;
bool aux_ic_5_led_0_2 = false;
bool aux_ic_5_led_0_3 = false;
bool aux_ic_5_led_0_4 = false;
bool aux_ic_5_led_0_5 = false;
bool aux_ic_5_led_0_6 = false;
bool aux_ic_5_led_0_7 = false;
bool aux_ic_5_node_0 = false;
bool aux_ic_5_node_1 = false;
bool aux_ic_5_node_2 = false;
bool aux_ic_5_node_3 = false;
bool aux_ic_5_node_4 = false;
bool aux_ic_5_node_5 = false;
bool aux_ic_5_node_6 = false;
bool aux_ic_5_node_7 = false;
bool aux_ic_5_not_8 = false;
bool aux_ic_5_not_9 = false;
bool aux_ic_5_not_10 = false;
bool aux_ic_5_node_11 = false;
bool aux_ic_5_node_12 = false;
bool aux_ic_5_node_13 = false;
bool aux_ic_5_node_14 = false;
bool aux_ic_5_node_15 = false;
bool aux_ic_5_node_16 = false;
bool aux_ic_5_node_17 = false;
bool aux_ic_5_node_18 = false;
bool aux_ic_5_node_19 = false;
bool aux_ic_5_node_20 = false;
bool aux_ic_5_node_21 = false;
bool aux_ic_5_node_22 = false;
bool aux_ic_5_and_23 = false;
bool aux_ic_5_and_24 = false;
bool aux_ic_5_and_25 = false;
bool aux_ic_5_and_26 = false;
bool aux_ic_5_and_27 = false;
bool aux_ic_5_and_28 = false;
bool aux_ic_5_and_29 = false;
bool aux_ic_5_and_30 = false;
bool aux_ic_5_node_31 = false;
bool aux_ic_5_node_32 = false;
bool aux_ic_5_node_33 = false;
bool aux_ic_5_node_34 = false;
bool aux_ic_5_node_35 = false;
bool aux_ic_5_node_36 = false;
bool aux_ic_5_node_37 = false;
bool aux_ic_5_node_38 = false;
bool aux_ic_5_node_39 = false;
bool aux_ic_5_node_40 = false;
bool aux_ic_5_node_41 = false;
bool aux_ic_5_node_42 = false;
bool aux_ic_5_node_43 = false;
bool aux_ic_5_node_44 = false;
bool aux_ic_5_node_45 = false;
bool aux_ic_5_node_46 = false;
bool aux_ic_5_node_47 = false;
bool aux_ic_5_node_48 = false;
bool aux_ic_5_node_49 = false;
bool aux_ic_5_node_50 = false;
bool aux_ic_5_node_51 = false;
bool aux_ic_5_node_52 = false;
bool aux_ic_5_node_53 = false;
bool aux_ic_5_node_54 = false;
// End IC: DECODER

void setup() {
    pinMode(buzzer7_g6, OUTPUT);
    pinMode(buzzer8_f6, OUTPUT);
    pinMode(buzzer9_d6, OUTPUT);
    pinMode(buzzer10_b7, OUTPUT);
    pinMode(buzzer11_c6, OUTPUT);
    pinMode(buzzer12_a7, OUTPUT);
    pinMode(buzzer13_e6, OUTPUT);
    pinMode(buzzer14_c7, OUTPUT);
}

void loop() {
    // Read input data

    // Update clocks
    unsigned long now = millis();
    if (now - aux_clock_0_lastTime >= aux_clock_0_interval) {
        aux_clock_0_lastTime = now;
        aux_clock_0 = !aux_clock_0;
    }
    // Buzzer: Buzzer //
    if (aux_ic_5_led_0_4) {
        tone(buzzer7_g6, 1568);
    } else {
        noTone(buzzer7_g6);
    }
    // Buzzer: Buzzer //
    if (aux_ic_5_led_0_3) {
        tone(buzzer8_f6, 1397);
    } else {
        noTone(buzzer8_f6);
    }
    // Buzzer: Buzzer //
    if (aux_ic_5_led_0_1) {
        tone(buzzer9_d6, 1175);
    } else {
        noTone(buzzer9_d6);
    }
    // Buzzer: Buzzer //
    if (aux_ic_5_led_0_6) {
        tone(buzzer10_b7, 3951);
    } else {
        noTone(buzzer10_b7);
    }
    // Buzzer: Buzzer //
    if (aux_ic_5_led_0) {
        tone(buzzer11_c6, 1047);
    } else {
        noTone(buzzer11_c6);
    }
    // Buzzer: Buzzer //
    if (aux_ic_5_led_0_5) {
        tone(buzzer12_a7, 3520);
    } else {
        noTone(buzzer12_a7);
    }
    // Buzzer: Buzzer //
    if (aux_ic_5_led_0_2) {
        tone(buzzer13_e6, 1319);
    } else {
        noTone(buzzer13_e6);
    }
    // Buzzer: Buzzer //
    if (aux_ic_5_led_0_7) {
        tone(buzzer14_c7, 2093);
    } else {
        noTone(buzzer14_c7);
    }

    // Update logic variables
    // IC: JKFLIPFLOP
    aux_ic_input_ic_0 = true;
    aux_ic_input_ic_1 = true;
    aux_ic_input_ic_2 = aux_clock_0;
    aux_ic_input_ic_3 = true;
    aux_ic_input_ic_4 = true;
    aux_ic_1_node_13 = true;
    aux_ic_1_node_11 = true;
    aux_ic_1_node_8 = aux_ic_input_ic_2;
    aux_ic_1_node_1 = aux_ic_1_node_11;
    aux_ic_1_node_3 = aux_ic_1_node_8;
    aux_ic_1_node_2 = aux_ic_1_node_13;
    // IC: DFLIPFLOP
    aux_ic_input_ic_0 = aux_ic_1_node_1;
    aux_ic_input_ic_1 = aux_ic_1_or_12;
    aux_ic_input_ic_2 = aux_ic_1_node_3;
    aux_ic_input_ic_3 = aux_ic_1_node_2;
    aux_ic_0_node_18 = aux_ic_input_ic_2;
    aux_ic_0_not_19 = !aux_ic_0_node_18;
    aux_ic_0_not_7 = !aux_ic_0_not_19;
    aux_ic_0_node_20 = aux_ic_0_not_19;
    aux_ic_0_node_1 = aux_ic_0_not_7;
    aux_ic_0_node_2 = aux_ic_0_node_20;
    aux_ic_0_node_3 = aux_ic_input_ic_1;
    aux_ic_0_node_16 = aux_ic_input_ic_0;
    aux_ic_0_node_17 = aux_ic_input_ic_3;
    aux_ic_0_not_13 = !aux_ic_0_node_3;
    aux_ic_0_node_21 = aux_ic_0_node_1;
    aux_ic_0_nand_4 = !(aux_ic_0_node_3 && aux_ic_0_node_2);
    aux_ic_0_nand_9 = !(aux_ic_0_node_16 && aux_ic_0_nand_4 && aux_ic_0_nand_8);
    aux_ic_0_node_22 = aux_ic_0_node_17;
    aux_ic_0_node_0 = aux_ic_0_node_21;
    aux_ic_0_nand_15 = !(aux_ic_0_node_20 && aux_ic_0_not_13);
    aux_ic_0_node_24 = aux_ic_0_node_16;
    aux_ic_0_nand_10 = !(aux_ic_0_nand_9 && aux_ic_0_node_0);
    aux_ic_0_nand_8 = !(aux_ic_0_nand_9 && aux_ic_0_nand_15 && aux_ic_0_node_22);
    aux_ic_0_nand_14 = !(aux_ic_0_node_21 && aux_ic_0_nand_8);
    aux_ic_0_nand_6 = !(aux_ic_0_node_24 && aux_ic_0_nand_10 && aux_ic_0_nand_11);
    aux_ic_0_node_23 = aux_ic_0_node_17;
    aux_ic_0_nand_11 = !(aux_ic_0_nand_6 && aux_ic_0_nand_14 && aux_ic_0_node_23);
    aux_ic_0_node_12 = aux_ic_0_nand_11;
    aux_ic_0_node_5 = aux_ic_0_nand_6;
    aux_ic_0_led_0 = aux_ic_0_node_5;
    aux_ic_0_led_0_1 = aux_ic_0_node_12;
    // End IC: DFLIPFLOP
    aux_ic_1_node_6 = aux_ic_0_led_0_1;
    aux_ic_1_node_7 = aux_ic_0_led_0;
    aux_ic_1_node_14 = true;
    aux_ic_1_node_15 = true;
    aux_ic_1_node_4 = aux_ic_1_node_7;
    aux_ic_1_node_5 = aux_ic_1_node_6;
    aux_ic_1_not_9 = !aux_ic_1_node_14;
    aux_ic_1_node_16 = aux_ic_1_node_15;
    aux_ic_1_and_17 = aux_ic_1_node_4 && aux_ic_1_not_9;
    aux_ic_1_and_10 = aux_ic_1_node_16 && aux_ic_1_node_5;
    aux_ic_1_or_12 = aux_ic_1_and_17 || aux_ic_1_and_10;
    aux_ic_1_node_18 = aux_ic_0_led_0;
    aux_ic_1_node_19 = aux_ic_0_led_0_1;
    aux_ic_1_q_0 = aux_ic_1_node_18;
    aux_ic_1_q_0_1 = aux_ic_1_node_19;
    // End IC: JKFLIPFLOP
    //JK FlipFlop
    if (aux_ic_1_q_0 && !aux_jk_flip_flop_2_0_q_inclk) { 
        // Toggle mode (J=1, K=1)
        bool aux = aux_jk_flip_flop_2_0_q;
        aux_jk_flip_flop_2_0_q = aux_jk_flip_flop_2_1_q;
        aux_jk_flip_flop_2_1_q = aux;
    }
    aux_jk_flip_flop_2_0_q_inclk = aux_ic_1_q_0;
    //End of JK FlipFlop
    //JK FlipFlop
    if (aux_jk_flip_flop_2_0_q && !aux_jk_flip_flop_3_0_q_inclk) { 
        // Toggle mode (J=1, K=1)
        bool aux = aux_jk_flip_flop_3_0_q;
        aux_jk_flip_flop_3_0_q = aux_jk_flip_flop_3_1_q;
        aux_jk_flip_flop_3_1_q = aux;
    }
    aux_jk_flip_flop_3_0_q_inclk = aux_jk_flip_flop_2_0_q;
    //End of JK FlipFlop
    //JK FlipFlop
    if (aux_jk_flip_flop_3_0_q && !aux_jk_flip_flop_4_0_q_inclk) { 
        // Toggle mode (J=1, K=1)
        bool aux = aux_jk_flip_flop_4_0_q;
        aux_jk_flip_flop_4_0_q = aux_jk_flip_flop_4_1_q;
        aux_jk_flip_flop_4_1_q = aux;
    }
    aux_jk_flip_flop_4_0_q_inclk = aux_jk_flip_flop_3_0_q;
    //End of JK FlipFlop
    // IC: DECODER
    aux_ic_input_ic_0 = aux_jk_flip_flop_4_1_q;
    aux_ic_input_ic_1 = aux_jk_flip_flop_3_1_q;
    aux_ic_input_ic_2 = aux_jk_flip_flop_2_1_q;
    aux_ic_5_node_34 = aux_ic_input_ic_1;
    aux_ic_5_node_35 = aux_ic_input_ic_2;
    aux_ic_5_node_36 = aux_ic_input_ic_0;
    aux_ic_5_node_11 = aux_ic_5_node_36;
    aux_ic_5_node_12 = aux_ic_5_node_34;
    aux_ic_5_node_13 = aux_ic_5_node_35;
    aux_ic_5_not_8 = !aux_ic_5_node_11;
    aux_ic_5_not_9 = !aux_ic_5_node_12;
    aux_ic_5_not_10 = !aux_ic_5_node_13;
    aux_ic_5_node_22 = aux_ic_5_not_10;
    aux_ic_5_node_21 = aux_ic_5_not_9;
    aux_ic_5_node_20 = aux_ic_5_not_8;
    aux_ic_5_node_43 = aux_ic_5_node_11;
    aux_ic_5_node_38 = aux_ic_5_node_12;
    aux_ic_5_node_31 = aux_ic_5_node_13;
    aux_ic_5_node_37 = aux_ic_5_node_22;
    aux_ic_5_node_48 = aux_ic_5_node_43;
    aux_ic_5_node_40 = aux_ic_5_node_31;
    aux_ic_5_node_41 = aux_ic_5_node_38;
    aux_ic_5_node_32 = aux_ic_5_node_21;
    aux_ic_5_node_33 = aux_ic_5_node_20;
    aux_ic_5_node_39 = aux_ic_5_node_33;
    aux_ic_5_node_44 = aux_ic_5_node_32;
    aux_ic_5_node_45 = aux_ic_5_node_37;
    aux_ic_5_node_46 = aux_ic_5_node_40;
    aux_ic_5_node_50 = aux_ic_5_node_41;
    aux_ic_5_node_51 = aux_ic_5_node_48;
    aux_ic_5_node_42 = aux_ic_5_node_39;
    aux_ic_5_node_47 = aux_ic_5_node_44;
    aux_ic_5_node_49 = aux_ic_5_node_45;
    aux_ic_5_node_52 = aux_ic_5_node_46;
    aux_ic_5_node_53 = aux_ic_5_node_50;
    aux_ic_5_node_54 = aux_ic_5_node_51;
    aux_ic_5_and_27 = aux_ic_5_node_43 && aux_ic_5_node_44 && aux_ic_5_node_45;
    aux_ic_5_and_29 = aux_ic_5_node_51 && aux_ic_5_node_50 && aux_ic_5_node_49;
    aux_ic_5_and_28 = aux_ic_5_node_48 && aux_ic_5_node_47 && aux_ic_5_node_46;
    aux_ic_5_and_30 = aux_ic_5_node_54 && aux_ic_5_node_53 && aux_ic_5_node_52;
    aux_ic_5_and_26 = aux_ic_5_node_42 && aux_ic_5_node_41 && aux_ic_5_node_40;
    aux_ic_5_and_25 = aux_ic_5_node_39 && aux_ic_5_node_38 && aux_ic_5_node_37;
    aux_ic_5_and_24 = aux_ic_5_node_33 && aux_ic_5_node_32 && aux_ic_5_node_31;
    aux_ic_5_and_23 = aux_ic_5_node_22 && aux_ic_5_node_21 && aux_ic_5_node_20;
    aux_ic_5_node_1 = aux_ic_5_and_29;
    aux_ic_5_node_2 = aux_ic_5_and_28;
    aux_ic_5_node_3 = aux_ic_5_and_27;
    aux_ic_5_node_4 = aux_ic_5_and_26;
    aux_ic_5_node_5 = aux_ic_5_and_25;
    aux_ic_5_node_6 = aux_ic_5_and_24;
    aux_ic_5_node_7 = aux_ic_5_and_23;
    aux_ic_5_node_14 = aux_ic_5_node_54;
    aux_ic_5_node_15 = aux_ic_5_node_42;
    aux_ic_5_node_16 = aux_ic_5_node_53;
    aux_ic_5_node_17 = aux_ic_5_node_47;
    aux_ic_5_node_18 = aux_ic_5_node_52;
    aux_ic_5_node_19 = aux_ic_5_node_49;
    aux_ic_5_node_0 = aux_ic_5_and_30;
    aux_ic_5_led_0 = aux_ic_5_node_7;
    aux_ic_5_led_0_1 = aux_ic_5_node_6;
    aux_ic_5_led_0_2 = aux_ic_5_node_5;
    aux_ic_5_led_0_3 = aux_ic_5_node_4;
    aux_ic_5_led_0_4 = aux_ic_5_node_3;
    aux_ic_5_led_0_5 = aux_ic_5_node_2;
    aux_ic_5_led_0_6 = aux_ic_5_node_1;
    aux_ic_5_led_0_7 = aux_ic_5_node_0;
    // End IC: DECODER

    // Write output data
}
