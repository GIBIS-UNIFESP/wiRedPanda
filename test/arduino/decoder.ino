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
const int led42_0 = A3;
const int led43_0 = A4;
const int led44_0 = A5;
const int led45_0 = 2;
const int led46_0 = 3;
const int led47_0 = 4;
const int led48_0 = 5;
const int led49_0 = 6;

/* ====== Aux. Variables ====== */
bool input_switch1_val = false;
bool input_switch2_val = false;
bool input_switch3_val = false;
bool aux_input_switch_0 = false;
bool aux_input_switch_1 = false;
bool aux_input_switch_2 = false;
bool aux_node_3 = false;
bool aux_node_4 = false;
bool aux_node_5 = false;
bool aux_not_6 = false;
bool aux_not_7 = false;
bool aux_not_8 = false;
bool aux_node_9 = false;
bool aux_node_10 = false;
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
bool aux_node_23 = false;
bool aux_node_24 = false;
bool aux_node_25 = false;
bool aux_node_26 = false;
bool aux_node_27 = false;
bool aux_node_28 = false;
bool aux_node_29 = false;
bool aux_node_30 = false;
bool aux_node_31 = false;
bool aux_node_32 = false;
bool aux_and_33 = false;
bool aux_and_34 = false;
bool aux_and_35 = false;
bool aux_and_36 = false;
bool aux_and_37 = false;
bool aux_and_38 = false;
bool aux_and_39 = false;
bool aux_and_40 = false;
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
    pinMode(led42_0, OUTPUT);
    pinMode(led43_0, OUTPUT);
    pinMode(led44_0, OUTPUT);
    pinMode(led45_0, OUTPUT);
    pinMode(led46_0, OUTPUT);
    pinMode(led47_0, OUTPUT);
    pinMode(led48_0, OUTPUT);
    pinMode(led49_0, OUTPUT);
}

void loop() {
    // Read input data
    input_switch1_val = digitalRead(input_switch1);
    input_switch2_val = digitalRead(input_switch2);
    input_switch3_val = digitalRead(input_switch3);

    // Update clocks

    // Update logic variables
    aux_input_switch_0 = input_switch1_val;
    aux_input_switch_1 = input_switch2_val;
    aux_input_switch_2 = input_switch3_val;
    aux_node_3 = aux_input_switch_2;
    aux_node_4 = aux_input_switch_0;
    aux_node_5 = aux_input_switch_1;
    aux_not_6 = !aux_node_3;
    aux_not_7 = !aux_node_4;
    aux_not_8 = !aux_node_5;
    aux_node_9 = aux_not_6;
    aux_node_10 = aux_not_7;
    aux_node_11 = aux_not_8;
    aux_node_12 = aux_node_5;
    aux_node_13 = aux_node_4;
    aux_node_14 = aux_node_3;
    aux_node_15 = aux_node_10;
    aux_node_16 = aux_node_9;
    aux_node_17 = aux_node_11;
    aux_node_18 = aux_node_12;
    aux_node_19 = aux_node_13;
    aux_node_20 = aux_node_14;
    aux_node_21 = aux_node_16;
    aux_node_22 = aux_node_15;
    aux_node_23 = aux_node_17;
    aux_node_24 = aux_node_18;
    aux_node_25 = aux_node_19;
    aux_node_26 = aux_node_20;
    aux_node_27 = aux_node_21;
    aux_node_28 = aux_node_22;
    aux_node_29 = aux_node_23;
    aux_node_30 = aux_node_24;
    aux_node_31 = aux_node_25;
    aux_node_32 = aux_node_26;
    aux_and_33 = aux_node_11 && aux_node_10 && aux_node_9;
    aux_and_34 = aux_node_16 && aux_node_15 && aux_node_12;
    aux_and_35 = aux_node_21 && aux_node_13 && aux_node_17;
    aux_and_36 = aux_node_27 && aux_node_19 && aux_node_18;
    aux_and_37 = aux_node_14 && aux_node_22 && aux_node_23;
    aux_and_38 = aux_node_20 && aux_node_28 && aux_node_24;
    aux_and_39 = aux_node_26 && aux_node_25 && aux_node_29;
    aux_and_40 = aux_node_32 && aux_node_31 && aux_node_30;
    aux_node_49 = aux_node_32;
    aux_node_50 = aux_node_27;
    aux_node_51 = aux_node_31;
    aux_node_52 = aux_node_28;
    aux_node_53 = aux_node_30;
    aux_node_54 = aux_node_29;

    // Write output data
    digitalWrite(led42_0, aux_and_40);
    digitalWrite(led43_0, aux_and_39);
    digitalWrite(led44_0, aux_and_38);
    digitalWrite(led45_0, aux_and_37);
    digitalWrite(led46_0, aux_and_36);
    digitalWrite(led47_0, aux_and_35);
    digitalWrite(led48_0, aux_and_34);
    digitalWrite(led49_0, aux_and_33);
}
