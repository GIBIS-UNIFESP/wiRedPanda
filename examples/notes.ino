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
boolean aux_clock_0 = LOW;
unsigned long aux_clock_0_lastTime = 0;
const unsigned long aux_clock_0_interval = 250;
// IC: JKFLIPFLOP
boolean aux_ic_1_q_0 = LOW;
boolean aux_ic_1_q_0_1 = LOW;
// IC: DFLIPFLOP
boolean aux_ic_0_led_0 = LOW;
boolean aux_ic_0_led_0_1 = LOW;
boolean aux_ic_0_node_0 = LOW;
boolean aux_ic_0_node_1 = LOW;
boolean aux_ic_0_node_2 = LOW;
boolean aux_ic_0_node_3 = LOW;
boolean aux_ic_0_nand_4 = LOW;
boolean aux_ic_0_node_5 = LOW;
boolean aux_ic_0_nand_6 = LOW;
boolean aux_ic_0_not_7 = LOW;
boolean aux_ic_0_nand_8 = LOW;
boolean aux_ic_0_nand_9 = LOW;
boolean aux_ic_0_nand_10 = LOW;
boolean aux_ic_0_nand_11 = LOW;
boolean aux_ic_0_node_12 = LOW;
boolean aux_ic_0_not_13 = LOW;
boolean aux_ic_0_nand_14 = LOW;
boolean aux_ic_0_nand_15 = LOW;
boolean aux_ic_0_node_16 = LOW;
boolean aux_ic_0_node_17 = LOW;
boolean aux_ic_0_node_18 = LOW;
boolean aux_ic_0_not_19 = LOW;
boolean aux_ic_0_node_20 = LOW;
boolean aux_ic_0_node_21 = LOW;
boolean aux_ic_0_node_22 = LOW;
boolean aux_ic_0_node_23 = LOW;
boolean aux_ic_0_node_24 = LOW;
boolean aux_ic_input_ic_0 = LOW;
boolean aux_ic_input_ic_1 = LOW;
boolean aux_ic_input_ic_2 = LOW;
boolean aux_ic_input_ic_3 = LOW;
// End IC: DFLIPFLOP
boolean aux_ic_1_node_1 = LOW;
boolean aux_ic_1_node_2 = LOW;
boolean aux_ic_1_node_3 = LOW;
boolean aux_ic_1_node_4 = LOW;
boolean aux_ic_1_node_5 = LOW;
boolean aux_ic_1_node_6 = LOW;
boolean aux_ic_1_node_7 = LOW;
boolean aux_ic_1_node_8 = LOW;
boolean aux_ic_1_not_9 = LOW;
boolean aux_ic_1_and_10 = LOW;
boolean aux_ic_1_node_11 = LOW;
boolean aux_ic_1_or_12 = LOW;
boolean aux_ic_1_node_13 = LOW;
boolean aux_ic_1_node_14 = LOW;
boolean aux_ic_1_node_15 = LOW;
boolean aux_ic_1_node_16 = LOW;
boolean aux_ic_1_and_17 = LOW;
boolean aux_ic_1_node_18 = LOW;
boolean aux_ic_1_node_19 = LOW;
boolean aux_ic_input_ic_4 = LOW;
// End IC: JKFLIPFLOP
boolean aux_jk_flip_flop_2_0_q = LOW;
boolean aux_jk_flip_flop_2_0_q_inclk = LOW;
boolean aux_jk_flip_flop_2_1_q = HIGH;
boolean aux_jk_flip_flop_2_1_q_inclk = LOW;
boolean aux_jk_flip_flop_3_0_q = LOW;
boolean aux_jk_flip_flop_3_0_q_inclk = LOW;
boolean aux_jk_flip_flop_3_1_q = HIGH;
boolean aux_jk_flip_flop_3_1_q_inclk = LOW;
boolean aux_jk_flip_flop_4_0_q = LOW;
boolean aux_jk_flip_flop_4_0_q_inclk = LOW;
boolean aux_jk_flip_flop_4_1_q = HIGH;
boolean aux_jk_flip_flop_4_1_q_inclk = LOW;
// IC: DECODER
boolean aux_ic_5_led_0 = LOW;
boolean aux_ic_5_led_0_1 = LOW;
boolean aux_ic_5_led_0_2 = LOW;
boolean aux_ic_5_led_0_3 = LOW;
boolean aux_ic_5_led_0_4 = LOW;
boolean aux_ic_5_led_0_5 = LOW;
boolean aux_ic_5_led_0_6 = LOW;
boolean aux_ic_5_led_0_7 = LOW;
boolean aux_ic_5_node_0 = LOW;
boolean aux_ic_5_node_1 = LOW;
boolean aux_ic_5_node_2 = LOW;
boolean aux_ic_5_node_3 = LOW;
boolean aux_ic_5_node_4 = LOW;
boolean aux_ic_5_node_5 = LOW;
boolean aux_ic_5_node_6 = LOW;
boolean aux_ic_5_node_7 = LOW;
boolean aux_ic_5_not_8 = LOW;
boolean aux_ic_5_not_9 = LOW;
boolean aux_ic_5_not_10 = LOW;
boolean aux_ic_5_node_11 = LOW;
boolean aux_ic_5_node_12 = LOW;
boolean aux_ic_5_node_13 = LOW;
boolean aux_ic_5_node_14 = LOW;
boolean aux_ic_5_node_15 = LOW;
boolean aux_ic_5_node_16 = LOW;
boolean aux_ic_5_node_17 = LOW;
boolean aux_ic_5_node_18 = LOW;
boolean aux_ic_5_node_19 = LOW;
boolean aux_ic_5_node_20 = LOW;
boolean aux_ic_5_node_21 = LOW;
boolean aux_ic_5_node_22 = LOW;
boolean aux_ic_5_and_23 = LOW;
boolean aux_ic_5_and_24 = LOW;
boolean aux_ic_5_and_25 = LOW;
boolean aux_ic_5_and_26 = LOW;
boolean aux_ic_5_and_27 = LOW;
boolean aux_ic_5_and_28 = LOW;
boolean aux_ic_5_and_29 = LOW;
boolean aux_ic_5_and_30 = LOW;
boolean aux_ic_5_node_31 = LOW;
boolean aux_ic_5_node_32 = LOW;
boolean aux_ic_5_node_33 = LOW;
boolean aux_ic_5_node_34 = LOW;
boolean aux_ic_5_node_35 = LOW;
boolean aux_ic_5_node_36 = LOW;
boolean aux_ic_5_node_37 = LOW;
boolean aux_ic_5_node_38 = LOW;
boolean aux_ic_5_node_39 = LOW;
boolean aux_ic_5_node_40 = LOW;
boolean aux_ic_5_node_41 = LOW;
boolean aux_ic_5_node_42 = LOW;
boolean aux_ic_5_node_43 = LOW;
boolean aux_ic_5_node_44 = LOW;
boolean aux_ic_5_node_45 = LOW;
boolean aux_ic_5_node_46 = LOW;
boolean aux_ic_5_node_47 = LOW;
boolean aux_ic_5_node_48 = LOW;
boolean aux_ic_5_node_49 = LOW;
boolean aux_ic_5_node_50 = LOW;
boolean aux_ic_5_node_51 = LOW;
boolean aux_ic_5_node_52 = LOW;
boolean aux_ic_5_node_53 = LOW;
boolean aux_ic_5_node_54 = LOW;
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
    // Reading input data //.

    // Updating clocks. //
    unsigned long now = millis();
    if (now - aux_clock_0_lastTime >= aux_clock_0_interval) {
        aux_clock_0_lastTime = now;
        aux_clock_0 = !aux_clock_0;
    }
    // Buzzer: Buzzer //
    if (aux_ic_5_led_0_4 == HIGH) {
        tone(buzzer7_g6, 1568);
    } else {
        noTone(buzzer7_g6);
    }
    // Buzzer: Buzzer //
    if (aux_ic_5_led_0_3 == HIGH) {
        tone(buzzer8_f6, 1397);
    } else {
        noTone(buzzer8_f6);
    }
    // Buzzer: Buzzer //
    if (aux_ic_5_led_0_1 == HIGH) {
        tone(buzzer9_d6, 1175);
    } else {
        noTone(buzzer9_d6);
    }
    // Buzzer: Buzzer //
    if (aux_ic_5_led_0_6 == HIGH) {
        tone(buzzer10_b7, 3951);
    } else {
        noTone(buzzer10_b7);
    }
    // Buzzer: Buzzer //
    if (aux_ic_5_led_0 == HIGH) {
        tone(buzzer11_c6, 1047);
    } else {
        noTone(buzzer11_c6);
    }
    // Buzzer: Buzzer //
    if (aux_ic_5_led_0_5 == HIGH) {
        tone(buzzer12_a7, 3520);
    } else {
        noTone(buzzer12_a7);
    }
    // Buzzer: Buzzer //
    if (aux_ic_5_led_0_2 == HIGH) {
        tone(buzzer13_e6, 1319);
    } else {
        noTone(buzzer13_e6);
    }
    // Buzzer: Buzzer //
    if (aux_ic_5_led_0_7 == HIGH) {
        tone(buzzer14_c7, 2093);
    } else {
        noTone(buzzer14_c7);
    }

    // Assigning aux variables. //
    // IC: JKFLIPFLOP
    aux_ic_input_ic_0 = HIGH;
    aux_ic_input_ic_1 = HIGH;
    aux_ic_input_ic_2 = aux_clock_0;
    aux_ic_input_ic_3 = HIGH;
    aux_ic_input_ic_4 = HIGH;
    aux_ic_1_node_13 = HIGH;
    aux_ic_1_node_11 = HIGH;
    aux_ic_1_node_8 = LOW;
    aux_ic_1_node_1 = aux_ic_1_node_11;
    aux_ic_1_node_3 = aux_ic_1_node_8;
    aux_ic_1_node_2 = aux_ic_1_node_13;
    // IC: DFLIPFLOP
    aux_ic_input_ic_0 = aux_ic_1_node_1;
    aux_ic_input_ic_1 = aux_ic_1_or_12;
    aux_ic_input_ic_2 = aux_ic_1_node_3;
    aux_ic_input_ic_3 = aux_ic_1_node_2;
    aux_ic_0_node_18 = LOW;
    aux_ic_0_not_19 = !aux_ic_0_node_18;
    aux_ic_0_not_7 = !aux_ic_0_not_19;
    aux_ic_0_node_20 = aux_ic_0_not_19;
    aux_ic_0_node_1 = aux_ic_0_not_7;
    aux_ic_0_node_2 = aux_ic_0_node_20;
    aux_ic_0_node_3 = LOW;
    aux_ic_0_node_16 = LOW;
    aux_ic_0_node_17 = LOW;
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
    aux_ic_1_node_14 = HIGH;
    aux_ic_1_node_15 = HIGH;
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
        if (HIGH && HIGH) { 
            boolean aux = aux_jk_flip_flop_2_0_q;
            aux_jk_flip_flop_2_0_q = aux_jk_flip_flop_2_1_q;
            aux_jk_flip_flop_2_1_q = aux;
        } else if (HIGH) {
            aux_jk_flip_flop_2_0_q = HIGH;
            aux_jk_flip_flop_2_1_q = LOW;
        } else if (HIGH) {
            aux_jk_flip_flop_2_0_q = LOW;
            aux_jk_flip_flop_2_1_q = HIGH;
        }
    }
    if (!HIGH) { 
        aux_jk_flip_flop_2_0_q = HIGH; //Preset
        aux_jk_flip_flop_2_1_q = LOW;
    } else if (!HIGH) { 
        aux_jk_flip_flop_2_0_q = LOW; //Clear
        aux_jk_flip_flop_2_1_q = HIGH;
    }
    aux_jk_flip_flop_2_0_q_inclk = aux_ic_1_q_0;
    //End of JK FlipFlop
    //JK FlipFlop
    if (aux_jk_flip_flop_2_0_q && !aux_jk_flip_flop_3_0_q_inclk) { 
        if (HIGH && HIGH) { 
            boolean aux = aux_jk_flip_flop_3_0_q;
            aux_jk_flip_flop_3_0_q = aux_jk_flip_flop_3_1_q;
            aux_jk_flip_flop_3_1_q = aux;
        } else if (HIGH) {
            aux_jk_flip_flop_3_0_q = HIGH;
            aux_jk_flip_flop_3_1_q = LOW;
        } else if (HIGH) {
            aux_jk_flip_flop_3_0_q = LOW;
            aux_jk_flip_flop_3_1_q = HIGH;
        }
    }
    if (!HIGH) { 
        aux_jk_flip_flop_3_0_q = HIGH; //Preset
        aux_jk_flip_flop_3_1_q = LOW;
    } else if (!HIGH) { 
        aux_jk_flip_flop_3_0_q = LOW; //Clear
        aux_jk_flip_flop_3_1_q = HIGH;
    }
    aux_jk_flip_flop_3_0_q_inclk = aux_jk_flip_flop_2_0_q;
    //End of JK FlipFlop
    //JK FlipFlop
    if (aux_jk_flip_flop_3_0_q && !aux_jk_flip_flop_4_0_q_inclk) { 
        if (HIGH && HIGH) { 
            boolean aux = aux_jk_flip_flop_4_0_q;
            aux_jk_flip_flop_4_0_q = aux_jk_flip_flop_4_1_q;
            aux_jk_flip_flop_4_1_q = aux;
        } else if (HIGH) {
            aux_jk_flip_flop_4_0_q = HIGH;
            aux_jk_flip_flop_4_1_q = LOW;
        } else if (HIGH) {
            aux_jk_flip_flop_4_0_q = LOW;
            aux_jk_flip_flop_4_1_q = HIGH;
        }
    }
    if (!HIGH) { 
        aux_jk_flip_flop_4_0_q = HIGH; //Preset
        aux_jk_flip_flop_4_1_q = LOW;
    } else if (!HIGH) { 
        aux_jk_flip_flop_4_0_q = LOW; //Clear
        aux_jk_flip_flop_4_1_q = HIGH;
    }
    aux_jk_flip_flop_4_0_q_inclk = aux_jk_flip_flop_3_0_q;
    //End of JK FlipFlop
    // IC: DECODER
    aux_ic_input_ic_0 = aux_jk_flip_flop_4_1_q;
    aux_ic_input_ic_1 = aux_jk_flip_flop_3_1_q;
    aux_ic_input_ic_2 = aux_jk_flip_flop_2_1_q;
    aux_ic_5_node_34 = LOW;
    aux_ic_5_node_35 = LOW;
    aux_ic_5_node_36 = LOW;
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

    // Writing output data. //
}
