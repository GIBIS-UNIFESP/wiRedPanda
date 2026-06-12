// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Common/TestUtils.h"

#include <QTest>

#include "App/Core/Application.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Nodes/QNEPort.h"
#include "App/Simulation/Simulation.h"

namespace TestUtils {

void setupTestEnvironment()
{
#ifdef Q_OS_LINUX
    qputenv("QT_QPA_PLATFORM", "offscreen");
    // Disable input method plugins — ibus daemon is single-threaded and serializes
    // parallel test startup, causing 8 processes to take 4s instead of 0.06s.
    qputenv("QT_IM_MODULES", "none");
#endif
    Comment::setVerbosity(-1);
    Application::interactiveMode = false;   // Suppress UI dialogs in tests
    Application::migrationEnabled = false;  // Preserve backward-compatibility test files as-is
}

void configureApp()
{
    QCoreApplication::setOrganizationName("GIBIS-UNIFESP");
    QCoreApplication::setApplicationName("wiRedPanda");
    QCoreApplication::setApplicationVersion(APP_VERSION);
}

std::unique_ptr<WorkSpace> createWorkspace()
{
    return std::make_unique<WorkSpace>();
}

QVector<InputSwitch *> createSwitches(int count)
{
    QVector<InputSwitch *> switches(count);
    for (int i = 0; i < count; ++i) {
        switches[i] = new InputSwitch();
    }
    return switches;
}

void setInputValues(const QVector<InputSwitch *> &switches, const QVector<bool> &values)
{
    QVERIFY2(switches.size() == values.size(), "Number of switches must match number of values");
    for (int i = 0; i < switches.size(); ++i) {
        if (values[i]) {
            switches[i]->setOn();
        } else {
            switches[i]->setOff();
        }
    }
}

QString examplesDir()
{
    return QString(QUOTE(CURRENTDIR)) + "/../Examples/";
}

QString backwardCompatibilityDir()
{
    return QString(QUOTE(CURRENTDIR)) + "/../Tests/BackwardCompatibility/";
}

QString cpuComponentsDir()
{
    return QString(QUOTE(CURRENTDIR)) + "/Integration/IC/Components/";
}

QString arduinoExpectedDir()
{
    return QString(QUOTE(CURRENTDIR)) + "/Integration/Arduino/";
}

QString systemVerilogExpectedDir()
{
    return QString(QUOTE(CURRENTDIR)) + "/Integration/SystemVerilog/";
}

bool inputStatus(GraphicElement *elm, int port)
{
    auto *inputPort = elm->inputPort(port);
    if (!inputPort) {
        qFatal("FATAL ERROR in inputStatus():\n"
               "  Attempted to read input port %d from element %s\n"
               "  Element type: %s\n"
               "  Element has ZERO input ports!\n"
               "  This element may only have OUTPUT ports (e.g., InputSwitch)\n"
               "  Use outputStatus() to read output ports, or read downstream elements to verify signal propagation.",
               port, qPrintable(elm->objectName()), elm->metaObject()->className());
    }
    return inputPort->status() == Status::Active;
}

bool outputStatus(GraphicElement *elm, int port)
{
    auto *outputPort = elm->outputPort(port);
    if (!outputPort) {
        qFatal("FATAL ERROR in outputStatus():\n"
               "  Attempted to read output port %d from element %s\n"
               "  Element type: %s\n"
               "  Element has no output port at this index!\n"
               "  Check that the port index is within valid range for this element type",
               port, qPrintable(elm->objectName()), elm->metaObject()->className());
    }
    return outputPort->status() == Status::Active;
}

int countConnections(Scene *scene)
{
    int count = 0;
    auto items = scene->items();
    for (auto *item : std::as_const(items)) {
        if (qgraphicsitem_cast<QNEConnection *>(item)) {
            ++count;
        }
    }
    return count;
}

QList<QNEConnection *> sceneConnections(Scene *scene)
{
    QList<QNEConnection *> connections;
    auto items = scene->items();
    for (auto *item : std::as_const(items)) {
        if (auto *conn = qgraphicsitem_cast<QNEConnection *>(item)) {
            connections.append(conn);
        }
    }
    return connections;
}

void setMultiBitInput(const QVector<InputSwitch *> &inputs, int value)
{
    for (int i = 0; i < inputs.size(); ++i) {
        bool bit = (value >> i) & 1;
        inputs[i]->setOn(bit);
    }
}

void clockCycle(Simulation *simulation, InputSwitch *clk)
{
    if (!simulation) {
        qFatal("FATAL ERROR in clockCycle(): Simulation pointer is nullptr");
    }
    if (!clk) {
        qFatal("FATAL ERROR in clockCycle(): Clock switch pointer is nullptr");
    }
    // Complete clock pulse: ensure rising edge followed by falling edge
    // This guarantees a rising edge occurs regardless of initial state
    clk->setOn(true);   // Rising edge (triggers latch on edge-triggered flip-flops)
    simulation->update();
    clk->setOff();      // Falling edge (return to low for next cycle)
    simulation->update();
}

void initSrc(GraphicElement &elm) { elm.initSimulationVectors(0, 1); }
void initElm(GraphicElement &elm) { elm.initSimulationVectors(elm.inputSize(), elm.outputSize()); }

void clockToggle(Simulation *simulation, InputSwitch *clk)
{
    if (!simulation) {
        qFatal("FATAL ERROR in clockToggle(): Simulation pointer is nullptr");
    }
    if (!clk) {
        qFatal("FATAL ERROR in clockToggle(): Clock switch pointer is nullptr");
    }
    // Single edge toggle for fine-grained clock control
    clk->setOn(!clk->isOn());
    simulation->update();
}
} // namespace TestUtils

