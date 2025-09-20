// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


/* ========= Inputs ========== */

/* ========= Outputs ========== */
const int led11_0 = A0;
const int led12_0 = A1;
const int led13_0 = A2;
const int led14_0 = A3;

/* ====== Aux. Variables ====== */
boolean aux_clock_0 = LOW;
unsigned long aux_clock_0_lastTime = 0;
const unsigned long aux_clock_0_interval = 125;
boolean aux_clock_1 = LOW;
unsigned long aux_clock_1_lastTime = 0;
const unsigned long aux_clock_1_interval = 500;
boolean aux_node_2 = LOW;
boolean aux_node_3 = LOW;
boolean aux_d_flip_flop_4_0_q = LOW;
boolean aux_d_flip_flop_4_0_q_inclk = LOW;
boolean aux_d_flip_flop_4_0_q_last = LOW;
boolean aux_d_flip_flop_4_1_q = HIGH;
boolean aux_d_flip_flop_4_1_q_inclk = LOW;
boolean aux_d_flip_flop_4_1_q_last = LOW;
boolean aux_node_5 = LOW;
boolean aux_d_flip_flop_6_0_q = LOW;
boolean aux_d_flip_flop_6_0_q_inclk = LOW;
boolean aux_d_flip_flop_6_0_q_last = LOW;
boolean aux_d_flip_flop_6_1_q = HIGH;
boolean aux_d_flip_flop_6_1_q_inclk = LOW;
boolean aux_d_flip_flop_6_1_q_last = LOW;
boolean aux_d_flip_flop_7_0_q = LOW;
boolean aux_d_flip_flop_7_0_q_inclk = LOW;
boolean aux_d_flip_flop_7_0_q_last = LOW;
boolean aux_d_flip_flop_7_1_q = HIGH;
boolean aux_d_flip_flop_7_1_q_inclk = LOW;
boolean aux_d_flip_flop_7_1_q_last = LOW;
boolean aux_node_8 = LOW;
boolean aux_d_flip_flop_9_0_q = LOW;
boolean aux_d_flip_flop_9_0_q_inclk = LOW;
boolean aux_d_flip_flop_9_0_q_last = LOW;
boolean aux_d_flip_flop_9_1_q = HIGH;
boolean aux_d_flip_flop_9_1_q_inclk = LOW;
boolean aux_d_flip_flop_9_1_q_last = LOW;

void setup() {
    pinMode(led11_0, OUTPUT);
    pinMode(led12_0, OUTPUT);
    pinMode(led13_0, OUTPUT);
    pinMode(led14_0, OUTPUT);
}

void loop() {
    // Reading input data //.

    // Updating clocks. //
    unsigned long now = millis();
    if (now - aux_clock_0_lastTime >= aux_clock_0_interval) {
        aux_clock_0_lastTime = now;
        aux_clock_0 = !aux_clock_0;
    }
    unsigned long now = millis();
    if (now - aux_clock_1_lastTime >= aux_clock_1_interval) {
        aux_clock_1_lastTime = now;
        aux_clock_1 = !aux_clock_1;
    }

    // Assigning aux variables. //
    aux_node_2 = aux_clock_0;
    aux_node_3 = aux_node_2;
    //D FlipFlop
    if (aux_node_2 && !aux_d_flip_flop_4_0_q_inclk) { 
        aux_d_flip_flop_4_0_q = aux_d_flip_flop_4_0_q_last;
        aux_d_flip_flop_4_1_q = !aux_d_flip_flop_4_0_q_last;
    }
    if (!HIGH) { 
        aux_d_flip_flop_4_0_q = HIGH; //Preset
        aux_d_flip_flop_4_1_q = LOW;
    } else if (!HIGH) { 
        aux_d_flip_flop_4_0_q = LOW; //Clear
        aux_d_flip_flop_4_1_q = HIGH;
    }
    aux_d_flip_flop_4_0_q_inclk = aux_node_2;
    aux_d_flip_flop_4_0_q_last = aux_clock_1;
    //End of D FlipFlop
    aux_node_5 = aux_node_3;
    //D FlipFlop
    if (aux_node_3 && !aux_d_flip_flop_6_0_q_inclk) { 
        aux_d_flip_flop_6_0_q = aux_d_flip_flop_6_0_q_last;
        aux_d_flip_flop_6_1_q = !aux_d_flip_flop_6_0_q_last;
    }
    if (!HIGH) { 
        aux_d_flip_flop_6_0_q = HIGH; //Preset
        aux_d_flip_flop_6_1_q = LOW;
    } else if (!HIGH) { 
        aux_d_flip_flop_6_0_q = LOW; //Clear
        aux_d_flip_flop_6_1_q = HIGH;
    }
    aux_d_flip_flop_6_0_q_inclk = aux_node_3;
    aux_d_flip_flop_6_0_q_last = aux_d_flip_flop_4_0_q;
    //End of D FlipFlop
    //D FlipFlop
    if (aux_node_5 && !aux_d_flip_flop_7_0_q_inclk) { 
        aux_d_flip_flop_7_0_q = aux_d_flip_flop_7_0_q_last;
        aux_d_flip_flop_7_1_q = !aux_d_flip_flop_7_0_q_last;
    }
    if (!HIGH) { 
        aux_d_flip_flop_7_0_q = HIGH; //Preset
        aux_d_flip_flop_7_1_q = LOW;
    } else if (!HIGH) { 
        aux_d_flip_flop_7_0_q = LOW; //Clear
        aux_d_flip_flop_7_1_q = HIGH;
    }
    aux_d_flip_flop_7_0_q_inclk = aux_node_5;
    aux_d_flip_flop_7_0_q_last = aux_d_flip_flop_6_0_q;
    //End of D FlipFlop
    aux_node_8 = aux_node_5;
    //D FlipFlop
    if (aux_node_8 && !aux_d_flip_flop_9_0_q_inclk) { 
        aux_d_flip_flop_9_0_q = aux_d_flip_flop_9_0_q_last;
        aux_d_flip_flop_9_1_q = !aux_d_flip_flop_9_0_q_last;
    }
    if (!HIGH) { 
        aux_d_flip_flop_9_0_q = HIGH; //Preset
        aux_d_flip_flop_9_1_q = LOW;
    } else if (!HIGH) { 
        aux_d_flip_flop_9_0_q = LOW; //Clear
        aux_d_flip_flop_9_1_q = HIGH;
    }
    aux_d_flip_flop_9_0_q_inclk = aux_node_8;
    aux_d_flip_flop_9_0_q_last = aux_d_flip_flop_7_0_q;
    //End of D FlipFlop

    // Writing output data. //
    digitalWrite(led11_0, aux_d_flip_flop_4_0_q);
    digitalWrite(led12_0, aux_d_flip_flop_7_0_q);
    digitalWrite(led13_0, aux_d_flip_flop_9_0_q);
    digitalWrite(led14_0, aux_d_flip_flop_6_0_q);
}
