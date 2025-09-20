// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


/* ========= Inputs ========== */
const int input_switch1_p2 = A0;
const int input_switch2_p1 = A1;
const int input_switch3_p3 = A2;

/* ========= Outputs ========== */
const int seven_segment_display32_g_middle = A3;
const int seven_segment_display32_f_upper_left = A4;
const int seven_segment_display32_e_lower_left = A5;
const int seven_segment_display32_d_bottom = 2;
const int seven_segment_display32_a_top = 3;
const int seven_segment_display32_b_upper_right = 4;
const int seven_segment_display32_dp_dot = 5;
const int seven_segment_display32_c_lower_right = 6;

/* ====== Aux. Variables ====== */
boolean input_switch1_p2_val = LOW;
boolean input_switch2_p1_val = LOW;
boolean input_switch3_p3_val = LOW;
boolean aux_input_switch_1 = LOW;
boolean aux_input_switch_2 = LOW;
boolean aux_input_switch_3 = LOW;
boolean aux_or_4 = LOW;
boolean aux_or_5 = LOW;
boolean aux_or_6 = LOW;
boolean aux_not_7 = LOW;
boolean aux_not_8 = LOW;
boolean aux_not_9 = LOW;
boolean aux_and_10 = LOW;
boolean aux_and_11 = LOW;
boolean aux_xnor_13 = LOW;
boolean aux_and_14 = LOW;
boolean aux_and_15 = LOW;
boolean aux_and_16 = LOW;
boolean aux_and_17 = LOW;
boolean aux_and_18 = LOW;
boolean aux_and_19 = LOW;
boolean aux_xnor_20 = LOW;
boolean aux_and_21 = LOW;
boolean aux_and_22 = LOW;
boolean aux_or_23 = LOW;
boolean aux_or_24 = LOW;
boolean aux_or_25 = LOW;
boolean aux_or_26 = LOW;
boolean aux_nand_27 = LOW;
boolean aux_or_28 = LOW;
boolean aux_or_29 = LOW;
boolean aux_node_30 = LOW;

void setup() {
    pinMode(input_switch1_p2, INPUT);
    pinMode(input_switch2_p1, INPUT);
    pinMode(input_switch3_p3, INPUT);
    pinMode(seven_segment_display32_g_middle, OUTPUT);
    pinMode(seven_segment_display32_f_upper_left, OUTPUT);
    pinMode(seven_segment_display32_e_lower_left, OUTPUT);
    pinMode(seven_segment_display32_d_bottom, OUTPUT);
    pinMode(seven_segment_display32_a_top, OUTPUT);
    pinMode(seven_segment_display32_b_upper_right, OUTPUT);
    pinMode(seven_segment_display32_dp_dot, OUTPUT);
    pinMode(seven_segment_display32_c_lower_right, OUTPUT);
}

void loop() {
    // Reading input data //.
    input_switch1_p2_val = digitalRead(input_switch1_p2);
    input_switch2_p1_val = digitalRead(input_switch2_p1);
    input_switch3_p3_val = digitalRead(input_switch3_p3);

    // Updating clocks. //

    // Assigning aux variables. //
    aux_input_switch_1 = input_switch1_p2_val;
    aux_input_switch_2 = input_switch2_p1_val;
    aux_input_switch_3 = input_switch3_p3_val;
    aux_or_4 = LOW || aux_input_switch_3;
    aux_or_5 = LOW || aux_input_switch_2;
    aux_or_6 = LOW || aux_input_switch_1;
    aux_not_7 = !aux_or_6;
    aux_not_8 = !aux_or_5;
    aux_not_9 = !aux_or_4;
    aux_and_10 = aux_or_6 && aux_not_9;
    aux_and_11 = aux_not_8 && aux_or_6;
    aux_xnor_13 = !(aux_not_8 ^ aux_not_9);
    aux_and_14 = aux_or_6 && aux_not_9;
    aux_and_15 = aux_not_8 && aux_not_9;
    aux_and_16 = aux_not_7 && aux_not_9;
    aux_and_17 = aux_input_switch_1 && aux_not_9;
    aux_and_18 = aux_or_5 && aux_not_7 && aux_or_4;
    aux_and_19 = aux_or_5 && aux_not_9;
    aux_xnor_20 = !(aux_not_7 ^ aux_not_9);
    aux_and_21 = aux_not_8 && aux_not_7 && aux_not_9;
    aux_and_22 = aux_or_5 && aux_not_7;
    aux_or_23 = aux_not_8 || aux_xnor_20;
    aux_or_24 = aux_and_22 || aux_and_11 || aux_and_17;
    aux_or_25 = aux_and_15 || aux_and_11 || aux_and_14 || aux_and_18;
    aux_or_26 = aux_and_10 || aux_and_21;
    aux_nand_27 = !(aux_not_8 && aux_or_6 && aux_not_9);
    aux_or_28 = aux_and_16 || aux_and_22 || aux_and_19;
    aux_or_29 = aux_xnor_13 || aux_or_6;
    aux_node_30 = LOW;

    // Writing output data. //
    digitalWrite(seven_segment_display32_g_middle, aux_or_24);
    digitalWrite(seven_segment_display32_f_upper_left, aux_or_28);
    digitalWrite(seven_segment_display32_e_lower_left, aux_or_26);
    digitalWrite(seven_segment_display32_d_bottom, aux_or_25);
    digitalWrite(seven_segment_display32_a_top, aux_or_29);
    digitalWrite(seven_segment_display32_b_upper_right, aux_or_23);
    digitalWrite(seven_segment_display32_dp_dot, aux_node_30);
    digitalWrite(seven_segment_display32_c_lower_right, aux_nand_27);
}
