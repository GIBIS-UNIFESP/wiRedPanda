// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "improvedcodegenerator.h"

#include "clock.h"
#include "common.h"
#include "graphicelement.h"
#include "qneconnection.h"
#include "qneport.h"

#include <QRegularExpression>
#include <QDebug>

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
#define endl Qt::endl
#endif

// Board-specific constants
const QHash<QString, int> ImprovedCodeGenerator::BOARD_RAM_LIMITS = {
    {"Arduino Uno", 2048},
    {"Arduino Nano", 2048},
    {"Arduino Mega", 8192},
    {"Arduino Leonardo", 2560}
};

const QHash<QString, int> ImprovedCodeGenerator::BOARD_PIN_COUNTS = {
    {"Arduino Uno", 20},
    {"Arduino Nano", 22},
    {"Arduino Mega", 70},
    {"Arduino Leonardo", 23}
};

const QHash<QString, QStringList> ImprovedCodeGenerator::BOARD_INTERRUPT_PINS = {
    {"Arduino Uno", {"2", "3"}},
    {"Arduino Nano", {"2", "3"}},
    {"Arduino Mega", {"2", "3", "18", "19", "20", "21"}},
    {"Arduino Leonardo", {"0", "1", "2", "3", "7"}}
};

ImprovedCodeGenerator::ImprovedCodeGenerator(const QString &fileName, const QVector<GraphicElement *> &elements)
    : m_file(fileName)
    , m_elements(elements)
{
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        addError("Failed to open output file: " + fileName);
        return;
    }
    
    m_stream.setDevice(&m_file);
    
    // Initialize available pins based on target board
    if (m_targetBoard == "Arduino Uno" || m_targetBoard == "Arduino Nano") {
        m_availablePins = QStringList{
            "A0", "A1", "A2", "A3", "A4", "A5",  // Analog pins
            "2", "3", "4", "5", "6", "7", "8",   // Digital pins  
            "9", "10", "11", "12", "13"          // PWM/Digital pins
        };
        m_interruptPins = QStringList{"2", "3"};
    } else if (m_targetBoard == "Arduino Mega") {
        // More pins available on Mega
        for (int i = 2; i <= 53; i++) {
            if (i != 0 && i != 1) { // Skip Serial pins
                m_availablePins.append(QString::number(i));
            }
        }
        for (int i = 0; i <= 15; i++) {
            m_availablePins.append("A" + QString::number(i));
        }
        m_interruptPins = QStringList{"2", "3", "18", "19", "20", "21"};
    }
    
    m_timerNames = QStringList{"Timer1", "Timer2", "Timer3", "Timer4", "Timer5"};
}

bool ImprovedCodeGenerator::generate()
{
    // Phase 1: Analyze and categorize elements
    categorizeElements();
    
    // Phase 2: Resource analysis and validation
    m_resourceProfile = analyzeResources();
    if (!validateResources()) {
        return false;
    }
    
    // Phase 3: Pin and resource assignment
    assignPins();
    assignInterrupts();
    assignTimers();
    
    // Phase 4: Generate code sections
    try {
        generateIncludes();
        generateConstants();
        generateStructures();
        generateGlobalVariables();
        generateInterruptHandlers();
        generateSetupFunction();
        generateMainLoop();
        generateUtilityFunctions();
        
        m_stream.flush();
        qCDebug(zero) << "Improved Arduino code generated successfully";
        return true;
        
    } catch (const std::exception &e) {
        addError(QString("Code generation failed: %1").arg(e.what()));
        return false;
    }
}

