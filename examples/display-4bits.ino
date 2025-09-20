// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //
//
// Target Board: Arduino UNO R3/R4
// Pin Usage: 12/18 pins
// Standard Arduino board with 20 I/O pins (14 digital + 6 analog)
//


/* ========= Inputs ========== */
const int input_switch1_d = A0;
const int input_switch2_c = A1;
const int input_switch3_b = A2;
const int input_switch4_a = A3;

/* ========= Outputs ========== */
const int seven_segment_display63_g_middle = A4;
const int seven_segment_display63_f_upper_left = A5;
const int seven_segment_display63_e_lower_left = 2;
const int seven_segment_display63_d_bottom = 3;
const int seven_segment_display63_a_top = 4;
const int seven_segment_display63_b_upper_right = 5;
const int seven_segment_display63_dp_dot = 6;
const int seven_segment_display63_c_lower_right = 7;

/* ====== Aux. Variables ====== */
bool input_switch1_d_val = false;
bool input_switch2_c_val = false;
bool input_switch3_b_val = false;
bool input_switch4_a_val = false;
bool aux_input_switch_0 = false;
bool aux_node_1 = false;
bool aux_input_switch_2 = false;
bool aux_input_switch_3 = false;
bool aux_not_4 = false;
bool aux_node_5 = false;
bool aux_node_6 = false;
bool aux_node_7 = false;
bool aux_not_8 = false;
bool aux_not_9 = false;
bool aux_input_switch_10 = false;
bool aux_node_11 = false;
bool aux_node_12 = false;
bool aux_node_13 = false;
bool aux_node_14 = false;
bool aux_node_15 = false;
bool aux_node_16 = false;
bool aux_node_17 = false;
bool aux_node_18 = false;
bool aux_node_19 = false;
bool aux_node_20 = false;
bool aux_node_21 = false;
bool aux_node_22 = false;
bool aux_and_23 = false;
bool aux_node_24 = false;
bool aux_node_25 = false;
bool aux_node_26 = false;
bool aux_node_27 = false;
bool aux_and_28 = false;
bool aux_node_29 = false;
bool aux_node_30 = false;
bool aux_and_31 = false;
bool aux_node_32 = false;
bool aux_node_33 = false;
bool aux_node_34 = false;
bool aux_node_35 = false;
bool aux_and_36 = false;
bool aux_node_37 = false;
bool aux_node_38 = false;
bool aux_node_39 = false;
bool aux_node_40 = false;
bool aux_node_41 = false;
bool aux_node_42 = false;
bool aux_node_43 = false;
bool aux_node_44 = false;
bool aux_and_45 = false;
bool aux_node_46 = false;
bool aux_node_47 = false;
bool aux_and_48 = false;
bool aux_and_49 = false;
bool aux_node_50 = false;
bool aux_and_51 = false;
bool aux_and_52 = false;
bool aux_or_53 = false;
bool aux_or_54 = false;
bool aux_not_55 = false;
bool aux_or_56 = false;
bool aux_or_57 = false;
bool aux_or_58 = false;
bool aux_or_59 = false;
bool aux_or_60 = false;
bool aux_node_61 = false;
bool aux_node_63 = false;
bool aux_node_64 = false;
bool aux_node_65 = false;
bool aux_node_66 = false;
bool aux_node_67 = false;
bool aux_node_68 = false;
bool aux_node_69 = false;

void setup() {
    pinMode(input_switch1_d, INPUT);
    pinMode(input_switch2_c, INPUT);
    pinMode(input_switch3_b, INPUT);
    pinMode(input_switch4_a, INPUT);
    pinMode(seven_segment_display63_g_middle, OUTPUT);
    pinMode(seven_segment_display63_f_upper_left, OUTPUT);
    pinMode(seven_segment_display63_e_lower_left, OUTPUT);
    pinMode(seven_segment_display63_d_bottom, OUTPUT);
    pinMode(seven_segment_display63_a_top, OUTPUT);
    pinMode(seven_segment_display63_b_upper_right, OUTPUT);
    pinMode(seven_segment_display63_dp_dot, OUTPUT);
    pinMode(seven_segment_display63_c_lower_right, OUTPUT);
}

