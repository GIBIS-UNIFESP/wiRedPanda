// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //
//
// Target Board: Arduino UNO R3/R4
// Pin Usage: 3/18 pins
// Standard Arduino board with 20 I/O pins (14 digital + 6 analog)
//


/* ========= Inputs ========== */
const int push_button1_d = A0;

/* ========= Outputs ========== */
const int led1_q_0 = A1;
const int led2_q_0 = A2;

/* ====== Aux. Variables ====== */
bool push_button1_d_val = false;
bool aux_nand_2 = false;
bool aux_nand_3 = false;
bool aux_not_4 = false;
bool aux_nand_5 = false;
bool aux_nand_6 = false;
bool aux_clock_7 = false;
unsigned long aux_clock_7_lastTime = 0;
const unsigned long aux_clock_7_interval = 1000;
bool aux_push_button_8 = false;

void setup() {
    pinMode(push_button1_d, INPUT);
    pinMode(led1_q_0, OUTPUT);
    pinMode(led2_q_0, OUTPUT);
}

void loop() {
    // Read input data
    push_button1_d_val = digitalRead(push_button1_d);

    // Update clocks
    unsigned long now = millis();
    if (now - aux_clock_7_lastTime >= aux_clock_7_interval) {
        aux_clock_7_lastTime = now;
        aux_clock_7 = !aux_clock_7;
    }

    // Update logic variables
    aux_nand_2 = !(aux_nand_3 && aux_nand_5);
    aux_nand_3 = !(aux_nand_6 && aux_nand_2);
    aux_not_4 = !aux_push_button_8;
    aux_nand_5 = !(aux_clock_7 && aux_not_4);
    aux_nand_6 = !(aux_push_button_8 && aux_clock_7);
    aux_push_button_8 = push_button1_d_val;

    // Write output data
    digitalWrite(led1_q_0, aux_nand_2);
    digitalWrite(led2_q_0, aux_nand_3);
}
