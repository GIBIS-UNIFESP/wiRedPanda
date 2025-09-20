// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


/* ========= Inputs ========== */
const int push_button1_x0 = A0;
const int push_button2_x1 = A1;

/* ========= Outputs ========== */
const int led7_xor_0 = A2;
const int led8_not_x0_0 = A3;
const int led9_and_0 = A4;
const int led10_or_0 = A5;

/* ====== Aux. Variables ====== */
boolean push_button1_x0_val = LOW;
boolean push_button2_x1_val = LOW;
boolean aux_push_button_0 = LOW;
boolean aux_push_button_1 = LOW;
boolean aux_not_2 = LOW;
boolean aux_xor_3 = LOW;
boolean aux_or_4 = LOW;
boolean aux_and_5 = LOW;

void setup() {
    pinMode(push_button1_x0, INPUT);
    pinMode(push_button2_x1, INPUT);
    pinMode(led7_xor_0, OUTPUT);
    pinMode(led8_not_x0_0, OUTPUT);
    pinMode(led9_and_0, OUTPUT);
    pinMode(led10_or_0, OUTPUT);
}

void loop() {
    // Reading input data //.
    push_button1_x0_val = digitalRead(push_button1_x0);
    push_button2_x1_val = digitalRead(push_button2_x1);

    // Updating clocks. //

    // Assigning aux variables. //
    aux_push_button_0 = push_button1_x0_val;
    aux_push_button_1 = push_button2_x1_val;
    aux_not_2 = !aux_push_button_0;
    aux_xor_3 = aux_push_button_0 ^ aux_push_button_1;
    aux_or_4 = aux_push_button_0 || aux_push_button_1;
    aux_and_5 = aux_push_button_0 && aux_push_button_1;

    // Writing output data. //
    digitalWrite(led7_xor_0, aux_xor_3);
    digitalWrite(led8_not_x0_0, aux_not_2);
    digitalWrite(led9_and_0, aux_and_5);
    digitalWrite(led10_or_0, aux_or_4);
}
