// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "templates.h"
#include <QRegularExpression>

// D Flip-Flop template with proper edge detection
const QString ArduinoTemplates::DFLIPFLOP_TEMPLATE = R"(
// D Flip-Flop: {{INSTANCE_NAME}}
// Pins: D={{DATA_PIN}}, CLK={{CLOCK_PIN}}, Q={{Q_PIN}}, Q̄={{Q_NOT_PIN}}
void update_{{INSTANCE_NAME}}() {
    // Read asynchronous control inputs
    bool preset = digitalRead({{PRESET_PIN}});
    bool clear = digitalRead({{CLEAR_PIN}});
    
    // Handle asynchronous preset/clear (active low)
    if (!preset && !clear) {
        // Both active: Clear dominates (reset-dominant)
        {{INSTANCE_NAME}}.Q = false;
        {{INSTANCE_NAME}}.Q_not = true;
    } else if (!preset) {
        // Preset active: Set Q=1, Q̄=0
        {{INSTANCE_NAME}}.Q = true;
        {{INSTANCE_NAME}}.Q_not = false;
    } else if (!clear) {
        // Clear active: Set Q=0, Q̄=1
        {{INSTANCE_NAME}}.Q = false;
        {{INSTANCE_NAME}}.Q_not = true;
    } else if ({{INSTANCE_NAME}}.triggered) {
        // Clock edge occurred: Use ISR-captured data
        {{INSTANCE_NAME}}.Q = {{INSTANCE_NAME}}.data_captured;
        {{INSTANCE_NAME}}.Q_not = !{{INSTANCE_NAME}}.data_captured;
        {{INSTANCE_NAME}}.triggered = false; // Clear edge flag
    }
    
    // Write outputs
    digitalWrite({{Q_PIN}}, {{INSTANCE_NAME}}.Q);
    digitalWrite({{Q_NOT_PIN}}, {{INSTANCE_NAME}}.Q_not);
}
)";

// JK Flip-Flop template with toggle functionality
const QString ArduinoTemplates::JKFLIPFLOP_TEMPLATE = R"(
// JK Flip-Flop: {{INSTANCE_NAME}}
// Pins: J={{J_PIN}}, CLK={{CLOCK_PIN}}, K={{K_PIN}}, Q={{Q_PIN}}, Q̄={{Q_NOT_PIN}}
void update_{{INSTANCE_NAME}}() {
    // Read asynchronous control inputs
    bool preset = digitalRead({{PRESET_PIN}});
    bool clear = digitalRead({{CLEAR_PIN}});
    
    // Handle asynchronous preset/clear (active low)
    if (!preset && !clear) {
        {{INSTANCE_NAME}}.Q = false;
        {{INSTANCE_NAME}}.Q_not = true;
    } else if (!preset) {
        {{INSTANCE_NAME}}.Q = true;
        {{INSTANCE_NAME}}.Q_not = false;
    } else if (!clear) {
        {{INSTANCE_NAME}}.Q = false;
        {{INSTANCE_NAME}}.Q_not = true;
    } else if ({{INSTANCE_NAME}}.triggered) {
        // Clock edge occurred: JK logic with captured inputs
        bool j_captured = {{INSTANCE_NAME}}.j_captured;
        bool k_captured = {{INSTANCE_NAME}}.k_captured;
        
        if (j_captured && k_captured) {
            // Toggle mode: J=1, K=1
            {{INSTANCE_NAME}}.Q = !{{INSTANCE_NAME}}.Q;
            {{INSTANCE_NAME}}.Q_not = !{{INSTANCE_NAME}}.Q_not;
        } else if (j_captured && !k_captured) {
            // Set mode: J=1, K=0
            {{INSTANCE_NAME}}.Q = true;
            {{INSTANCE_NAME}}.Q_not = false;
        } else if (!j_captured && k_captured) {
            // Reset mode: J=0, K=1
            {{INSTANCE_NAME}}.Q = false;
            {{INSTANCE_NAME}}.Q_not = true;
        }
        // Hold mode: J=0, K=0 - no change
        
        {{INSTANCE_NAME}}.triggered = false;
    }
    
    // Write outputs
    digitalWrite({{Q_PIN}}, {{INSTANCE_NAME}}.Q);
    digitalWrite({{Q_NOT_PIN}}, {{INSTANCE_NAME}}.Q_not);
}
)";

