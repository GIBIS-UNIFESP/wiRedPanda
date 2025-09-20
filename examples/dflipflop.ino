// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


/* ========= Inputs ========== */
const int push_button1_d = A0;
const int input_switch2__preset = A1;
const int input_switch3__clear = A2;

/* ========= Outputs ========== */
const int led24_0 = A3;
const int led25_0 = A4;

/* ====== Aux. Variables ====== */
boolean push_button1_d_val = LOW;
boolean input_switch2__preset_val = LOW;
boolean input_switch3__clear_val = LOW;
boolean aux_clock_0 = LOW;
unsigned long aux_clock_0_lastTime = 0;
const unsigned long aux_clock_0_interval = 1000;
boolean aux_not_1 = LOW;
boolean aux_not_2 = LOW;
boolean aux_node_3 = LOW;
boolean aux_node_4 = LOW;
boolean aux_node_5 = LOW;
boolean aux_push_button_6 = LOW;
boolean aux_input_switch_7 = LOW;
boolean aux_input_switch_8 = LOW;
boolean aux_not_9 = LOW;
boolean aux_node_10 = LOW;
boolean aux_nand_11 = LOW;
boolean aux_nand_12 = LOW;
boolean aux_node_13 = LOW;
boolean aux_node_14 = LOW;
boolean aux_nand_15 = LOW;
boolean aux_node_16 = LOW;
boolean aux_nand_17 = LOW;
boolean aux_nand_18 = LOW;
boolean aux_nand_19 = LOW;
boolean aux_nand_20 = LOW;
boolean aux_node_21 = LOW;
boolean aux_nand_22 = LOW;

void setup() {
    pinMode(push_button1_d, INPUT);
    pinMode(input_switch2__preset, INPUT);
    pinMode(input_switch3__clear, INPUT);
    pinMode(led24_0, OUTPUT);
    pinMode(led25_0, OUTPUT);
}

void loop() {
    // Reading input data //.
    push_button1_d_val = digitalRead(push_button1_d);
    input_switch2__preset_val = digitalRead(input_switch2__preset);
    input_switch3__clear_val = digitalRead(input_switch3__clear);

    // Updating clocks. //
    unsigned long now = millis();
    if (now - aux_clock_0_lastTime >= aux_clock_0_interval) {
        aux_clock_0_lastTime = now;
        aux_clock_0 = !aux_clock_0;
    }

    // Assigning aux variables. //
    aux_not_1 = !aux_clock_0;
    aux_not_2 = !aux_not_1;
    aux_node_3 = aux_not_1;
    aux_node_4 = aux_not_2;
    aux_node_5 = aux_node_3;
    aux_push_button_6 = push_button1_d_val;
    aux_input_switch_7 = input_switch2__preset_val;
    aux_input_switch_8 = input_switch3__clear_val;
    aux_not_9 = !aux_push_button_6;
    aux_node_10 = aux_node_4;
    aux_nand_11 = !(aux_push_button_6 && aux_node_5);
    aux_nand_12 = !(aux_input_switch_7 && aux_nand_11 && aux_nand_18);
    aux_node_13 = aux_input_switch_8;
    aux_node_14 = aux_node_10;
    aux_nand_15 = !(aux_node_3 && aux_not_9);
    aux_node_16 = aux_input_switch_7;
    aux_nand_17 = !(aux_nand_12 && aux_node_14);
    aux_nand_18 = !(aux_nand_12 && aux_nand_15 && aux_node_13);
    aux_nand_19 = !(aux_node_10 && aux_nand_18);
    aux_nand_20 = !(aux_node_16 && aux_nand_17 && aux_nand_22);
    aux_node_21 = aux_input_switch_8;
    aux_nand_22 = !(aux_nand_20 && aux_nand_19 && aux_node_21);

    // Writing output data. //
    digitalWrite(led24_0, aux_nand_22);
    digitalWrite(led25_0, aux_nand_20);
}
