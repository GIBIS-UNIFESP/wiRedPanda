// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //
//
// Target Board: Arduino UNO R3/R4
// Pin Usage: 12/18 pins
// Standard Arduino board with 20 I/O pins (14 digital + 6 analog)
//


/* ========= Inputs ========== */
const int input_switch1_b = A0;
const int input_switch2_a = A1;
const int input_switch3_c = A2;
const int input_switch4_d = A3;

/* ========= Outputs ========== */
const int seven_segment_display29_g_middle = A4;
const int seven_segment_display29_f_upper_left = A5;
const int seven_segment_display29_e_lower_left = 2;
const int seven_segment_display29_d_bottom = 3;
const int seven_segment_display29_a_top = 4;
const int seven_segment_display29_b_upper_right = 5;
const int seven_segment_display29_dp_dot = 6;
const int seven_segment_display29_c_lower_right = 7;

/* ====== Aux. Variables ====== */
bool input_switch1_b_val = false;
bool input_switch2_a_val = false;
bool input_switch3_c_val = false;
bool input_switch4_d_val = false;
bool aux_node_0 = false;
bool aux_node_1 = false;
bool aux_node_2 = false;
bool aux_node_3 = false;
bool aux_and_4 = false;
bool aux_node_5 = false;
bool aux_node_6 = false;
bool aux_node_7 = false;
bool aux_node_8 = false;
bool aux_node_9 = false;
bool aux_node_10 = false;
bool aux_node_11 = false;
bool aux_node_12 = false;
bool aux_node_13 = false;
bool aux_node_14 = false;
bool aux_node_15 = false;
bool aux_or_16 = false;
bool aux_or_17 = false;
bool aux_node_18 = false;
bool aux_node_19 = false;
bool aux_node_20 = false;
bool aux_node_21 = false;
bool aux_node_22 = false;
bool aux_node_23 = false;
bool aux_node_24 = false;
bool aux_or_25 = false;
bool aux_or_26 = false;
bool aux_or_27 = false;
bool aux_node_29 = false;
bool aux_node_30 = false;
bool aux_node_31 = false;
bool aux_or_32 = false;
bool aux_node_33 = false;
bool aux_node_34 = false;
bool aux_node_35 = false;
bool aux_node_36 = false;
bool aux_or_37 = false;
bool aux_and_38 = false;
bool aux_and_39 = false;
bool aux_and_40 = false;
bool aux_and_41 = false;
bool aux_and_42 = false;
bool aux_and_43 = false;
bool aux_node_44 = false;
bool aux_node_45 = false;
bool aux_and_46 = false;
bool aux_node_47 = false;
bool aux_node_48 = false;
bool aux_and_49 = false;
bool aux_node_50 = false;
bool aux_node_51 = false;
bool aux_node_52 = false;
bool aux_node_53 = false;
bool aux_node_54 = false;
bool aux_node_55 = false;
bool aux_node_56 = false;
bool aux_node_57 = false;
bool aux_input_switch_58 = false;
bool aux_input_switch_59 = false;
bool aux_input_switch_60 = false;
bool aux_input_switch_61 = false;
bool aux_node_62 = false;
bool aux_node_63 = false;
bool aux_node_64 = false;
bool aux_node_65 = false;
bool aux_not_66 = false;
bool aux_not_67 = false;
bool aux_not_68 = false;
bool aux_not_69 = false;

void setup() {
    pinMode(input_switch1_b, INPUT);
    pinMode(input_switch2_a, INPUT);
    pinMode(input_switch3_c, INPUT);
    pinMode(input_switch4_d, INPUT);
    pinMode(seven_segment_display29_g_middle, OUTPUT);
    pinMode(seven_segment_display29_f_upper_left, OUTPUT);
    pinMode(seven_segment_display29_e_lower_left, OUTPUT);
    pinMode(seven_segment_display29_d_bottom, OUTPUT);
    pinMode(seven_segment_display29_a_top, OUTPUT);
    pinMode(seven_segment_display29_b_upper_right, OUTPUT);
    pinMode(seven_segment_display29_dp_dot, OUTPUT);
    pinMode(seven_segment_display29_c_lower_right, OUTPUT);
}