// SR Flip-Flop template with proper set/reset logic
const QString ArduinoTemplates::SRFLIPFLOP_TEMPLATE = R"(
// SR Flip-Flop: {{INSTANCE_NAME}}
// Pins: S={{S_PIN}}, CLK={{CLOCK_PIN}}, R={{R_PIN}}, Q={{Q_PIN}}, Q̄={{Q_NOT_PIN}}
void update_{{INSTANCE_NAME}}() {
    // Read asynchronous control inputs
    bool preset = digitalRead({{PRESET_PIN}});
    bool clear = digitalRead({{CLEAR_PIN}});
    
    // Handle asynchronous preset/clear (active low)
    if (!preset && !clear) {
        {{INSTANCE_NAME}}.Q = false;
        {{INSTANCE_NAME}}.Q_not = true;
    } else if (!preset) {
        {{INSTANCE_NAME}}.Q = true;
        {{INSTANCE_NAME}}.Q_not = false;
    } else if (!clear) {
        {{INSTANCE_NAME}}.Q = false;
        {{INSTANCE_NAME}}.Q_not = true;
    } else if ({{INSTANCE_NAME}}.triggered) {
        // Clock edge occurred: SR logic with captured inputs
        bool s_captured = {{INSTANCE_NAME}}.s_captured;
        bool r_captured = {{INSTANCE_NAME}}.r_captured;
        
        if (s_captured && r_captured) {
            // Forbidden state: Implementation-defined behavior
            // wiRedPanda handles this as both outputs HIGH
            {{INSTANCE_NAME}}.Q = true;
            {{INSTANCE_NAME}}.Q_not = true;
        } else if (s_captured && !r_captured) {
            // Set mode: S=1, R=0
            {{INSTANCE_NAME}}.Q = true;
            {{INSTANCE_NAME}}.Q_not = false;
        } else if (!s_captured && r_captured) {
            // Reset mode: S=0, R=1
            {{INSTANCE_NAME}}.Q = false;
            {{INSTANCE_NAME}}.Q_not = true;
        }
        // Hold mode: S=0, R=0 - no change
        
        {{INSTANCE_NAME}}.triggered = false;
    }
    
    // Write outputs
    digitalWrite({{Q_PIN}}, {{INSTANCE_NAME}}.Q);
    digitalWrite({{Q_NOT_PIN}}, {{INSTANCE_NAME}}.Q_not);
}
)";

// T Flip-Flop template with toggle logic
const QString ArduinoTemplates::TFLIPFLOP_TEMPLATE = R"(
// T Flip-Flop: {{INSTANCE_NAME}}
// Pins: T={{T_PIN}}, CLK={{CLOCK_PIN}}, Q={{Q_PIN}}, Q̄={{Q_NOT_PIN}}
void update_{{INSTANCE_NAME}}() {
    // Read asynchronous control inputs
    bool preset = digitalRead({{PRESET_PIN}});
    bool clear = digitalRead({{CLEAR_PIN}});
    
    // Handle asynchronous preset/clear (active low)
    if (!preset && !clear) {
        {{INSTANCE_NAME}}.Q = false;
        {{INSTANCE_NAME}}.Q_not = true;
    } else if (!preset) {
        {{INSTANCE_NAME}}.Q = true;
        {{INSTANCE_NAME}}.Q_not = false;
    } else if (!clear) {
        {{INSTANCE_NAME}}.Q = false;
        {{INSTANCE_NAME}}.Q_not = true;
    } else if ({{INSTANCE_NAME}}.triggered) {
        // Clock edge occurred: T logic with captured input
        bool t_captured = {{INSTANCE_NAME}}.t_captured;
        
        if (t_captured) {
            // Toggle mode: T=1
            {{INSTANCE_NAME}}.Q = !{{INSTANCE_NAME}}.Q;
            {{INSTANCE_NAME}}.Q_not = !{{INSTANCE_NAME}}.Q_not;
        }
        // Hold mode: T=0 - no change
        
        {{INSTANCE_NAME}}.triggered = false;
    }
    
    // Write outputs
    digitalWrite({{Q_PIN}}, {{INSTANCE_NAME}}.Q);
    digitalWrite({{Q_NOT_PIN}}, {{INSTANCE_NAME}}.Q_not);
}
)";