ResourceProfile ImprovedCodeGenerator::analyzeResources() const
{
    ResourceProfile profile;
    
    for (auto *elm : m_elements) {
        switch (elm->elementType()) {
        case ElementType::DFlipFlop:
        case ElementType::JKFlipFlop:
        case ElementType::SRFlipFlop:
        case ElementType::TFlipFlop:
            profile.flipflop_count++;
            profile.required_pins += 4; // Data, Clock, Q, Q̄ (minimum)
            profile.estimated_ram_bytes += 8; // State structure
            profile.requires_interrupts = true;
            break;
            
        case ElementType::Clock:
            profile.clock_domains++;
            profile.estimated_ram_bytes += 12; // Clock domain structure
            profile.requires_timers = true;
            break;
            
        case ElementType::InputButton:
        case ElementType::InputSwitch:
            profile.required_pins += 1;
            profile.estimated_ram_bytes += 2; // Debouncing state
            break;
            
        case ElementType::Led:
            profile.required_pins += 1;
            profile.estimated_ram_bytes += 1;
            break;
            
        case ElementType::And:
        case ElementType::Or:
        case ElementType::Not:
        case ElementType::Nand:
        case ElementType::Nor:
        case ElementType::Xor:
        case ElementType::Xnor:
            profile.combinational_elements++;
            profile.estimated_ram_bytes += 2; // Variable storage
            break;
            
        default:
            profile.combinational_elements++;
            profile.estimated_ram_bytes += 2;
            break;
        }
    }
    
    // Add overhead for Arduino framework
    profile.estimated_ram_bytes += 200; // Arduino core overhead
    profile.estimated_flash_bytes = profile.flipflop_count * 500 + 
                                   profile.combinational_elements * 50 + 
                                   2000; // Base code size
    
    return profile;
}

bool ImprovedCodeGenerator::validateResources() const
{
    const ResourceProfile &profile = m_resourceProfile;
    bool valid = true;
    
    // Check pin requirements
    int availablePins = BOARD_PIN_COUNTS.value(m_targetBoard, 20);
    if (profile.required_pins > availablePins) {
        const_cast<ImprovedCodeGenerator*>(this)->addError(
            QString("Circuit requires %1 pins, but %2 only has %3 available")
            .arg(profile.required_pins).arg(m_targetBoard).arg(availablePins));
        valid = false;
    }
    
    // Check RAM requirements
    int availableRAM = BOARD_RAM_LIMITS.value(m_targetBoard, 2048);
    if (profile.estimated_ram_bytes > availableRAM * 0.8) { // Leave 20% for stack
        const_cast<ImprovedCodeGenerator*>(this)->addWarning(
            QString("Circuit requires %1 bytes RAM, approaching %2 limit of %3 bytes")
            .arg(profile.estimated_ram_bytes).arg(m_targetBoard).arg(availableRAM));
    }
    
    // Check interrupt requirements
    QStringList interruptPins = BOARD_INTERRUPT_PINS.value(m_targetBoard);
    if (profile.requires_interrupts && profile.flipflop_count > interruptPins.size()) {
        const_cast<ImprovedCodeGenerator*>(this)->addWarning(
            QString("Circuit has %1 flip-flops but only %2 interrupt pins available. Some flip-flops will use polling.")
            .arg(profile.flipflop_count).arg(interruptPins.size()));
    }
    
    return valid;
}

void ImprovedCodeGenerator::generateIncludes()
{
    m_stream << "// ================================================================== //" << endl;
    m_stream << "// == This code was generated by wiRedPanda Improved Code Generator == //" << endl;
    m_stream << "// == Target: " << m_targetBoard << QString(50 - m_targetBoard.length(), ' ') << " == //" << endl;
    m_stream << "// ================================================================== //" << endl;
    m_stream << endl;
    
    // Essential includes
    m_stream << "#include <Arduino.h>" << endl;
    
    if (m_resourceProfile.requires_interrupts) {
        m_stream << "#include <avr/interrupt.h>" << endl;
    }
    
    if (m_enableDebouncing) {
        m_stream << endl;
        m_stream << "// Debouncing configuration" << endl;
        m_stream << "#define DEBOUNCE_DELAY 50  // milliseconds" << endl;
        m_stream << "#define VALIDATION_TIME 5  // milliseconds for edge validation" << endl;
    }
    
    m_stream << endl;
}

void ImprovedCodeGenerator::generateConstants()
{
    m_stream << "// ========== Pin Definitions ========== //" << endl;
    
    // Input pin definitions
    for (const auto &pin : m_inputMap) {
        m_stream << QString("const int %1 = %2;").arg(pin.varName, pin.pin) << endl;
    }
    
    m_stream << endl;
    
    // Output pin definitions  
    for (const auto &pin : m_outputMap) {
        m_stream << QString("const int %1 = %2;").arg(pin.varName, pin.pin) << endl;
    }
    
    m_stream << endl;
    
    // Circuit configuration constants
    m_stream << "// ========== Circuit Configuration ========== //" << endl;
    m_stream << QString("#define MAX_FLIPFLOPS %1").arg(m_flipFlops.size()) << endl;
    m_stream << QString("#define MAX_CLOCK_DOMAINS %1").arg(m_clockDomains.size()) << endl;
    
    if (!m_flipFlops.isEmpty()) {
        m_stream << "#define USES_INTERRUPTS 1" << endl;
    }
    
    m_stream << endl;
}

