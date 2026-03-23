// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>

#include <QApplication>
#include <QTest>

#include "App/Core/Common.h"
#include "App/Element/GraphicElement.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"

class InputSwitch;
class QNEConnection;
class Simulation;

/**
 * @brief Macro for accessing test data directory
 */
#define TEST_DATA_DIR (QString(QUOTE(CURRENTDIR)) + "/../Examples/")

#define QUOTE(string) _QUOTE(string)
#define _QUOTE(string) #string

namespace TestUtils {

/**
 * @brief Set up the test environment (headless mode on Linux, register types)
 */
void setupTestEnvironment();

/**
 * @brief Configure QApplication settings
 */
void configureApp();

/**
 * @brief Create a workspace with an initialized scene
 * @return Unique pointer to the workspace
 */
std::unique_ptr<WorkSpace> createWorkspace();

/**
 * @brief Create multiple input switches
 * @param count Number of switches to create
 * @return Vector of input switch pointers
 */
QVector<InputSwitch *> createSwitches(int count);

/**
 * @brief Set values on multiple input switches
 * @param switches Vector of switches to set
 * @param values Boolean values to set (must match switches size)
 */
void setInputValues(const QVector<InputSwitch *> &switches, const QVector<bool> &values);

/**
 * @brief Get the path to the examples directory
 * @return QString path to examples
 */
QString examplesDir();

/**
 * @brief Get the path to the backward compatibility test directory
 * @return QString path to Tests/BackwardCompatibility/
 */
QString backwardCompatibilityDir();

/**
 * @brief Get the path to CPU test components directory
 * @return QString path to Integration/IC/Components/
 */
QString cpuComponentsDir();

/**
 * @brief Get the path to expected Arduino output directory
 * @return QString path to Integration/Arduino/
 */
QString arduinoExpectedDir();

/**
 * @brief Delete all items in a container
 * @param container Container of pointers to delete
 */
template<typename Container>
void deleteAll(Container &container)
{
    qDeleteAll(container);
    container.clear();
}

/**
 * @brief Helper to get input status as bool
 * @param elm Element to check
 * @param port Port index (default 0)
 * @return true if port status is Active
 */
bool getInputStatus(GraphicElement *elm, int port = 0);

/**
 * @brief Helper to get output status as bool
 * @param elm Element to check
 * @param port Port index (default 0)
 * @return true if port status is Active
 */
bool getOutputStatus(GraphicElement *elm, int port = 0);

/**
 * @brief Count the number of connections in a scene
 * @param scene Scene to count connections in
 * @return Number of QNEConnection items in the scene
 */
int countConnections(Scene *scene);

/**
 * @brief Get all connections in a scene
 * @param scene Scene to get connections from
 * @return List of all QNEConnection items in the scene
 */
QList<QNEConnection *> getConnections(Scene *scene);

/**
 * @brief Set multiple input bits from an integer value
 *
 * Useful for sequential and memory tests where you need to set N bits at once.
 * Example: setMultiBitInput({sw0, sw1, sw2, sw3}, 0b1010) sets:
 *   sw0=0, sw1=1, sw2=0, sw3=1 (bit order matches array order)
 *
 * @param inputs Vector of InputSwitch pointers in bit order (index 0 = LSB)
 * @param value Integer value to decompose into bits
 */
void setMultiBitInput(const QVector<InputSwitch *> &inputs, int value);

/**
 * @brief Read multiple output bits from graphic elements
 *
 * Reads bit values from multiple elements' input ports and combines into
 * a single integer. Useful for reading results from multi-bit outputs.
 * Example: readMultiBitOutput({led0, led1, led2, led3}, 0) reads 4 bits
 *   Result bit pattern: led3|led2|led1|led0 (MSB to LSB)
 *
 * @param elements Vector of GraphicElement pointers in bit order (index 0 = LSB)
 * @param port Input port index to read from each element (default 0)
 * @return Combined integer value from all bits
 */
template<typename T>
inline int readMultiBitOutput(const QVector<T *> &elements, int port = 0)
{
    int result = 0;
    for (int i = 0; i < elements.size(); ++i) {
        if (!elements[i]) {
            qFatal("FATAL ERROR in readMultiBitOutput(): Element at index %d is nullptr", i);
        }
        if (getInputStatus(static_cast<GraphicElement *>(elements[i]), port)) {
            result |= (1 << i);
        }
    }
    return result;
}

/**
 * @brief Execute a complete clock pulse (rising edge + falling edge)
 *
 * For sequential logic testing, this performs a complete clock cycle:
 * LOW → HIGH (rising edge, triggers latch) → LOW (falling edge, ready for next)
 *
 * Usage:
 *   InputSwitch clk;
 *   clockCycle(sim, &clk);  // Complete pulse: 0→1→0
 *   // Flip-flop has latched on the rising edge
 *
 * @param simulation Simulation to run
 * @param clk Clock input switch
 */
void clockCycle(Simulation *simulation, InputSwitch *clk);

/**
 * @brief Toggle the clock signal once (single edge)
 *
 * For cases that need fine-grained clock control (multi-phase clocking,
 * level-sensitive latches, testing specific clock states).
 *
 * Usage:
 *   InputSwitch clk;  // starts LOW
 *   clockToggle(sim, &clk);  // LOW → HIGH (rising edge)
 *   // ... do something while clock is HIGH ...
 *   clockToggle(sim, &clk);  // HIGH → LOW (falling edge)
 *
 * @param simulation Simulation to run
 * @param clk Clock input switch to toggle
 */
void clockToggle(Simulation *simulation, InputSwitch *clk);
} // namespace TestUtils

