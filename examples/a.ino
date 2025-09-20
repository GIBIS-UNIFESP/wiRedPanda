// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


/* ========= Inputs ========== */
const int push_button1_d = A0;

/* ========= Outputs ========== */
const int led4_q_0 = A1;
const int led5_q_0 = A2;

/* ====== Aux. Variables ====== */
boolean push_button1_d_val = LOW;
boolean aux_push_button_0 = LOW;
boolean aux_clock_1 = LOW;
unsigned long aux_clock_1_lastTime = 0;
const unsigned long aux_clock_1_interval = 1000;
// IC: DLATCH
boolean aux_ic_2_q_0 = LOW;
boolean aux_ic_2_q_0_1 = LOW;
boolean aux_ic_2_node_0 = LOW;
boolean aux_ic_2_node_1 = LOW;
boolean aux_ic_2_nand_2 = LOW;
boolean aux_ic_2_nand_3 = LOW;
boolean aux_ic_2_not_4 = LOW;
boolean aux_ic_2_nand_5 = LOW;
boolean aux_ic_2_nand_6 = LOW;
boolean aux_ic_2_node_7 = LOW;
boolean aux_ic_2_node_8 = LOW;
boolean aux_ic_input_ic_0 = LOW;
boolean aux_ic_input_ic_1 = LOW;
// End IC: DLATCH

void setup() {
    pinMode(push_button1_d, INPUT);
    pinMode(led4_q_0, OUTPUT);
    pinMode(led5_q_0, OUTPUT);
}

void loop() {
    // Reading input data //.
    push_button1_d_val = digitalRead(push_button1_d);

    // Updating clocks. //
    unsigned long now = millis();
    if (now - aux_clock_1_lastTime >= aux_clock_1_interval) {
        aux_clock_1_lastTime = now;
        aux_clock_1 = !aux_clock_1;
    }

    // Assigning aux variables. //
    aux_push_button_0 = push_button1_d_val;
    // IC: DLATCH
    aux_ic_input_ic_0 = aux_push_button_0;
    aux_ic_input_ic_1 = aux_clock_1;
    aux_ic_2_node_8 = aux_ic_input_ic_0;
    aux_ic_2_not_4 = !aux_ic_2_node_8;
    aux_ic_2_node_7 = aux_ic_input_ic_1;
    aux_ic_2_nand_5 = !(aux_ic_2_node_7 && aux_ic_2_not_4);
    aux_ic_2_nand_2 = !(aux_ic_2_nand_3 && aux_ic_2_nand_5);
    aux_ic_2_nand_6 = !(aux_ic_2_node_8 && aux_ic_2_node_7);
    aux_ic_2_nand_3 = !(aux_ic_2_nand_6 && aux_ic_2_nand_2);
    aux_ic_2_node_0 = aux_ic_2_nand_2;
    aux_ic_2_node_1 = aux_ic_2_nand_3;
    aux_ic_2_q_0 = aux_ic_2_node_1;
    aux_ic_2_q_0_1 = aux_ic_2_node_0;
    // End IC: DLATCH

    // Writing output data. //
    digitalWrite(led4_q_0, aux_ic_2_q_0);
    digitalWrite(led5_q_0, aux_ic_2_q_0_1);
}
