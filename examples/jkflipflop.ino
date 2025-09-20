// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


/* ========= Inputs ========== */
const int input_switch1__clear = A0;
const int input_switch2__preset = A1;
const int input_switch3_j = A2;
const int input_switch4_k = A3;

/* ========= Outputs ========== */
const int led19_q_0 = A4;
const int led20_q_0 = A5;

/* ====== Aux. Variables ====== */
boolean input_switch1__clear_val = LOW;
boolean input_switch2__preset_val = LOW;
boolean input_switch3_j_val = LOW;
boolean input_switch4_k_val = LOW;
boolean aux_input_switch_0 = LOW;
boolean aux_input_switch_1 = LOW;
boolean aux_clock_2 = LOW;
unsigned long aux_clock_2_lastTime = 0;
const unsigned long aux_clock_2_interval = 1000;
boolean aux_node_3 = LOW;
boolean aux_node_4 = LOW;
boolean aux_node_5 = LOW;
// IC: DFLIPFLOP
boolean aux_ic_6_led_0 = LOW;
boolean aux_ic_6_led_0_1 = LOW;
boolean aux_ic_6_node_0 = LOW;
boolean aux_ic_6_node_1 = LOW;
boolean aux_ic_6_node_2 = LOW;
boolean aux_ic_6_node_3 = LOW;
boolean aux_ic_6_nand_4 = LOW;
boolean aux_ic_6_node_5 = LOW;
boolean aux_ic_6_nand_6 = LOW;
boolean aux_ic_6_not_7 = LOW;
boolean aux_ic_6_nand_8 = LOW;
boolean aux_ic_6_nand_9 = LOW;
boolean aux_ic_6_nand_10 = LOW;
boolean aux_ic_6_nand_11 = LOW;
boolean aux_ic_6_node_12 = LOW;
boolean aux_ic_6_not_13 = LOW;
boolean aux_ic_6_nand_14 = LOW;
boolean aux_ic_6_nand_15 = LOW;
boolean aux_ic_6_node_16 = LOW;
boolean aux_ic_6_node_17 = LOW;
boolean aux_ic_6_node_18 = LOW;
boolean aux_ic_6_not_19 = LOW;
boolean aux_ic_6_node_20 = LOW;
boolean aux_ic_6_node_21 = LOW;
boolean aux_ic_6_node_22 = LOW;
boolean aux_ic_6_node_23 = LOW;
boolean aux_ic_6_node_24 = LOW;
boolean aux_ic_input_ic_0 = LOW;
boolean aux_ic_input_ic_1 = LOW;
boolean aux_ic_input_ic_2 = LOW;
boolean aux_ic_input_ic_3 = LOW;
// End IC: DFLIPFLOP
boolean aux_node_7 = LOW;
boolean aux_node_8 = LOW;
boolean aux_input_switch_9 = LOW;
boolean aux_input_switch_10 = LOW;
boolean aux_node_11 = LOW;
boolean aux_node_12 = LOW;
boolean aux_not_13 = LOW;
boolean aux_node_14 = LOW;
boolean aux_and_15 = LOW;
boolean aux_and_16 = LOW;
boolean aux_or_17 = LOW;

void setup() {
    pinMode(input_switch1__clear, INPUT);
    pinMode(input_switch2__preset, INPUT);
    pinMode(input_switch3_j, INPUT);
    pinMode(input_switch4_k, INPUT);
    pinMode(led19_q_0, OUTPUT);
    pinMode(led20_q_0, OUTPUT);
}

void loop() {
    // Reading input data //.
    input_switch1__clear_val = digitalRead(input_switch1__clear);
    input_switch2__preset_val = digitalRead(input_switch2__preset);
    input_switch3_j_val = digitalRead(input_switch3_j);
    input_switch4_k_val = digitalRead(input_switch4_k);

    // Updating clocks. //
    unsigned long now = millis();
    if (now - aux_clock_2_lastTime >= aux_clock_2_interval) {
        aux_clock_2_lastTime = now;
        aux_clock_2 = !aux_clock_2;
    }

    // Assigning aux variables. //
    aux_input_switch_0 = input_switch1__clear_val;
    aux_input_switch_1 = input_switch2__preset_val;
    aux_node_3 = aux_input_switch_1;
    aux_node_4 = aux_clock_2;
    aux_node_5 = aux_input_switch_0;
    // IC: DFLIPFLOP
    aux_ic_input_ic_0 = aux_node_3;
    aux_ic_input_ic_1 = aux_or_17;
    aux_ic_input_ic_2 = aux_node_4;
    aux_ic_input_ic_3 = aux_node_5;
    aux_ic_6_node_18 = LOW;
    aux_ic_6_not_19 = !aux_ic_6_node_18;
    aux_ic_6_not_7 = !aux_ic_6_not_19;
    aux_ic_6_node_20 = aux_ic_6_not_19;
    aux_ic_6_node_1 = aux_ic_6_not_7;
    aux_ic_6_node_2 = aux_ic_6_node_20;
    aux_ic_6_node_3 = LOW;
    aux_ic_6_node_16 = LOW;
    aux_ic_6_node_17 = LOW;
    aux_ic_6_not_13 = !aux_ic_6_node_3;
    aux_ic_6_node_21 = aux_ic_6_node_1;
    aux_ic_6_nand_4 = !(aux_ic_6_node_3 && aux_ic_6_node_2);
    aux_ic_6_nand_9 = !(aux_ic_6_node_16 && aux_ic_6_nand_4 && aux_ic_6_nand_8);
    aux_ic_6_node_22 = aux_ic_6_node_17;
    aux_ic_6_node_0 = aux_ic_6_node_21;
    aux_ic_6_nand_15 = !(aux_ic_6_node_20 && aux_ic_6_not_13);
    aux_ic_6_node_24 = aux_ic_6_node_16;
    aux_ic_6_nand_10 = !(aux_ic_6_nand_9 && aux_ic_6_node_0);
    aux_ic_6_nand_8 = !(aux_ic_6_nand_9 && aux_ic_6_nand_15 && aux_ic_6_node_22);
    aux_ic_6_nand_14 = !(aux_ic_6_node_21 && aux_ic_6_nand_8);
    aux_ic_6_nand_6 = !(aux_ic_6_node_24 && aux_ic_6_nand_10 && aux_ic_6_nand_11);
    aux_ic_6_node_23 = aux_ic_6_node_17;
    aux_ic_6_nand_11 = !(aux_ic_6_nand_6 && aux_ic_6_nand_14 && aux_ic_6_node_23);
    aux_ic_6_node_12 = aux_ic_6_nand_11;
    aux_ic_6_node_5 = aux_ic_6_nand_6;
    aux_ic_6_led_0 = aux_ic_6_node_5;
    aux_ic_6_led_0_1 = aux_ic_6_node_12;
    // End IC: DFLIPFLOP
    aux_node_7 = aux_ic_6_led_0_1;
    aux_node_8 = aux_ic_6_led_0;
    aux_input_switch_9 = input_switch3_j_val;
    aux_input_switch_10 = input_switch4_k_val;
    aux_node_11 = aux_node_8;
    aux_node_12 = aux_node_7;
    aux_not_13 = !aux_input_switch_9;
    aux_node_14 = aux_input_switch_10;
    aux_and_15 = aux_node_11 && aux_not_13;
    aux_and_16 = aux_node_14 && aux_node_12;
    aux_or_17 = aux_and_15 || aux_and_16;

    // Writing output data. //
    digitalWrite(led19_q_0, aux_ic_6_led_0);
    digitalWrite(led20_q_0, aux_ic_6_led_0_1);
}
