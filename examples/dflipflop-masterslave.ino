// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


/* ========= Inputs ========== */
const int push_button1_d = A0;

/* ========= Outputs ========== */
const int led8_q_0 = A1;
const int led9_q_0 = A2;

/* ====== Aux. Variables ====== */
boolean push_button1_d_val = LOW;
boolean aux_clock_0 = LOW;
unsigned long aux_clock_0_lastTime = 0;
const unsigned long aux_clock_0_interval = 1000;
boolean aux_not_1 = LOW;
boolean aux_push_button_2 = LOW;
boolean aux_node_3 = LOW;
// IC: DLATCH
boolean aux_ic_4_q_0 = LOW;
boolean aux_ic_4_q_0_1 = LOW;
boolean aux_ic_4_node_0 = LOW;
boolean aux_ic_4_node_1 = LOW;
boolean aux_ic_4_nand_2 = LOW;
boolean aux_ic_4_nand_3 = LOW;
boolean aux_ic_4_not_4 = LOW;
boolean aux_ic_4_nand_5 = LOW;
boolean aux_ic_4_nand_6 = LOW;
boolean aux_ic_4_node_7 = LOW;
boolean aux_ic_4_node_8 = LOW;
boolean aux_ic_input_ic_0 = LOW;
boolean aux_ic_input_ic_1 = LOW;
// End IC: DLATCH
boolean aux_node_5 = LOW;
// IC: DLATCH
boolean aux_ic_6_q_0 = LOW;
boolean aux_ic_6_q_0_1 = LOW;
boolean aux_ic_6_node_0 = LOW;
boolean aux_ic_6_node_1 = LOW;
boolean aux_ic_6_nand_2 = LOW;
boolean aux_ic_6_nand_3 = LOW;
boolean aux_ic_6_not_4 = LOW;
boolean aux_ic_6_nand_5 = LOW;
boolean aux_ic_6_nand_6 = LOW;
boolean aux_ic_6_node_7 = LOW;
boolean aux_ic_6_node_8 = LOW;
// End IC: DLATCH

void setup() {
    pinMode(push_button1_d, INPUT);
    pinMode(led8_q_0, OUTPUT);
    pinMode(led9_q_0, OUTPUT);
}

void loop() {
    // Reading input data //.
    push_button1_d_val = digitalRead(push_button1_d);

    // Updating clocks. //
    unsigned long now = millis();
    if (now - aux_clock_0_lastTime >= aux_clock_0_interval) {
        aux_clock_0_lastTime = now;
        aux_clock_0 = !aux_clock_0;
    }

    // Assigning aux variables. //
    aux_not_1 = !aux_clock_0;
    aux_push_button_2 = push_button1_d_val;
    aux_node_3 = aux_clock_0;
    // IC: DLATCH
    aux_ic_input_ic_0 = aux_push_button_2;
    aux_ic_input_ic_1 = aux_node_3;
    aux_ic_4_node_8 = LOW;
    aux_ic_4_not_4 = !aux_ic_4_node_8;
    aux_ic_4_node_7 = LOW;
    aux_ic_4_nand_5 = !(aux_ic_4_node_7 && aux_ic_4_not_4);
    aux_ic_4_nand_2 = !(aux_ic_4_nand_3 && aux_ic_4_nand_5);
    aux_ic_4_nand_6 = !(aux_ic_4_node_8 && aux_ic_4_node_7);
    aux_ic_4_nand_3 = !(aux_ic_4_nand_6 && aux_ic_4_nand_2);
    aux_ic_4_node_0 = aux_ic_4_nand_2;
    aux_ic_4_node_1 = aux_ic_4_nand_3;
    aux_ic_4_q_0 = aux_ic_4_node_1;
    aux_ic_4_q_0_1 = aux_ic_4_node_0;
    // End IC: DLATCH
    aux_node_5 = aux_not_1;
    // IC: DLATCH
    aux_ic_input_ic_0 = aux_ic_4_q_0;
    aux_ic_input_ic_1 = aux_node_5;
    aux_ic_6_node_8 = LOW;
    aux_ic_6_not_4 = !aux_ic_6_node_8;
    aux_ic_6_node_7 = LOW;
    aux_ic_6_nand_5 = !(aux_ic_6_node_7 && aux_ic_6_not_4);
    aux_ic_6_nand_2 = !(aux_ic_6_nand_3 && aux_ic_6_nand_5);
    aux_ic_6_nand_6 = !(aux_ic_6_node_8 && aux_ic_6_node_7);
    aux_ic_6_nand_3 = !(aux_ic_6_nand_6 && aux_ic_6_nand_2);
    aux_ic_6_node_0 = aux_ic_6_nand_2;
    aux_ic_6_node_1 = aux_ic_6_nand_3;
    aux_ic_6_q_0 = aux_ic_6_node_1;
    aux_ic_6_q_0_1 = aux_ic_6_node_0;
    // End IC: DLATCH

    // Writing output data. //
    digitalWrite(led8_q_0, aux_ic_6_q_0_1);
    digitalWrite(led9_q_0, aux_ic_6_q_0);
}