// Interrupt handler template with debouncing
const QString ArduinoTemplates::INTERRUPT_HANDLER_TEMPLATE = R"(
// Interrupt Service Routine for {{INSTANCE_NAME}} (Pin {{INTERRUPT_PIN}})
void {{INSTANCE_NAME}}_edgeISR() {
{{#if ENABLE_DEBOUNCING}}
    unsigned long current_time = millis();
    if (current_time - {{INSTANCE_NAME}}.last_edge_time < DEBOUNCE_DELAY) {
        return; // Ignore bounced edge
    }
    {{INSTANCE_NAME}}.last_edge_time = current_time;
{{/if}}
    
    // Capture data/control inputs at exact moment of clock edge
{{#if IS_DFLIPFLOP}}
    bool current_data = digitalRead({{DATA_PIN}});
    {{INSTANCE_NAME}}.data_captured = current_data;
{{/if}}
{{#if IS_JKFLIPFLOP}}
    bool current_j = digitalRead({{J_PIN}});
    bool current_k = digitalRead({{K_PIN}});
    {{INSTANCE_NAME}}.j_captured = current_j;
    {{INSTANCE_NAME}}.k_captured = current_k;
{{/if}}
{{#if IS_SRFLIPFLOP}}
    bool current_s = digitalRead({{S_PIN}});
    bool current_r = digitalRead({{R_PIN}});
    {{INSTANCE_NAME}}.s_captured = current_s;
    {{INSTANCE_NAME}}.r_captured = current_r;
{{/if}}
{{#if IS_TFLIPFLOP}}
    bool current_t = digitalRead({{T_PIN}});
    {{INSTANCE_NAME}}.t_captured = current_t;
{{/if}}
    
    // Signal that edge was detected
    {{INSTANCE_NAME}}.triggered = true;
}
)";

// Hardware timer ISR template
const QString ArduinoTemplates::TIMER_ISR_TEMPLATE = R"(
// Hardware Timer ISR for clock generation
ISR({{TIMER_VECTOR}}) {
    // Update clock domains
{{#each CLOCK_DOMAINS}}
    clock_domains[{{@index}}].state = !clock_domains[{{@index}}].state;
    
    // Trigger connected elements on rising edge
    if (clock_domains[{{@index}}].state) {
        for (int i = 0; i < clock_domains[{{@index}}].element_count; i++) {
            int ff_id = clock_domains[{{@index}}].connected_elements[i];
            if (ff_id >= 0 && ff_id < MAX_FLIPFLOPS) {
                // Call appropriate edge ISR
                switch (ff_states[ff_id].type) {
                    case FF_TYPE_D:
                        {{ELEMENTS.[i].INSTANCE_NAME}}_edgeISR();
                        break;
                    // Add other types as needed
                }
            }
        }
    }
{{/each}}
}
)";

// Setup section template
const QString ArduinoTemplates::SETUP_SECTION_TEMPLATE = R"(
void setup() {
    // Initialize serial communication
    Serial.begin(9600);
    Serial.println("wiRedPanda Arduino Circuit Starting...");
    Serial.println("Target: {{TARGET_BOARD}}");
    Serial.println("Flip-flops: {{FLIPFLOP_COUNT}}, Clock domains: {{CLOCK_DOMAIN_COUNT}}");
    
    // Configure input pins
{{#each INPUT_PINS}}
    pinMode({{PIN}}, INPUT);
{{#if REQUIRES_PULLUP}}
    digitalWrite({{PIN}}, HIGH); // Enable internal pull-up
{{/if}}
{{/each}}
    
    // Configure output pins
{{#each OUTPUT_PINS}}
    pinMode({{PIN}}, OUTPUT);
    digitalWrite({{PIN}}, LOW); // Initialize to LOW
{{/each}}
    
    // Initialize flip-flop states
{{#each FLIPFLOPS}}
    {{INSTANCE_NAME}}.Q = false;
    {{INSTANCE_NAME}}.Q_not = true;
    {{INSTANCE_NAME}}.triggered = false;
    {{INSTANCE_NAME}}.data_captured = false;
{{#if ../ENABLE_DEBOUNCING}}
    {{INSTANCE_NAME}}.last_edge_time = 0;
{{/if}}
{{/each}}
    
    // Attach interrupt handlers
{{#each INTERRUPT_HANDLERS}}
    attachInterrupt(digitalPinToInterrupt({{INTERRUPT_PIN}}), {{INSTANCE_NAME}}_edgeISR, RISING);
    Serial.println("Attached interrupt for {{INSTANCE_NAME}} on pin {{INTERRUPT_PIN}}");
{{/each}}
    
{{#if USES_HARDWARE_TIMERS}}
    // Configure hardware timers
    setupHardwareTimers();
{{/if}}
    
    Serial.println("Setup complete. Starting main loop...");
    delay(1000); // Brief delay for stability
}
)";

// Main loop section template
const QString ArduinoTemplates::LOOP_SECTION_TEMPLATE = R"(
void loop() {
    // Phase 1: Read all inputs
{{#each INPUT_PINS}}
    {{VAR_NAME}}_val = digitalRead({{PIN}});
{{/each}}
    
    // Phase 2: Update edge-triggered flip-flops
{{#each FLIPFLOPS}}
    update_{{INSTANCE_NAME}}();
{{/each}}
    
    // Phase 3: Update combinational logic
{{COMBINATIONAL_LOGIC}}
    
    // Phase 4: Write all outputs
{{#each OUTPUT_PINS}}
    digitalWrite({{PIN}}, {{DRIVING_VARIABLE}});
{{/each}}
    
{{#if DEBUG_MODE}}
    // Debug output (remove for production)
    static unsigned long last_debug = 0;
    if (millis() - last_debug > 1000) {
        Serial.print("Debug: ");
{{#each FLIPFLOPS}}
        Serial.print("{{INSTANCE_NAME}}.Q=");
        Serial.print({{INSTANCE_NAME}}.Q ? "1" : "0");
        Serial.print(" ");
{{/each}}
        Serial.println();
        last_debug = millis();
    }
{{/if}}
}
)";

// Template substitution implementation
QString ArduinoTemplates::substituteVariables(const QString &templateStr, const QHash<QString, QString> &vars)
{
    QString result = templateStr;
    
    // Simple variable substitution using {{VARIABLE}} syntax
    QRegularExpression re(R"(\{\{([^}]+)\}\})");
    QRegularExpressionMatchIterator matches = re.globalMatch(templateStr);
    
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        QString placeholder = match.captured(0); // Full match including {{}}
        QString varName = match.captured(1);     // Variable name
        
        if (vars.contains(varName)) {
            result.replace(placeholder, vars[varName]);
        } else {
            result.replace(placeholder, "UNDEFINED_" + varName);
        }
    }
    
    return result;
}

// Template generation methods
QString ArduinoTemplates::generateDFlipFlopTemplate(const QHash<QString, QString> &vars)
{
    return substituteVariables(DFLIPFLOP_TEMPLATE, vars);
}

QString ArduinoTemplates::generateJKFlipFlopTemplate(const QHash<QString, QString> &vars)
{
    return substituteVariables(JKFLIPFLOP_TEMPLATE, vars);
}

QString ArduinoTemplates::generateSRFlipFlopTemplate(const QHash<QString, QString> &vars)
{
    return substituteVariables(SRFLIPFLOP_TEMPLATE, vars);
}

QString ArduinoTemplates::generateTFlipFlopTemplate(const QHash<QString, QString> &vars)
{
    return substituteVariables(TFLIPFLOP_TEMPLATE, vars);
}

QString ArduinoTemplates::generateInterruptHandlerTemplate(const QHash<QString, QString> &vars)
{
    return substituteVariables(INTERRUPT_HANDLER_TEMPLATE, vars);
}

QString ArduinoTemplates::generateTimerISRTemplate(const QHash<QString, QString> &vars)
{
    return substituteVariables(TIMER_ISR_TEMPLATE, vars);
}

QString ArduinoTemplates::generateSetupSectionTemplate(const QHash<QString, QString> &vars)
{
    return substituteVariables(SETUP_SECTION_TEMPLATE, vars);
}

QString ArduinoTemplates::generateLoopSectionTemplate(const QHash<QString, QString> &vars)
{
    return substituteVariables(LOOP_SECTION_TEMPLATE, vars);
}

QString ArduinoTemplates::generateCombinationalTemplate(const QString &operation, const QHash<QString, QString> &vars)
{
    QString templateStr;
    
    if (operation == "AND") {
        templateStr = "{{OUTPUT}} = {{INPUT1}} && {{INPUT2}};";
    } else if (operation == "OR") {
        templateStr = "{{OUTPUT}} = {{INPUT1}} || {{INPUT2}};";
    } else if (operation == "NOT") {
        templateStr = "{{OUTPUT}} = !{{INPUT1}};";
    } else if (operation == "NAND") {
        templateStr = "{{OUTPUT}} = !({{INPUT1}} && {{INPUT2}});";
    } else if (operation == "NOR") {
        templateStr = "{{OUTPUT}} = !({{INPUT1}} || {{INPUT2}});";
    } else if (operation == "XOR") {
        templateStr = "{{OUTPUT}} = {{INPUT1}} ^ {{INPUT2}};";
    } else if (operation == "XNOR") {
        templateStr = "{{OUTPUT}} = !({{INPUT1}} ^ {{INPUT2}});";
    } else {
        templateStr = "// Unknown operation: " + operation;
    }
    
    return substituteVariables(templateStr, vars);
}