void ImprovedCodeGenerator::generateStructures()
{
    if (m_flipFlops.isEmpty()) {
        return;
    }
    
    m_stream << "// ========== Data Structures ========== //" << endl;
    
    // Flip-flop state structure
    m_stream << "struct FlipFlopState {" << endl;
    m_stream << "    volatile bool Q;" << endl;
    m_stream << "    volatile bool Q_not;" << endl;
    m_stream << "    volatile bool data_captured;" << endl;
    m_stream << "    volatile bool triggered;" << endl;
    m_stream << "    volatile bool preset;" << endl;
    m_stream << "    volatile bool clear;" << endl;
    if (m_enableDebouncing) {
        m_stream << "    volatile unsigned long last_edge_time;" << endl;
    }
    m_stream << "};" << endl;
    m_stream << endl;
    
    // Clock domain structure if needed
    if (!m_clockDomains.isEmpty()) {
        m_stream << "struct ClockDomain {" << endl;
        m_stream << "    volatile bool state;" << endl;
        m_stream << "    volatile unsigned long last_toggle;" << endl;
        m_stream << "    unsigned long period_us;" << endl;
        m_stream << "    int connected_elements[8];" << endl;
        m_stream << "    int element_count;" << endl;
        m_stream << "};" << endl;
        m_stream << endl;
    }
}

void ImprovedCodeGenerator::generateGlobalVariables()
{
    m_stream << "// ========== Global Variables ========== //" << endl;
    
    // Flip-flop state arrays
    if (!m_flipFlops.isEmpty()) {
        m_stream << QString("FlipFlopState ff_states[%1];").arg(m_flipFlops.size()) << endl;
        
        // Individual flip-flop references for easier access
        for (int i = 0; i < m_flipFlops.size(); ++i) {
            const auto &ff = m_flipFlops[i];
            m_stream << QString("#define %1 ff_states[%2]").arg(ff.instanceName, QString::number(i)) << endl;
        }
        m_stream << endl;
    }
    
    // Clock domain variables
    if (!m_clockDomains.isEmpty()) {
        m_stream << QString("ClockDomain clock_domains[%1];").arg(m_clockDomains.size()) << endl;
        m_stream << endl;
    }
    
    // Input reading variables
    for (const auto &pin : m_inputMap) {
        m_stream << QString("bool %1_val = LOW;").arg(pin.varName) << endl;
    }
    m_stream << endl;
    
    // Combinational logic variables
    for (auto *elm : m_elements) {
        if (!isSequentialElement(elm) && elm->elementGroup() != ElementGroup::Input && 
            elm->elementGroup() != ElementGroup::Output && elm->elementType() != ElementType::Clock) {
            QString varName = getElementInstanceName(elm);
            m_stream << QString("bool %1 = LOW;").arg(varName) << endl;
        }
    }
    m_stream << endl;
}