void loop() {
    // Read input data
    input_switch1_d_val = digitalRead(input_switch1_d);
    input_switch2_c_val = digitalRead(input_switch2_c);
    input_switch3_b_val = digitalRead(input_switch3_b);
    input_switch4_a_val = digitalRead(input_switch4_a);

    // Update clocks

    // Update logic variables
    aux_input_switch_0 = input_switch1_d_val;
    aux_node_1 = aux_input_switch_0;
    aux_input_switch_2 = input_switch2_c_val;
    aux_input_switch_3 = input_switch3_b_val;
    aux_not_4 = !aux_node_1;
    aux_node_5 = aux_input_switch_2;
    aux_node_6 = aux_input_switch_3;
    aux_node_7 = aux_node_5;
    aux_not_8 = !aux_node_5;
    aux_not_9 = !aux_node_6;
    aux_input_switch_10 = input_switch4_a_val;
    aux_node_11 = aux_not_4;
    aux_node_12 = aux_node_11;
    aux_node_13 = aux_node_6;
    aux_node_14 = aux_node_1;
    aux_node_15 = aux_not_9;
    aux_node_16 = aux_node_7;
    aux_node_17 = aux_not_8;
    aux_node_18 = aux_input_switch_10;
    aux_node_19 = aux_node_18;
    aux_node_20 = aux_node_13;
    aux_node_21 = aux_node_17;
    aux_node_22 = aux_node_16;
    aux_and_23 = aux_node_15 && aux_node_11;
    aux_node_24 = aux_node_12;
    aux_node_25 = aux_node_14;
    aux_node_26 = aux_node_19;
    aux_node_27 = aux_node_21;
    aux_and_28 = aux_node_22 && aux_node_24;
    aux_node_29 = aux_node_20;
    aux_node_30 = aux_node_15;
    aux_and_31 = aux_node_17 && aux_node_12;
    aux_node_32 = aux_node_25;
    aux_node_33 = aux_and_23;
    aux_node_34 = aux_node_32;
    aux_node_35 = aux_node_30;
    aux_and_36 = aux_node_29 && aux_node_27;
    aux_node_37 = aux_node_22;
    aux_node_38 = aux_and_28;
    aux_node_39 = aux_node_26;
    aux_node_40 = aux_node_24;
    aux_node_41 = aux_node_29;
    aux_node_42 = aux_and_31;
    aux_node_43 = aux_node_33;
    aux_node_44 = aux_node_38;
    aux_and_45 = aux_node_13 && aux_node_14;
    aux_node_46 = aux_node_39;
    aux_node_47 = aux_node_42;
    aux_and_48 = aux_node_41 && aux_node_40;
    aux_and_49 = aux_node_16 && aux_node_25;
    aux_node_50 = aux_node_43;
    aux_and_51 = aux_node_35 && aux_node_37;
    aux_and_52 = aux_node_34 && aux_and_36;
    aux_or_53 = aux_node_44 || aux_and_36 || aux_node_46 || aux_and_51;
    aux_or_54 = aux_node_47 || aux_and_48 || aux_and_36 || aux_node_39;
    aux_not_55 = !aux_node_18;
    aux_or_56 = aux_node_20 || aux_node_21 || aux_node_32;
    aux_or_57 = aux_node_50 || aux_node_44;
    aux_or_58 = aux_node_26 || aux_node_43 || aux_node_38 || aux_and_51 || aux_and_52;
    aux_or_59 = aux_and_49 || aux_node_30 || aux_node_42;
    aux_or_60 = aux_and_45 || aux_node_19 || aux_node_7 || aux_node_33;
    aux_node_61 = aux_node_40;
    aux_node_63 = aux_node_46;
    aux_node_64 = aux_not_55;
    aux_node_65 = aux_node_35;
    aux_node_66 = aux_node_41;
    aux_node_67 = aux_node_27;
    aux_node_68 = aux_node_37;
    aux_node_69 = aux_node_34;

    // Write output data
    digitalWrite(seven_segment_display63_g_middle, aux_or_53);
    digitalWrite(seven_segment_display63_f_upper_left, aux_or_54);
    digitalWrite(seven_segment_display63_e_lower_left, aux_or_57);
    digitalWrite(seven_segment_display63_d_bottom, aux_or_58);
    digitalWrite(seven_segment_display63_a_top, aux_or_60);
    digitalWrite(seven_segment_display63_b_upper_right, aux_or_59);
    digitalWrite(seven_segment_display63_dp_dot, false);
    digitalWrite(seven_segment_display63_c_lower_right, aux_or_56);
}