/**
 * @brief Helper class for building test circuits with less boilerplate
 *
 * Usage:
 *   WorkSpace workspace;
 *   CircuitBuilder builder(workspace.scene());
 *
 *   InputSwitch sw1, sw2;
 *   And andGate;
 *   Led led;
 *
 *   builder.add(&sw1, &sw2, &andGate, &led);
 *   builder.connect(&sw1, 0, &andGate, 0);
 *   builder.connect(&sw2, 0, &andGate, 1);
 *   builder.connect(&andGate, 0, &led, 0);
 */
class CircuitBuilder
{
public:
    explicit CircuitBuilder(Scene *scene);

    /**
     * @brief Add a single element to the scene
     */
    void addElement(GraphicElement *elm);

    /**
     * @brief Add multiple elements to the scene (variadic template)
     */
    template<typename... Args>
    void add(Args *...elements)
    {
        (addElement(elements), ...);
    }

    /**
     * @brief Create a connection between two elements and add to scene
     * @param from Source element
     * @param fromPort Output port index
     * @param to Destination element
     * @param toPort Input port index
     * @return Pointer to created connection
     */
    QNEConnection *connect(GraphicElement *from, int fromPort,
                           GraphicElement *to, int toPort);

    /**
     * @brief Get the scene
     */
    Scene *scene() const { return m_scene; }

    /**
     * @brief Get the simulation from the scene
     */
    Simulation *simulation() const;

    /**
     * @brief Initialize and return simulation
     */
    Simulation *initSimulation();

    /**
     * @brief Create a connection using output port label
     * @param from Source element
     * @param fromLabel Output port label on source element
     * @param to Destination element
     * @param toPort Input port index on destination element
     * @return Pointer to created connection
     */
    QNEConnection *connect(GraphicElement *from, const QString &fromLabel,
                           GraphicElement *to, int toPort);

    /**
     * @brief Create a connection using input port label
     * @param from Source element
     * @param fromPort Output port index on source element
     * @param to Destination element
     * @param toLabel Input port label on destination element
     * @return Pointer to created connection
     */
    QNEConnection *connect(GraphicElement *from, int fromPort,
                           GraphicElement *to, const QString &toLabel);

    /**
     * @brief Create a connection using both port labels
     * @param from Source element
     * @param fromLabel Output port label on source element
     * @param to Destination element
     * @param toLabel Input port label on destination element
     * @return Pointer to created connection
     */
    QNEConnection *connect(GraphicElement *from, const QString &fromLabel,
                           GraphicElement *to, const QString &toLabel);

private:
    Scene *m_scene;

    /**
     * @brief Port direction traits for template-based port access
     */
    struct InputPortTraits
    {
        static int getCount(GraphicElement *elm);
        static class QNEPort *getPort(GraphicElement *elm, int idx);
        static const char *getPortType();
        static const char *getNoPortsMessage();
    };

    struct OutputPortTraits
    {
        static int getCount(GraphicElement *elm);
        static class QNEPort *getPort(GraphicElement *elm, int idx);
        static const char *getPortType();
        static const char *getNoPortsMessage();
    };

    /**
     * @brief Generic implementation for finding port by label
     * @tparam PortTraits Traits specifying input or output port access
     */
    template<typename PortTraits>
    int getPortByLabelImpl(GraphicElement *element, const QString &label);

    /**
     * @brief Generic implementation for getting available ports list
     * @tparam PortTraits Traits specifying input or output port access
     */
    template<typename PortTraits>
    QString getAvailablePortsImpl(GraphicElement *element) const;

    /**
     * @brief Find input port index by label (for IC elements)
     * @param element The IC element
     * @param label Port label to search for
     * @return Port index (0-based)
     * @throws std::runtime_error if port not found
     */
    int getInputPortByLabel(GraphicElement *element, const QString &label);

    /**
     * @brief Find output port index by label (for IC elements)
     * @param element The IC element
     * @param label Port label to search for
     * @return Port index (0-based)
     * @throws std::runtime_error if port not found
     */
    int getOutputPortByLabel(GraphicElement *element, const QString &label);

    /**
     * @brief Get list of available input ports as formatted string for error messages
     * @param element The IC element
     * @return Formatted string of [index] label pairs
     */
    QString getAvailableInputPorts(GraphicElement *element) const;

    /**
     * @brief Get list of available output ports as formatted string for error messages
     * @param element The IC element
     * @return Formatted string of [index] label pairs
     */
    QString getAvailableOutputPorts(GraphicElement *element) const;
};

