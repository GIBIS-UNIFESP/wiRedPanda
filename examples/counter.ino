// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


/* ========= Inputs ========== */

/* ========= Outputs ========== */
const int led5_0 = A0;
const int led6_0 = A1;
const int led7_0 = A2;

/* ====== Aux. Variables ====== */
boolean aux_clock_0 = LOW;
unsigned long aux_clock_0_lastTime = 0;
const unsigned long aux_clock_0_interval = 250;
boolean aux_jk_flip_flop_1_0_q = LOW;
boolean aux_jk_flip_flop_1_0_q_inclk = LOW;
boolean aux_jk_flip_flop_1_1_q = HIGH;
boolean aux_jk_flip_flop_1_1_q_inclk = LOW;
boolean aux_jk_flip_flop_2_0_q = LOW;
boolean aux_jk_flip_flop_2_0_q_inclk = LOW;
boolean aux_jk_flip_flop_2_1_q = HIGH;
boolean aux_jk_flip_flop_2_1_q_inclk = LOW;
boolean aux_jk_flip_flop_3_0_q = LOW;
boolean aux_jk_flip_flop_3_0_q_inclk = LOW;
boolean aux_jk_flip_flop_3_1_q = HIGH;
boolean aux_jk_flip_flop_3_1_q_inclk = LOW;

void setup() {
    pinMode(led5_0, OUTPUT);
    pinMode(led6_0, OUTPUT);
    pinMode(led7_0, OUTPUT);
}

void loop() {
    // Reading input data //.

    // Updating clocks. //
    unsigned long now = millis();
    if (now - aux_clock_0_lastTime >= aux_clock_0_interval) {
        aux_clock_0_lastTime = now;
        aux_clock_0 = !aux_clock_0;
    }

    // Assigning aux variables. //
    //JK FlipFlop
    if (aux_clock_0 && !aux_jk_flip_flop_1_0_q_inclk) { 
        if (HIGH && HIGH) { 
            boolean aux = aux_jk_flip_flop_1_0_q;
            aux_jk_flip_flop_1_0_q = aux_jk_flip_flop_1_1_q;
            aux_jk_flip_flop_1_1_q = aux;
        } else if (HIGH) {
            aux_jk_flip_flop_1_0_q = HIGH;
            aux_jk_flip_flop_1_1_q = LOW;
        } else if (HIGH) {
            aux_jk_flip_flop_1_0_q = LOW;
            aux_jk_flip_flop_1_1_q = HIGH;
        }
    }
    if (!HIGH || !HIGH) { 
        aux_jk_flip_flop_1_0_q = !HIGH; //Preset
        aux_jk_flip_flop_1_1_q = !HIGH; //Clear
    }
    aux_jk_flip_flop_1_0_q_inclk = aux_clock_0;
    //End of JK FlipFlop
    //JK FlipFlop
    if (aux_jk_flip_flop_1_0_q && !aux_jk_flip_flop_2_0_q_inclk) { 
        if (HIGH && HIGH) { 
            boolean aux = aux_jk_flip_flop_2_0_q;
            aux_jk_flip_flop_2_0_q = aux_jk_flip_flop_2_1_q;
            aux_jk_flip_flop_2_1_q = aux;
        } else if (HIGH) {
            aux_jk_flip_flop_2_0_q = HIGH;
            aux_jk_flip_flop_2_1_q = LOW;
        } else if (HIGH) {
            aux_jk_flip_flop_2_0_q = LOW;
            aux_jk_flip_flop_2_1_q = HIGH;
        }
    }
    if (!HIGH || !HIGH) { 
        aux_jk_flip_flop_2_0_q = !HIGH; //Preset
        aux_jk_flip_flop_2_1_q = !HIGH; //Clear
    }
    aux_jk_flip_flop_2_0_q_inclk = aux_jk_flip_flop_1_0_q;
    //End of JK FlipFlop
    //JK FlipFlop
    if (aux_jk_flip_flop_2_0_q && !aux_jk_flip_flop_3_0_q_inclk) { 
        if (HIGH && HIGH) { 
            boolean aux = aux_jk_flip_flop_3_0_q;
            aux_jk_flip_flop_3_0_q = aux_jk_flip_flop_3_1_q;
            aux_jk_flip_flop_3_1_q = aux;
        } else if (HIGH) {
            aux_jk_flip_flop_3_0_q = HIGH;
            aux_jk_flip_flop_3_1_q = LOW;
        } else if (HIGH) {
            aux_jk_flip_flop_3_0_q = LOW;
            aux_jk_flip_flop_3_1_q = HIGH;
        }
    }
    if (!HIGH || !HIGH) { 
        aux_jk_flip_flop_3_0_q = !HIGH; //Preset
        aux_jk_flip_flop_3_1_q = !HIGH; //Clear
    }
    aux_jk_flip_flop_3_0_q_inclk = aux_jk_flip_flop_2_0_q;
    //End of JK FlipFlop

    // Writing output data. //
    digitalWrite(led5_0, aux_jk_flip_flop_3_1_q);
    digitalWrite(led6_0, aux_jk_flip_flop_2_1_q);
    digitalWrite(led7_0, aux_jk_flip_flop_1_1_q);
}