void ImprovedCodeGenerator::generateInterruptHandlers()
{
    if (m_flipFlops.isEmpty()) {
        return;
    }
    
    m_stream << "// ========== Interrupt Service Routines ========== //" << endl;
    
    for (int i = 0; i < m_flipFlops.size(); ++i) {
        const auto &ff = m_flipFlops[i];
        if (ff.interruptPin >= 0) {
            m_stream << QString("// ISR for %1 (Pin %2)").arg(ff.instanceName, QString::number(ff.interruptPin)) << endl;
            m_stream << QString("void %1_edgeISR() {").arg(ff.instanceName) << endl;
            
            if (m_enableDebouncing) {
                m_stream << "    unsigned long current_time = millis();" << endl;
                m_stream << QString("    if (current_time - %1.last_edge_time < DEBOUNCE_DELAY) {").arg(ff.instanceName) << endl;
                m_stream << "        return; // Ignore bounced edge" << endl;
                m_stream << "    }" << endl;
                m_stream << QString("    %1.last_edge_time = current_time;").arg(ff.instanceName) << endl;
                m_stream << endl;
            }
            
            m_stream << "    // Capture data at exact moment of clock edge" << endl;
            m_stream << QString("    bool current_data = digitalRead(%1);").arg(ff.pins.data) << endl;
            m_stream << QString("    %1.data_captured = current_data;").arg(ff.instanceName) << endl;
            m_stream << QString("    %1.triggered = true;").arg(ff.instanceName) << endl;
            m_stream << "}" << endl;
            m_stream << endl;
        }
    }
    
    // Hardware timer ISRs for clock generation
    if (!m_clockDomains.isEmpty()) {
        m_stream << "// Hardware timer ISR for clock generation" << endl;
        m_stream << "ISR(TIMER1_COMPA_vect) {" << endl;
        m_stream << "    // Toggle clock state for each domain" << endl;
        m_stream << "    for (int i = 0; i < MAX_CLOCK_DOMAINS; i++) {" << endl;
        m_stream << "        clock_domains[i].state = !clock_domains[i].state;" << endl;
        m_stream << "        // Trigger connected elements on rising edge" << endl;
        m_stream << "        if (clock_domains[i].state) {" << endl;
        m_stream << "            for (int j = 0; j < clock_domains[i].element_count; j++) {" << endl;
        m_stream << "                int ff_id = clock_domains[i].connected_elements[j];" << endl;
        m_stream << "                if (ff_id >= 0 && ff_id < MAX_FLIPFLOPS) {" << endl;
        m_stream << "                    // Trigger flip-flop edge detection" << endl;
        m_stream << "                    // Note: This would call the appropriate ISR" << endl;
        m_stream << "                }" << endl;
        m_stream << "            }" << endl;
        m_stream << "        }" << endl;
        m_stream << "    }" << endl;
        m_stream << "}" << endl;
        m_stream << endl;
    }
}

void ImprovedCodeGenerator::categorizeElements()
{
    m_flipFlops.clear();
    m_clockDomains.clear();
    
    int ffCounter = 0;
    int clockCounter = 0;
    
    for (auto *elm : m_elements) {
        QString instanceName = getElementInstanceName(elm);
        
        switch (elm->elementType()) {
        case ElementType::DFlipFlop: {
            FlipFlopDescriptor ff(instanceName, "DFlipFlop", PinAssignment());
            ff.pins.data = getNextAvailablePin();
            ff.pins.clock = getNextAvailablePin(); 
            ff.pins.preset = getNextAvailablePin();
            ff.pins.clear = getNextAvailablePin();
            ff.pins.q_output = getNextAvailablePin();
            ff.pins.q_not_output = getNextAvailablePin();
            
            // Try to assign interrupt pin for clock
            QString interruptPin = getNextInterruptPin();
            if (!interruptPin.isEmpty()) {
                ff.interruptPin = interruptPin.toInt();
                ff.pins.clock = interruptPin;
            }
            
            m_flipFlops.append(ff);
            ffCounter++;
            break;
        }
        
        case ElementType::JKFlipFlop: {
            FlipFlopDescriptor ff(instanceName, "JKFlipFlop", PinAssignment());
            ff.pins.data = getNextAvailablePin(); // J input
            ff.pins.clock = getNextAvailablePin();
            ff.pins.preset = getNextAvailablePin(); // K input  
            ff.pins.clear = getNextAvailablePin();
            ff.pins.q_output = getNextAvailablePin();
            ff.pins.q_not_output = getNextAvailablePin();
            
            QString interruptPin = getNextInterruptPin();
            if (!interruptPin.isEmpty()) {
                ff.interruptPin = interruptPin.toInt();
                ff.pins.clock = interruptPin;
            }
            
            m_flipFlops.append(ff);
            ffCounter++;
            break;
        }
        
        case ElementType::SRFlipFlop: {
            FlipFlopDescriptor ff(instanceName, "SRFlipFlop", PinAssignment());
            ff.pins.data = getNextAvailablePin(); // S input
            ff.pins.clock = getNextAvailablePin();
            ff.pins.preset = getNextAvailablePin(); // R input
            ff.pins.clear = getNextAvailablePin();
            ff.pins.q_output = getNextAvailablePin();
            ff.pins.q_not_output = getNextAvailablePin();
            
            QString interruptPin = getNextInterruptPin();
            if (!interruptPin.isEmpty()) {
                ff.interruptPin = interruptPin.toInt();
                ff.pins.clock = interruptPin;
            }
            
            m_flipFlops.append(ff);
            ffCounter++;
            break;
        }
        
        case ElementType::TFlipFlop: {
            FlipFlopDescriptor ff(instanceName, "TFlipFlop", PinAssignment());
            ff.pins.data = getNextAvailablePin(); // T input
            ff.pins.clock = getNextAvailablePin();
            ff.pins.preset = getNextAvailablePin();
            ff.pins.clear = getNextAvailablePin();
            ff.pins.q_output = getNextAvailablePin();
            ff.pins.q_not_output = getNextAvailablePin();
            
            QString interruptPin = getNextInterruptPin();
            if (!interruptPin.isEmpty()) {
                ff.interruptPin = interruptPin.toInt();
                ff.pins.clock = interruptPin;
            }
            
            m_flipFlops.append(ff);
            ffCounter++;
            break;
        }
        
        case ElementType::Clock: {
            auto *clockElm = qobject_cast<Clock *>(elm);
            if (clockElm) {
                ClockDomain domain(instanceName, clockElm->frequency());
                domain.timerName = QString("Timer%1").arg(clockCounter + 1);
                m_clockDomains.append(domain);
                clockCounter++;
            }
            break;
        }
        
        default:
            // Handle other elements in combinational logic
            break;
        }
    }
}