void loop() {
    // Read input data
    input_switch1_b_val = digitalRead(input_switch1_b);
    input_switch2_a_val = digitalRead(input_switch2_a);
    input_switch3_c_val = digitalRead(input_switch3_c);
    input_switch4_d_val = digitalRead(input_switch4_d);

    // Update clocks

    // Update logic variables
    aux_node_0 = aux_node_18;
    aux_node_1 = aux_node_0;
    aux_node_2 = aux_node_10;
    aux_node_3 = aux_node_15;
    aux_and_4 = aux_node_2 && aux_node_3;
    aux_node_5 = aux_node_6;
    aux_node_6 = aux_and_43;
    aux_node_7 = aux_node_8;
    aux_node_8 = aux_and_40;
    aux_node_9 = aux_node_23;
    aux_node_10 = aux_node_22;
    aux_node_11 = aux_node_24;
    aux_node_12 = aux_node_31;
    aux_node_13 = aux_node_14;
    aux_node_14 = aux_node_34;
    aux_node_15 = aux_node_30;
    aux_or_16 = aux_node_7 || aux_and_39 || aux_node_1 || aux_and_41;
    aux_or_17 = aux_node_5 || aux_and_4 || aux_and_39 || aux_node_0;
    aux_node_18 = aux_node_36;
    aux_node_19 = aux_node_20;
    aux_node_20 = aux_node_21;
    aux_node_21 = aux_and_46;
    aux_node_22 = aux_node_48;
    aux_node_23 = aux_node_29;
    aux_node_24 = aux_node_33;
    aux_or_25 = aux_node_22 || aux_node_23 || aux_node_24;
    aux_or_26 = aux_node_19 || aux_node_7;
    aux_or_27 = aux_node_18 || aux_node_20 || aux_node_8 || aux_and_41 || aux_and_38;
    aux_node_29 = aux_not_68;
    aux_node_30 = aux_node_44;
    aux_node_31 = aux_node_45;
    aux_or_32 = aux_and_42 || aux_node_31 || aux_node_6;
    aux_node_33 = aux_node_47;
    aux_node_34 = aux_node_35;
    aux_node_35 = aux_node_64;
    aux_node_36 = aux_node_62;
    aux_or_37 = aux_and_49 || aux_node_36 || aux_node_35 || aux_node_21;
    aux_and_38 = aux_node_11 && aux_and_39;
    aux_and_39 = aux_node_10 && aux_node_9;
    aux_and_40 = aux_node_14 && aux_node_15;
    aux_and_41 = aux_node_12 && aux_node_13;
    aux_and_42 = aux_node_34 && aux_node_33;
    aux_and_43 = aux_node_29 && aux_node_30;
    aux_node_44 = aux_not_69;
    aux_node_45 = aux_not_66;
    aux_and_46 = aux_node_45 && aux_node_44;
    aux_node_47 = aux_node_65;
    aux_node_48 = aux_node_63;
    aux_and_49 = aux_node_48 && aux_node_47;
    aux_node_50 = aux_node_11;
    aux_node_51 = aux_node_13;
    aux_node_52 = aux_node_9;
    aux_node_53 = aux_node_2;
    aux_node_54 = aux_node_12;
    aux_node_55 = aux_not_67;
    aux_node_56 = aux_node_1;
    aux_node_57 = aux_node_3;
    aux_input_switch_58 = input_switch1_b_val;
    aux_input_switch_59 = input_switch2_a_val;
    aux_input_switch_60 = input_switch3_c_val;
    aux_input_switch_61 = input_switch4_d_val;
    aux_node_62 = aux_input_switch_59;
    aux_node_63 = aux_input_switch_58;
    aux_node_64 = aux_input_switch_60;
    aux_node_65 = aux_input_switch_61;
    aux_not_66 = !aux_node_63;
    aux_not_67 = !aux_node_62;
    aux_not_68 = !aux_node_64;
    aux_not_69 = !aux_node_65;

    // Write output data
    digitalWrite(seven_segment_display29_g_middle, aux_or_16);
    digitalWrite(seven_segment_display29_f_upper_left, aux_or_17);
    digitalWrite(seven_segment_display29_e_lower_left, aux_or_26);
    digitalWrite(seven_segment_display29_d_bottom, aux_or_27);
    digitalWrite(seven_segment_display29_a_top, aux_or_37);
    digitalWrite(seven_segment_display29_b_upper_right, aux_or_32);
    digitalWrite(seven_segment_display29_dp_dot, false);
    digitalWrite(seven_segment_display29_c_lower_right, aux_or_25);
}