// CircuitBuilder implementation

CircuitBuilder::CircuitBuilder(Scene *scene)
    : m_scene(scene)
{
}

// Trait implementations
int CircuitBuilder::InputPortTraits::count(GraphicElement *elm)
{
    return elm->inputSize();
}

QNEPort *CircuitBuilder::InputPortTraits::port(GraphicElement *elm, int idx)
{
    return elm->inputPort(idx);
}

const char *CircuitBuilder::InputPortTraits::portType()
{
    return "input";
}

const char *CircuitBuilder::InputPortTraits::noPortsMessage()
{
    return "[No input ports available]";
}

int CircuitBuilder::OutputPortTraits::count(GraphicElement *elm)
{
    return elm->outputSize();
}

QNEPort *CircuitBuilder::OutputPortTraits::port(GraphicElement *elm, int idx)
{
    return elm->outputPort(idx);
}

const char *CircuitBuilder::OutputPortTraits::portType()
{
    return "output";
}

const char *CircuitBuilder::OutputPortTraits::noPortsMessage()
{
    return "[No output ports available]";
}

// Generic template implementations
template<typename PortTraits>
int CircuitBuilder::portByLabelImpl(GraphicElement *element, const QString &label)
{
    if (!element) {
        qFatal("FATAL ERROR in CircuitBuilder::getPortByLabel(): Element is nullptr");
    }

    int portCount = PortTraits::count(element);
    for (int i = 0; i < portCount; ++i) {
        auto *port = PortTraits::port(element, i);
        if (port && port->name() == label) {
            return i;
        }
    }

    QString errorMsg = QString(
        "FATAL ERROR in CircuitBuilder::getPortByLabel():\n"
        "  %1 port '%2' not found on element '%3'\n"
        "  Available %1 ports:\n    %4"
    ).arg(PortTraits::portType(),
          label,
          element->objectName(),
          availablePortsImpl<PortTraits>(element));

    qFatal("%s", qPrintable(errorMsg));
    return -1;
}

template<typename PortTraits>
QString CircuitBuilder::availablePortsImpl(GraphicElement *element) const
{
    if (!element) {
        return "ERROR: Element is nullptr";
    }

    QStringList ports;
    int portCount = PortTraits::count(element);

    if (portCount == 0) {
        return PortTraits::noPortsMessage();
    }

    for (int i = 0; i < portCount; ++i) {
        auto *port = PortTraits::port(element, i);
        if (port) {
            QString portName = port->name().isEmpty() ? "(unnamed)" : port->name();
            ports.append(QString("[%1] %2").arg(i).arg(portName));
        } else {
            ports.append(QString("[%1] (port is null)").arg(i));
        }
    }

    return ports.join("\n    ");
}

