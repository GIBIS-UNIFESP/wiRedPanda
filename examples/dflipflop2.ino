// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


/* ========= Inputs ========== */
const int input_switch1 = A0;

/* ========= Outputs ========== */
const int led22_0 = A1;
const int led24_0 = A2;

/* ====== Aux. Variables ====== */
boolean input_switch1_val = LOW;
boolean aux_clock_0 = LOW;
unsigned long aux_clock_0_lastTime = 0;
const unsigned long aux_clock_0_interval = 1000;
boolean aux_not_1 = LOW;
boolean aux_node_2 = LOW;
boolean aux_node_3 = LOW;
boolean aux_input_switch_4 = LOW;
boolean aux_not_5 = LOW;
boolean aux_not_6 = LOW;
boolean aux_node_7 = LOW;
boolean aux_node_8 = LOW;
boolean aux_nand_9 = LOW;
boolean aux_node_10 = LOW;
boolean aux_nand_11 = LOW;
boolean aux_nand_12 = LOW;
boolean aux_node_13 = LOW;
boolean aux_node_14 = LOW;
boolean aux_nand_15 = LOW;
boolean aux_nand_16 = LOW;
boolean aux_node_17 = LOW;
boolean aux_nand_18 = LOW;
boolean aux_d_flip_flop_19_0_q = LOW;
boolean aux_d_flip_flop_19_0_q_inclk = LOW;
boolean aux_d_flip_flop_19_0_q_last = LOW;
boolean aux_d_flip_flop_19_1_q = HIGH;
boolean aux_d_flip_flop_19_1_q_inclk = LOW;
boolean aux_d_flip_flop_19_1_q_last = LOW;
boolean aux_nand_20 = LOW;
boolean aux_nand_22 = LOW;

void setup() {
    pinMode(input_switch1, INPUT);
    pinMode(led22_0, OUTPUT);
    pinMode(led24_0, OUTPUT);
}

void loop() {
    // Reading input data //.
    input_switch1_val = digitalRead(input_switch1);

    // Updating clocks. //
    unsigned long now = millis();
    if (now - aux_clock_0_lastTime >= aux_clock_0_interval) {
        aux_clock_0_lastTime = now;
        aux_clock_0 = !aux_clock_0;
    }

    // Assigning aux variables. //
    aux_not_1 = !aux_clock_0;
    aux_node_2 = aux_not_1;
    aux_node_3 = aux_node_2;
    aux_input_switch_4 = input_switch1_val;
    aux_not_5 = !aux_input_switch_4;
    aux_not_6 = !aux_node_3;
    aux_node_7 = aux_not_6;
    aux_node_8 = aux_input_switch_4;
    aux_nand_9 = !(aux_node_2 && aux_not_5);
    aux_node_10 = aux_node_7;
    aux_nand_11 = !(aux_node_8 && aux_not_1);
    aux_nand_12 = !(aux_nand_15 && aux_nand_9);
    aux_node_13 = aux_node_10;
    aux_node_14 = aux_clock_0;
    aux_nand_15 = !(aux_nand_11 && aux_nand_12);
    aux_nand_16 = !(aux_nand_15 && aux_node_13);
    aux_node_17 = aux_node_14;
    aux_nand_18 = !(aux_node_10 && aux_nand_12);
    //D FlipFlop
    if (aux_node_17 && !aux_d_flip_flop_19_0_q_inclk) { 
        aux_d_flip_flop_19_0_q = aux_d_flip_flop_19_0_q_last;
        aux_d_flip_flop_19_1_q = !aux_d_flip_flop_19_0_q_last;
    }
    if (!HIGH) { 
        aux_d_flip_flop_19_0_q = HIGH; //Preset
        aux_d_flip_flop_19_1_q = LOW;
    } else if (!HIGH) { 
        aux_d_flip_flop_19_0_q = LOW; //Clear
        aux_d_flip_flop_19_1_q = HIGH;
    }
    aux_d_flip_flop_19_0_q_inclk = aux_node_17;
    aux_d_flip_flop_19_0_q_last = aux_node_8;
    //End of D FlipFlop
    aux_nand_20 = !(aux_nand_16 && aux_nand_22);
    aux_nand_22 = !(aux_nand_20 && aux_nand_18);

    // Writing output data. //
    digitalWrite(led22_0, aux_nand_20);
    digitalWrite(led24_0, aux_d_flip_flop_19_0_q);
}