QString ImprovedCodeGenerator::getNextAvailablePin()
{
    if (m_availablePins.isEmpty()) {
        addError("No more pins available for circuit");
        return "ERROR";
    }
    return m_availablePins.takeFirst();
}

QString ImprovedCodeGenerator::getNextInterruptPin()
{
    if (m_interruptPins.isEmpty()) {
        return QString(); // No interrupt pins available
    }
    QString pin = m_interruptPins.takeFirst();
    // Remove from general available pins too
    m_availablePins.removeOne(pin);
    return pin;
}

QString ImprovedCodeGenerator::sanitizeVariableName(const QString &input) const
{
    return input.toLower().trimmed().replace(' ', '_').replace('-', '_').replace(QRegularExpression("\\W"), "");
}

QString ImprovedCodeGenerator::getElementInstanceName(GraphicElement *elm)
{
    if (m_instanceNames.contains(elm)) {
        return m_instanceNames[elm];
    }
    
    QString baseName = elm->objectName();
    if (baseName.isEmpty()) {
        baseName = "element";
    }
    
    QString instanceName = sanitizeVariableName(baseName) + "_" + QString::number(m_globalCounter++);
    const_cast<ImprovedCodeGenerator*>(this)->m_instanceNames[elm] = instanceName;
    return instanceName;
}

bool ImprovedCodeGenerator::isSequentialElement(GraphicElement *elm) const
{
    ElementType type = elm->elementType();
    return (type == ElementType::DFlipFlop || type == ElementType::JKFlipFlop ||
            type == ElementType::SRFlipFlop || type == ElementType::TFlipFlop ||
            type == ElementType::DLatch || type == ElementType::SRLatch);
}

void ImprovedCodeGenerator::addWarning(const QString &warning)
{
    m_warnings.append(warning);
    qCWarning(zero) << "Arduino CodeGen Warning:" << warning;
}

void ImprovedCodeGenerator::addError(const QString &error)
{
    m_errors.append(error);
    qCCritical(zero) << "Arduino CodeGen Error:" << error;
}

void ImprovedCodeGenerator::assignPins()
{
    // Input elements
    for (auto *elm : m_elements) {
        if (elm->elementType() == ElementType::InputButton || elm->elementType() == ElementType::InputSwitch) {
            QString pin = getNextAvailablePin();
            QString varName = sanitizeVariableName(elm->objectName() + "_input");
            
            EnhancedMappedPin mappedPin(elm, pin, varName, elm->outputPort(), 0);
            mappedPin.requiresDebouncing = (elm->elementType() == ElementType::InputButton);
            m_inputMap.append(mappedPin);
        }
    }
    
    // Output elements  
    for (auto *elm : m_elements) {
        if (elm->elementGroup() == ElementGroup::Output) {
            QString pin = getNextAvailablePin();
            QString varName = sanitizeVariableName(elm->objectName() + "_output");
            
            EnhancedMappedPin mappedPin(elm, pin, varName, elm->inputPort(), 0);
            m_outputMap.append(mappedPin);
        }
    }
}

