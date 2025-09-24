// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //
//
// Target Board: Arduino UNO R3/R4
// Pin Usage: 11/18 pins
// Standard Arduino board with 20 I/O pins (14 digital + 6 analog)
//


/* ========= Inputs ========== */
const int input_switch1 = A0;
const int input_switch2 = A1;
const int input_switch3 = A2;

/* ========= Outputs ========== */
const int led1_0 = A3;
const int led2_0 = A4;
const int led3_0 = A5;
const int led4_0 = 2;
const int led5_0 = 3;
const int led6_0 = 4;
const int led7_0 = 5;
const int led8_0 = 6;

/* ====== Aux. Variables ====== */
bool input_switch1_val = false;
bool input_switch2_val = false;
bool input_switch3_val = false;
bool aux_not_8 = false;
bool aux_not_9 = false;
bool aux_not_10 = false;
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
bool aux_and_24 = false;
bool aux_and_25 = false;
bool aux_and_26 = false;
bool aux_and_27 = false;
bool aux_and_28 = false;
bool aux_and_29 = false;
bool aux_and_30 = false;
bool aux_node_31 = false;
bool aux_node_32 = false;
bool aux_node_33 = false;
bool aux_input_switch_34 = false;
bool aux_input_switch_35 = false;
bool aux_input_switch_36 = false;
bool aux_node_37 = false;
bool aux_node_38 = false;
bool aux_node_39 = false;
bool aux_node_40 = false;
bool aux_node_41 = false;
bool aux_node_42 = false;
bool aux_node_43 = false;
bool aux_node_44 = false;
bool aux_node_45 = false;
bool aux_node_46 = false;
bool aux_node_47 = false;
bool aux_node_48 = false;
bool aux_node_49 = false;
bool aux_node_50 = false;
bool aux_node_51 = false;
bool aux_node_52 = false;
bool aux_node_53 = false;
bool aux_node_54 = false;

void setup() {
    pinMode(input_switch1, INPUT);
    pinMode(input_switch2, INPUT);
    pinMode(input_switch3, INPUT);
    pinMode(led1_0, OUTPUT);
    pinMode(led2_0, OUTPUT);
    pinMode(led3_0, OUTPUT);
    pinMode(led4_0, OUTPUT);
    pinMode(led5_0, OUTPUT);
    pinMode(led6_0, OUTPUT);
    pinMode(led7_0, OUTPUT);
    pinMode(led8_0, OUTPUT);
}

void loop() {
    // Read input data
    input_switch1_val = digitalRead(input_switch1);
    input_switch2_val = digitalRead(input_switch2);
    input_switch3_val = digitalRead(input_switch3);

    // Update clocks

    // Update logic variables
    aux_not_8 = !aux_node_11;
    aux_not_9 = !aux_node_12;
    aux_not_10 = !aux_node_13;
    aux_node_11 = aux_input_switch_36;
    aux_node_12 = aux_input_switch_34;
    aux_node_13 = aux_input_switch_35;
    aux_node_14 = aux_node_54;
    aux_node_15 = aux_node_42;
    aux_node_16 = aux_node_53;
    aux_node_17 = aux_node_47;
    aux_node_18 = aux_node_52;
    aux_node_19 = aux_node_49;
    aux_node_20 = aux_not_8;
    aux_node_21 = aux_not_9;
    aux_node_22 = aux_not_10;
    aux_and_23 = aux_node_22 && aux_node_21 && aux_node_20;
    aux_and_24 = aux_node_33 && aux_node_32 && aux_node_31;
    aux_and_25 = aux_node_39 && aux_node_38 && aux_node_37;
    aux_and_26 = aux_node_42 && aux_node_41 && aux_node_40;
    aux_and_27 = aux_node_43 && aux_node_44 && aux_node_45;
    aux_and_28 = aux_node_48 && aux_node_47 && aux_node_46;
    aux_and_29 = aux_node_51 && aux_node_50 && aux_node_49;
    aux_and_30 = aux_node_54 && aux_node_53 && aux_node_52;
    aux_node_31 = aux_node_13;
    aux_node_32 = aux_node_21;
    aux_node_33 = aux_node_20;
    aux_input_switch_34 = input_switch1_val;
    aux_input_switch_35 = input_switch2_val;
    aux_input_switch_36 = input_switch3_val;
    aux_node_37 = aux_node_22;
    aux_node_38 = aux_node_12;
    aux_node_39 = aux_node_33;
    aux_node_40 = aux_node_31;
    aux_node_41 = aux_node_38;
    aux_node_42 = aux_node_39;
    aux_node_43 = aux_node_11;
    aux_node_44 = aux_node_32;
    aux_node_45 = aux_node_37;
    aux_node_46 = aux_node_40;
    aux_node_47 = aux_node_44;
    aux_node_48 = aux_node_43;
    aux_node_49 = aux_node_45;
    aux_node_50 = aux_node_41;
    aux_node_51 = aux_node_48;
    aux_node_52 = aux_node_46;
    aux_node_53 = aux_node_50;
    aux_node_54 = aux_node_51;

    // Write output data
    digitalWrite(led1_0, aux_and_30);
    digitalWrite(led2_0, aux_and_29);
    digitalWrite(led3_0, aux_and_28);
    digitalWrite(led4_0, aux_and_27);
    digitalWrite(led5_0, aux_and_26);
    digitalWrite(led6_0, aux_and_25);
    digitalWrite(led7_0, aux_and_24);
    digitalWrite(led8_0, aux_and_23);
}
