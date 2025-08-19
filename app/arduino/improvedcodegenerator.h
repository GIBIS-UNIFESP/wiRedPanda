// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QCoreApplication>
#include <QFile>
#include <QHash>
#include <QTextStream>
#include <QVector>
#include <QStringList>

class GraphicElement;
class QNEPort;

// Resource profile for circuit analysis
struct ResourceProfile {
    int flipflop_count = 0;
    int combinational_elements = 0;
    int clock_domains = 0;
    int required_pins = 0;
    int estimated_ram_bytes = 0;
    int estimated_flash_bytes = 0;
    bool requires_interrupts = false;
    bool requires_timers = false;
    QStringList warnings;
    QStringList errors;
};

// Pin assignment structure
struct PinAssignment {
    QString data;
    QString clock;
    QString preset;
    QString clear;
    QString q_output;
    QString q_not_output;
    QString enable;
    
    PinAssignment() = default;
    PinAssignment(const QString &d, const QString &clk, const QString &q, const QString &qnot)
        : data(d), clock(clk), q_output(q), q_not_output(qnot) {}
};

// Enhanced mapped pin with debouncing info
struct EnhancedMappedPin {
    GraphicElement *element = nullptr;
    QNEPort *port = nullptr;
    QString pin;
    QString varName;
    int portNumber = 0;
    bool requiresDebouncing = false;
    bool isClockInput = false;
    bool isInterruptCapable = false;
    
    EnhancedMappedPin() = default;
    EnhancedMappedPin(GraphicElement *elm, const QString &p, const QString &var, QNEPort *pt, int pNum = 0)
        : element(elm), port(pt), pin(p), varName(var), portNumber(pNum) {}
};

// Flip-flop state management
struct FlipFlopDescriptor {
    QString instanceName;
    QString elementType;
    PinAssignment pins;
    int interruptPin = -1;
    bool usesHardwareTimer = false;
    QString clockDomain;
    
    FlipFlopDescriptor() = default;
    FlipFlopDescriptor(const QString &name, const QString &type, const PinAssignment &p)
        : instanceName(name), elementType(type), pins(p) {}
};

// Clock domain management
struct ClockDomain {
    QString name;
    int frequency_hz = 1;
    QString timerName;
    QStringList connectedElements;
    bool usesHardwareTimer = true;
    bool requiresDebouncing = false;
    
    ClockDomain() = default;
    ClockDomain(const QString &n, int freq) : name(n), frequency_hz(freq) {}
};

class ImprovedCodeGenerator
{
    Q_DECLARE_TR_FUNCTIONS(ImprovedCodeGenerator)

public:
    explicit ImprovedCodeGenerator(const QString &fileName, const QVector<GraphicElement *> &elements);

    // Main generation method
    bool generate();
    
    // Resource analysis
    ResourceProfile analyzeResources() const;
    bool validateResources() const;
    
    // Configuration methods
    void setTargetBoard(const QString &board) { m_targetBoard = board; }
    void setOptimizationLevel(int level) { m_optimizationLevel = level; }
    void setEnableDebouncing(bool enable) { m_enableDebouncing = enable; }
    void setEnableResourceWarnings(bool enable) { m_enableWarnings = enable; }

private:
    // Core generation methods
    void generateIncludes();
    void generateConstants();
    void generateStructures();
    void generateGlobalVariables();
    void generateInterruptHandlers();
    void generateSetupFunction();
    void generateMainLoop();
    void generateUtilityFunctions();
    
    // Element-specific generators
    void generateDFlipFlopCode(const FlipFlopDescriptor &ff);
    void generateJKFlipFlopCode(const FlipFlopDescriptor &ff);
    void generateSRFlipFlopCode(const FlipFlopDescriptor &ff);
    void generateTFlipFlopCode(const FlipFlopDescriptor &ff);
    void generateClockDomainCode(const ClockDomain &domain);
    void generateCombinationalLogic();
    
    // Resource management
    void categorizeElements();
    void assignPins();
    void assignInterrupts();
    void assignTimers();
    void calculateMemoryUsage();
    
    // Pin management
    QString getNextAvailablePin();
    QString getNextInterruptPin();
    bool isPinInterruptCapable(const QString &pin) const;
    
    // Utility methods
    QString sanitizeVariableName(const QString &input) const;
    QString getPortVariableName(QNEPort *port) const;
    QString getElementInstanceName(GraphicElement *elm);
    bool isSequentialElement(GraphicElement *elm) const;
    bool requiresClockInput(GraphicElement *elm) const;
    
    // Validation methods
    void addWarning(const QString &warning);
    void addError(const QString &error);
    bool hasErrors() const { return !m_errors.isEmpty(); }
    
    // Template generation
    QString generateFromTemplate(const QString &templateName, const QHash<QString, QString> &variables) const;
    
    // Member variables
    QFile m_file;
    QTextStream m_stream;
    QStringList m_availablePins;
    QStringList m_interruptPins;
    QStringList m_timerNames;
    QVector<EnhancedMappedPin> m_inputMap;
    QVector<EnhancedMappedPin> m_outputMap;
    QVector<FlipFlopDescriptor> m_flipFlops;
    QVector<ClockDomain> m_clockDomains;
    QHash<QNEPort *, QString> m_varMap;
    QHash<GraphicElement *, QString> m_instanceNames;
    
    const QVector<GraphicElement *> m_elements;
    QString m_targetBoard = "Arduino Uno";
    int m_optimizationLevel = 2;
    bool m_enableDebouncing = true;
    bool m_enableWarnings = true;
    int m_globalCounter = 1;
    
    // Resource tracking
    mutable ResourceProfile m_resourceProfile;
    QStringList m_warnings;
    QStringList m_errors;
    
    // Constants for different Arduino boards
    static const QHash<QString, int> BOARD_RAM_LIMITS;
    static const QHash<QString, int> BOARD_PIN_COUNTS;
    static const QHash<QString, QStringList> BOARD_INTERRUPT_PINS;
};
