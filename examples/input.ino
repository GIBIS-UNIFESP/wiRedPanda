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
bool push_button1_x0_val = false;
bool push_button2_x1_val = false;
bool aux_push_button_0 = false;
bool aux_push_button_1 = false;
bool aux_not_2 = false;
bool aux_xor_3 = false;
bool aux_or_4 = false;
bool aux_and_5 = false;

void setup() {
    pinMode(push_button1_x0, INPUT);
    pinMode(push_button2_x1, INPUT);
    pinMode(led7_xor_0, OUTPUT);
    pinMode(led8_not_x0_0, OUTPUT);
    pinMode(led9_and_0, OUTPUT);
    pinMode(led10_or_0, OUTPUT);
}

void loop() {
    // Read input data
    push_button1_x0_val = digitalRead(push_button1_x0);
    push_button2_x1_val = digitalRead(push_button2_x1);

    // Update clocks

    // Update logic variables
    aux_push_button_0 = push_button1_x0_val;
    aux_push_button_1 = push_button2_x1_val;
    aux_not_2 = !aux_push_button_0;
    aux_xor_3 = aux_push_button_0 ^ aux_push_button_1;
    aux_or_4 = aux_push_button_0 || aux_push_button_1;
    aux_and_5 = aux_push_button_0 && aux_push_button_1;

    // Write output data
    digitalWrite(led7_xor_0, aux_xor_3);
    digitalWrite(led8_not_x0_0, aux_not_2);
    digitalWrite(led9_and_0, aux_and_5);
    digitalWrite(led10_or_0, aux_or_4);
}
