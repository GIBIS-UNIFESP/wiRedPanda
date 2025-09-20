// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


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
const int led55_0 = 6;

/* ====== Aux. Variables ====== */
boolean input_switch1_val = LOW;
boolean input_switch2_val = LOW;
boolean input_switch3_val = LOW;
boolean aux_input_switch_0 = LOW;
boolean aux_input_switch_1 = LOW;
boolean aux_input_switch_2 = LOW;
boolean aux_node_3 = LOW;
boolean aux_node_4 = LOW;
boolean aux_node_5 = LOW;
boolean aux_not_6 = LOW;
boolean aux_not_7 = LOW;
boolean aux_not_8 = LOW;
boolean aux_node_9 = LOW;
boolean aux_node_10 = LOW;
boolean aux_node_11 = LOW;
boolean aux_node_12 = LOW;
boolean aux_node_13 = LOW;
boolean aux_node_14 = LOW;
boolean aux_node_15 = LOW;
boolean aux_node_16 = LOW;
boolean aux_node_17 = LOW;
boolean aux_node_18 = LOW;
boolean aux_node_19 = LOW;
boolean aux_node_20 = LOW;
boolean aux_node_21 = LOW;
boolean aux_node_22 = LOW;
boolean aux_node_23 = LOW;
boolean aux_node_24 = LOW;
boolean aux_node_25 = LOW;
boolean aux_node_26 = LOW;
boolean aux_node_27 = LOW;
boolean aux_node_28 = LOW;
boolean aux_node_29 = LOW;
boolean aux_node_30 = LOW;
boolean aux_node_31 = LOW;
boolean aux_node_32 = LOW;
boolean aux_and_33 = LOW;
boolean aux_and_34 = LOW;
boolean aux_and_35 = LOW;
boolean aux_and_36 = LOW;
boolean aux_and_37 = LOW;
boolean aux_and_38 = LOW;
boolean aux_and_39 = LOW;
boolean aux_and_40 = LOW;
boolean aux_node_48 = LOW;
boolean aux_node_49 = LOW;
boolean aux_node_50 = LOW;
boolean aux_node_51 = LOW;
boolean aux_node_52 = LOW;
boolean aux_node_53 = LOW;

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
    pinMode(led55_0, OUTPUT);
}

void loop() {
    // Reading input data //.
    input_switch1_val = digitalRead(input_switch1);
    input_switch2_val = digitalRead(input_switch2);
    input_switch3_val = digitalRead(input_switch3);

    // Updating clocks. //

    // Assigning aux variables. //
    aux_input_switch_0 = input_switch1_val;
    aux_input_switch_1 = input_switch2_val;
    aux_input_switch_2 = input_switch3_val;
    aux_node_3 = aux_input_switch_2;
    aux_node_4 = aux_input_switch_0;
    aux_node_5 = aux_input_switch_1;
    aux_not_6 = !aux_node_3;
    aux_not_7 = !aux_node_4;
    aux_not_8 = !aux_node_5;
    aux_node_9 = aux_not_8;
    aux_node_10 = aux_not_7;
    aux_node_11 = aux_not_6;
    aux_node_12 = aux_node_3;
    aux_node_13 = aux_node_4;
    aux_node_14 = aux_node_5;
    aux_node_15 = aux_node_9;
    aux_node_16 = aux_node_12;
    aux_node_17 = aux_node_14;
    aux_node_18 = aux_node_13;
    aux_node_19 = aux_node_10;
    aux_node_20 = aux_node_11;
    aux_node_21 = aux_node_20;
    aux_node_22 = aux_node_19;
    aux_node_23 = aux_node_15;
    aux_node_24 = aux_node_17;
    aux_node_25 = aux_node_18;
    aux_node_26 = aux_node_16;
    aux_node_27 = aux_node_21;
    aux_node_28 = aux_node_22;
    aux_node_29 = aux_node_23;
    aux_node_30 = aux_node_24;
    aux_node_31 = aux_node_25;
    aux_node_32 = aux_node_26;
    aux_and_33 = aux_node_12 && aux_node_22 && aux_node_23;
    aux_and_34 = aux_node_26 && aux_node_25 && aux_node_29;
    aux_and_35 = aux_node_16 && aux_node_28 && aux_node_24;
    aux_and_36 = aux_node_32 && aux_node_31 && aux_node_30;
    aux_and_37 = aux_node_27 && aux_node_18 && aux_node_17;
    aux_and_38 = aux_node_21 && aux_node_13 && aux_node_15;
    aux_and_39 = aux_node_20 && aux_node_19 && aux_node_14;
    aux_and_40 = aux_node_9 && aux_node_10 && aux_node_11;
    aux_node_48 = aux_node_32;
    aux_node_49 = aux_node_27;
    aux_node_50 = aux_node_31;
    aux_node_51 = aux_node_28;
    aux_node_52 = aux_node_30;
    aux_node_53 = aux_node_29;

    // Writing output data. //
    digitalWrite(led42_0, aux_and_34);
    digitalWrite(led43_0, aux_and_35);
    digitalWrite(led44_0, aux_and_33);
    digitalWrite(led45_0, aux_and_37);
    digitalWrite(led46_0, aux_and_38);
    digitalWrite(led47_0, aux_and_39);
    digitalWrite(led48_0, aux_and_40);
    digitalWrite(led55_0, aux_and_36);
}