void ImprovedCodeGenerator::assignInterrupts()
{
    // Interrupts are assigned during categorizeElements()
    // This method can be used for additional interrupt configuration
}

void ImprovedCodeGenerator::assignTimers()
{
    // Timer assignment for clock domains
    for (int i = 0; i < m_clockDomains.size() && i < m_timerNames.size(); ++i) {
        m_clockDomains[i].timerName = m_timerNames[i];
        m_clockDomains[i].usesHardwareTimer = true;
    }
}

void ImprovedCodeGenerator::generateSetupFunction()
{
    m_stream << "// ========== Setup Function ========== //" << endl;
    m_stream << "void setup() {" << endl;
    m_stream << "    // Initialize serial communication for debugging" << endl;
    m_stream << "    Serial.begin(9600);" << endl;
    m_stream << "    Serial.println(\"wiRedPanda Arduino Circuit Starting...\");" << endl;
    m_stream << endl;
    
    // Configure input pins
    m_stream << "    // Configure input pins" << endl;
    for (const auto &pin : m_inputMap) {
        m_stream << QString("    pinMode(%1, INPUT);").arg(pin.varName) << endl;
        if (pin.requiresDebouncing) {
            m_stream << QString("    digitalWrite(%1, HIGH); // Enable internal pull-up").arg(pin.varName) << endl;
        }
    }
    m_stream << endl;
    
    // Configure output pins
    m_stream << "    // Configure output pins" << endl;
    for (const auto &pin : m_outputMap) {
        m_stream << QString("    pinMode(%1, OUTPUT);").arg(pin.varName) << endl;
    }
    m_stream << endl;
    
    // Initialize flip-flop states
    if (!m_flipFlops.isEmpty()) {
        m_stream << "    // Initialize flip-flop states" << endl;
        for (int i = 0; i < m_flipFlops.size(); ++i) {
            const auto &ff = m_flipFlops[i];
            m_stream << QString("    %1.Q = false;").arg(ff.instanceName) << endl;
            m_stream << QString("    %1.Q_not = true;").arg(ff.instanceName) << endl;
            m_stream << QString("    %1.triggered = false;").arg(ff.instanceName) << endl;
            m_stream << QString("    %1.data_captured = false;").arg(ff.instanceName) << endl;
            if (m_enableDebouncing) {
                m_stream << QString("    %1.last_edge_time = 0;").arg(ff.instanceName) << endl;
            }
        }
        m_stream << endl;
        
        // Attach interrupts
        m_stream << "    // Attach interrupt handlers" << endl;
        for (const auto &ff : m_flipFlops) {
            if (ff.interruptPin >= 0) {
                m_stream << QString("    attachInterrupt(digitalPinToInterrupt(%1), %2_edgeISR, RISING);")
                            .arg(QString::number(ff.interruptPin), ff.instanceName) << endl;
            }
        }
        m_stream << endl;
    }
    
    // Initialize hardware timers
    if (!m_clockDomains.isEmpty()) {
        m_stream << "    // Initialize hardware timers for clock generation" << endl;
        m_stream << "    // Configure Timer1 for clock generation" << endl;
        m_stream << "    TCCR1A = 0;" << endl;
        m_stream << "    TCCR1B = (1 << WGM12) | (1 << CS12); // CTC mode, prescaler 256" << endl;
        m_stream << "    OCR1A = 15624; // 1Hz with 16MHz clock and 256 prescaler" << endl;
        m_stream << "    TIMSK1 = (1 << OCIE1A); // Enable timer interrupt" << endl;
        m_stream << endl;
    }
    
    m_stream << "    Serial.println(\"Setup complete. Starting main loop...\");" << endl;
    m_stream << "}" << endl;
    m_stream << endl;
}

