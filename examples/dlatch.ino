// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


/* ========= Inputs ========== */
const int push_button1_d = A0;

/* ========= Outputs ========== */
const int led8_q_0 = A1;
const int led9_q_0 = A2;

/* ====== Aux. Variables ====== */
bool push_button1_d_val = false;
bool aux_push_button_0 = false;
bool aux_not_1 = false;
bool aux_clock_2 = false;
unsigned long aux_clock_2_lastTime = 0;
const unsigned long aux_clock_2_interval = 1000;
bool aux_nand_3 = false;
bool aux_nand_4 = false;
bool aux_nand_5 = false;
bool aux_nand_6 = false;

void setup() {
    pinMode(push_button1_d, INPUT);
    pinMode(led8_q_0, OUTPUT);
    pinMode(led9_q_0, OUTPUT);
}

void loop() {
    // Read input data
    push_button1_d_val = digitalRead(push_button1_d);

    // Update clocks
    unsigned long now = millis();
    if (now - aux_clock_2_lastTime >= aux_clock_2_interval) {
        aux_clock_2_lastTime = now;
        aux_clock_2 = !aux_clock_2;
    }

    // Update logic variables
    aux_push_button_0 = push_button1_d_val;
    aux_not_1 = !aux_push_button_0;
    aux_nand_3 = !(aux_clock_2 && aux_not_1);
    aux_nand_4 = !(aux_nand_6 && aux_nand_3);
    aux_nand_5 = !(aux_push_button_0 && aux_clock_2);
    aux_nand_6 = !(aux_nand_5 && aux_nand_4);

    // Write output data
    digitalWrite(led8_q_0, aux_nand_4);
    digitalWrite(led9_q_0, aux_nand_6);
}
