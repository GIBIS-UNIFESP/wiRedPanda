// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


/* ========= Inputs ========== */
const int push_button1_t = A0;
const int input_switch2__preset = A1;
const int input_switch3__clear = A2;

/* ========= Outputs ========== */
const int led12_q_0 = A3;
const int led13_q_0 = A4;

/* ====== Aux. Variables ====== */
boolean push_button1_t_val = LOW;
boolean input_switch2__preset_val = LOW;
boolean input_switch3__clear_val = LOW;
boolean aux_node_0 = LOW;
boolean aux_push_button_1 = LOW;
boolean aux_node_2 = LOW;
boolean aux_not_3 = LOW;
boolean aux_and_4 = LOW;
boolean aux_clock_5 = LOW;
unsigned long aux_clock_5_lastTime = 0;
const unsigned long aux_clock_5_interval = 1000;
boolean aux_input_switch_6 = LOW;
boolean aux_input_switch_7 = LOW;
boolean aux_or_8 = LOW;
// IC: DFLIPFLOP
boolean aux_ic_9_led_0 = LOW;
boolean aux_ic_9_led_0_1 = LOW;
boolean aux_ic_9_node_0 = LOW;
boolean aux_ic_9_node_1 = LOW;
boolean aux_ic_9_node_2 = LOW;
boolean aux_ic_9_node_3 = LOW;
boolean aux_ic_9_nand_4 = LOW;
boolean aux_ic_9_node_5 = LOW;
boolean aux_ic_9_nand_6 = LOW;
boolean aux_ic_9_not_7 = LOW;
boolean aux_ic_9_nand_8 = LOW;
boolean aux_ic_9_nand_9 = LOW;
boolean aux_ic_9_nand_10 = LOW;
boolean aux_ic_9_nand_11 = LOW;
boolean aux_ic_9_node_12 = LOW;
boolean aux_ic_9_not_13 = LOW;
boolean aux_ic_9_nand_14 = LOW;
boolean aux_ic_9_nand_15 = LOW;
boolean aux_ic_9_node_16 = LOW;
boolean aux_ic_9_node_17 = LOW;
boolean aux_ic_9_node_18 = LOW;
boolean aux_ic_9_not_19 = LOW;
boolean aux_ic_9_node_20 = LOW;
boolean aux_ic_9_node_21 = LOW;
boolean aux_ic_9_node_22 = LOW;
boolean aux_ic_9_node_23 = LOW;
boolean aux_ic_9_node_24 = LOW;
boolean aux_ic_input_ic_0 = LOW;
boolean aux_ic_input_ic_1 = LOW;
boolean aux_ic_input_ic_2 = LOW;
boolean aux_ic_input_ic_3 = LOW;
// End IC: DFLIPFLOP
boolean aux_node_10 = LOW;
boolean aux_and_13 = LOW;

void setup() {
    pinMode(push_button1_t, INPUT);
    pinMode(input_switch2__preset, INPUT);
    pinMode(input_switch3__clear, INPUT);
    pinMode(led12_q_0, OUTPUT);
    pinMode(led13_q_0, OUTPUT);
}

void loop() {
    // Reading input data //.
    push_button1_t_val = digitalRead(push_button1_t);
    input_switch2__preset_val = digitalRead(input_switch2__preset);
    input_switch3__clear_val = digitalRead(input_switch3__clear);

    // Updating clocks. //
    unsigned long now = millis();
    if (now - aux_clock_5_lastTime >= aux_clock_5_interval) {
        aux_clock_5_lastTime = now;
        aux_clock_5 = !aux_clock_5;
    }

    // Assigning aux variables. //
    aux_node_0 = aux_ic_9_led_0;
    aux_push_button_1 = push_button1_t_val;
    aux_node_2 = aux_node_0;
    aux_not_3 = !aux_push_button_1;
    aux_and_4 = aux_node_2 && aux_not_3;
    aux_input_switch_6 = input_switch2__preset_val;
    aux_input_switch_7 = input_switch3__clear_val;
    aux_or_8 = aux_and_4 || aux_and_13;
    // IC: DFLIPFLOP
    aux_ic_input_ic_0 = aux_input_switch_6;
    aux_ic_input_ic_1 = aux_or_8;
    aux_ic_input_ic_2 = aux_clock_5;
    aux_ic_input_ic_3 = aux_input_switch_7;
    aux_ic_9_node_18 = aux_ic_input_ic_2;
    aux_ic_9_not_19 = !aux_ic_9_node_18;
    aux_ic_9_not_7 = !aux_ic_9_not_19;
    aux_ic_9_node_20 = aux_ic_9_not_19;
    aux_ic_9_node_1 = aux_ic_9_not_7;
    aux_ic_9_node_2 = aux_ic_9_node_20;
    aux_ic_9_node_3 = aux_ic_input_ic_1;
    aux_ic_9_node_16 = aux_ic_input_ic_0;
    aux_ic_9_node_17 = aux_ic_input_ic_3;
    aux_ic_9_not_13 = !aux_ic_9_node_3;
    aux_ic_9_node_21 = aux_ic_9_node_1;
    aux_ic_9_nand_4 = !(aux_ic_9_node_3 && aux_ic_9_node_2);
    aux_ic_9_nand_9 = !(aux_ic_9_node_16 && aux_ic_9_nand_4 && aux_ic_9_nand_8);
    aux_ic_9_node_22 = aux_ic_9_node_17;
    aux_ic_9_node_0 = aux_ic_9_node_21;
    aux_ic_9_nand_15 = !(aux_ic_9_node_20 && aux_ic_9_not_13);
    aux_ic_9_node_24 = aux_ic_9_node_16;
    aux_ic_9_nand_10 = !(aux_ic_9_nand_9 && aux_ic_9_node_0);
    aux_ic_9_nand_8 = !(aux_ic_9_nand_9 && aux_ic_9_nand_15 && aux_ic_9_node_22);
    aux_ic_9_nand_14 = !(aux_ic_9_node_21 && aux_ic_9_nand_8);
    aux_ic_9_nand_6 = !(aux_ic_9_node_24 && aux_ic_9_nand_10 && aux_ic_9_nand_11);
    aux_ic_9_node_23 = aux_ic_9_node_17;
    aux_ic_9_nand_11 = !(aux_ic_9_nand_6 && aux_ic_9_nand_14 && aux_ic_9_node_23);
    aux_ic_9_node_12 = aux_ic_9_nand_11;
    aux_ic_9_node_5 = aux_ic_9_nand_6;
    aux_ic_9_led_0 = aux_ic_9_node_5;
    aux_ic_9_led_0_1 = aux_ic_9_node_12;
    // End IC: DFLIPFLOP
    aux_node_10 = aux_push_button_1;
    aux_and_13 = aux_node_10 && aux_ic_9_led_0_1;

    // Writing output data. //
    digitalWrite(led12_q_0, aux_ic_9_led_0);
    digitalWrite(led13_q_0, aux_ic_9_led_0_1);
}