void CircuitBuilder::addElement(GraphicElement *elm)
{
    if (!m_scene) {
        qFatal("FATAL ERROR in CircuitBuilder::addElement(): Scene is nullptr");
    }
    if (!elm) {
        qFatal("FATAL ERROR in CircuitBuilder::addElement(): Element is nullptr");
    }
    m_scene->addItem(elm);
}

QNEConnection *CircuitBuilder::connect(GraphicElement *from, int fromPort,
                                        GraphicElement *to, int toPort)
{
    if (!m_scene) {
        qFatal("FATAL ERROR in CircuitBuilder::connect(): Scene is nullptr");
    }
    if (!from) {
        qFatal("FATAL ERROR in CircuitBuilder::connect(): 'from' element is nullptr");
    }
    if (!to) {
        qFatal("FATAL ERROR in CircuitBuilder::connect(): 'to' element is nullptr");
    }

    auto *fromOutput = from->outputPort(fromPort);
    if (!fromOutput) {
        qFatal("FATAL ERROR in CircuitBuilder::connect():\n"
               "  'from' element %s (type: %s) has no output port %d\n"
               "  Check that the port index is within valid range for this element type",
               qPrintable(from->objectName()),
               from->metaObject()->className(),
               fromPort);
    }

    auto *toInput = to->inputPort(toPort);
    if (!toInput) {
        qFatal("FATAL ERROR in CircuitBuilder::connect():\n"
               "  'to' element %s (type: %s) has no input port %d\n"
               "  Check that the port index is within valid range for this element type",
               qPrintable(to->objectName()),
               to->metaObject()->className(),
               toPort);
    }

    auto *conn = new QNEConnection();
    conn->setStartPort(fromOutput);
    conn->setEndPort(toInput);
    m_scene->addItem(conn);
    return conn;
}

Simulation *CircuitBuilder::simulation() const
{
    if (!m_scene) {
        qWarning("WARNING in CircuitBuilder::simulation(): Scene is nullptr");
        return nullptr;
    }
    auto *sim = m_scene->simulation();
    if (!sim) {
        qWarning("WARNING in CircuitBuilder::simulation(): Scene has no simulation");
    }
    return sim;
}

Simulation *CircuitBuilder::initSimulation()
{
    if (!m_scene) {
        qFatal("FATAL ERROR in CircuitBuilder::initSimulation(): Scene is nullptr");
    }
    auto *sim = m_scene->simulation();
    if (!sim) {
        qFatal("FATAL ERROR in CircuitBuilder::initSimulation(): Scene has no simulation");
    }
    sim->initialize();
    return sim;
}

int CircuitBuilder::inputPortByLabel(GraphicElement *element, const QString &label)
{
    return portByLabelImpl<InputPortTraits>(element, label);
}

int CircuitBuilder::outputPortByLabel(GraphicElement *element, const QString &label)
{
    return portByLabelImpl<OutputPortTraits>(element, label);
}

QString CircuitBuilder::availableInputPorts(GraphicElement *element) const
{
    return availablePortsImpl<InputPortTraits>(element);
}

QString CircuitBuilder::availableOutputPorts(GraphicElement *element) const
{
    return availablePortsImpl<OutputPortTraits>(element);
}

QNEConnection *CircuitBuilder::connect(GraphicElement *from, const QString &fromLabel,
                                        GraphicElement *to, int toPort)
{
    int fromPort = outputPortByLabel(from, fromLabel);
    return connect(from, fromPort, to, toPort);
}

QNEConnection *CircuitBuilder::connect(GraphicElement *from, int fromPort,
                                        GraphicElement *to, const QString &toLabel)
{
    int toPortIndex = inputPortByLabel(to, toLabel);
    return connect(from, fromPort, to, toPortIndex);
}

QNEConnection *CircuitBuilder::connect(GraphicElement *from, const QString &fromLabel,
                                        GraphicElement *to, const QString &toLabel)
{
    int fromPort = outputPortByLabel(from, fromLabel);
    int toPort = inputPortByLabel(to, toLabel);
    return connect(from, fromPort, to, toPort);
}