void ImprovedCodeGenerator::generateMainLoop()
{
    m_stream << "// ========== Main Loop ========== //" << endl;
    m_stream << "void loop() {" << endl;
    
    // Phase 1: Read all inputs
    m_stream << "    // Phase 1: Read all inputs" << endl;
    for (const auto &pin : m_inputMap) {
        m_stream << QString("    %1_val = digitalRead(%1);").arg(pin.varName) << endl;
    }
    m_stream << endl;
    
    // Phase 2: Update flip-flops that have been triggered
    if (!m_flipFlops.isEmpty()) {
        m_stream << "    // Phase 2: Update edge-triggered flip-flops" << endl;
        for (int i = 0; i < m_flipFlops.size(); ++i) {
            const auto &ff = m_flipFlops[i];
            m_stream << QString("    update_%1();").arg(ff.instanceName) << endl;
        }
        m_stream << endl;
    }
    
    // Phase 3: Update combinational logic
    m_stream << "    // Phase 3: Update combinational logic" << endl;
    generateCombinationalLogic();
    m_stream << endl;
    
    // Phase 4: Write outputs
    m_stream << "    // Phase 4: Write all outputs" << endl;
    for (const auto &pin : m_outputMap) {
        // Find the variable that drives this output
        QString drivingVar = "LOW"; // Default
        // This would need more sophisticated connection tracking
        m_stream << QString("    digitalWrite(%1, %2);").arg(pin.varName, drivingVar) << endl;
    }
    
    m_stream << "}" << endl;
    m_stream << endl;
}

void ImprovedCodeGenerator::generateUtilityFunctions()
{
    if (m_flipFlops.isEmpty()) {
        return;
    }
    
    m_stream << "// ========== Flip-Flop Update Functions ========== //" << endl;
    
    for (const auto &ff : m_flipFlops) {
        generateDFlipFlopCode(ff);
    }
}

void ImprovedCodeGenerator::generateDFlipFlopCode(const FlipFlopDescriptor &ff)
{
    m_stream << QString("// Update function for %1").arg(ff.instanceName) << endl;
    m_stream << QString("void update_%1() {").arg(ff.instanceName) << endl;
    
    m_stream << "    // Read asynchronous control inputs" << endl;
    m_stream << QString("    bool preset = digitalRead(%1);").arg(ff.pins.preset) << endl;
    m_stream << QString("    bool clear = digitalRead(%1);").arg(ff.pins.clear) << endl;
    m_stream << endl;
    
    m_stream << "    // Handle asynchronous preset/clear (active low)" << endl;
    m_stream << "    if (!preset && !clear) {" << endl;
    m_stream << "        // Both active: Clear dominates (reset-dominant)" << endl;
    m_stream << QString("        %1.Q = false;").arg(ff.instanceName) << endl;
    m_stream << QString("        %1.Q_not = true;").arg(ff.instanceName) << endl;
    m_stream << "    } else if (!preset) {" << endl;
    m_stream << "        // Preset active: Set Q=1, Q̄=0" << endl;
    m_stream << QString("        %1.Q = true;").arg(ff.instanceName) << endl;
    m_stream << QString("        %1.Q_not = false;").arg(ff.instanceName) << endl;
    m_stream << "    } else if (!clear) {" << endl;
    m_stream << "        // Clear active: Set Q=0, Q̄=1" << endl;
    m_stream << QString("        %1.Q = false;").arg(ff.instanceName) << endl;
    m_stream << QString("        %1.Q_not = true;").arg(ff.instanceName) << endl;
    m_stream << QString("    } else if (%1.triggered) {").arg(ff.instanceName) << endl;
    m_stream << "        // Clock edge occurred: Use captured data" << endl;
    m_stream << QString("        %1.Q = %1.data_captured;").arg(ff.instanceName) << endl;
    m_stream << QString("        %1.Q_not = !%1.data_captured;").arg(ff.instanceName) << endl;
    m_stream << QString("        %1.triggered = false; // Clear edge flag").arg(ff.instanceName) << endl;
    m_stream << "    }" << endl;
    m_stream << endl;
    
    m_stream << "    // Write outputs" << endl;
    m_stream << QString("    digitalWrite(%1, %2.Q);").arg(ff.pins.q_output, ff.instanceName) << endl;
    m_stream << QString("    digitalWrite(%1, %2.Q_not);").arg(ff.pins.q_not_output, ff.instanceName) << endl;
    m_stream << "}" << endl;
    m_stream << endl;
}

void ImprovedCodeGenerator::generateCombinationalLogic()
{
    // This is a simplified version - would need full connection tracking
    m_stream << "    // Combinational logic updates would go here" << endl;
    m_stream << "    // TODO: Implement based on circuit connections" << endl;
}
