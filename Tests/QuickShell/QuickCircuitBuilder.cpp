// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/QuickCircuitBuilder.h"

#include "App/Core/ItemWithId.h"
#include "App/Element/GraphicElement.h"
#include "App/Simulation/Simulation.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

QuickCircuitBuilder::QuickCircuitBuilder() = default;
QuickCircuitBuilder::~QuickCircuitBuilder() = default;

void QuickCircuitBuilder::registerId(ItemWithId *item)
{
    if (item->id() < 0) {
        item->setId(m_itemRegistry.nextId());
    } else {
        m_itemRegistry.setLastId(item->id());
    }
    m_itemRegistry.registerItem(item);
}

GraphicElement *QuickCircuitBuilder::addElement(GraphicElement *elm)
{
    if (!elm) {
        qFatal("FATAL ERROR in QuickCircuitBuilder::addElement(): Element is nullptr");
    }
    m_elements.append(elm);
    registerId(elm);
    return elm;
}

GraphicElement *QuickCircuitBuilder::addOwnedElement(GraphicElement *elm)
{
    if (!elm) {
        qFatal("FATAL ERROR in QuickCircuitBuilder::addOwnedElement(): Element is nullptr");
    }
    m_elements.append(elm);
    m_owned.emplace_back(elm);
    registerId(elm);
    return elm;
}

QVector<GraphicElement *> QuickCircuitBuilder::elements() const
{
    return m_elements;
}

struct QuickCircuitBuilder::InputPortTraits
{
    static int count(GraphicElement *elm) { return elm->inputSize(); }
    static Port *port(GraphicElement *elm, int idx) { return elm->inputPort(idx); }
    static const char *portType() { return "input"; }
    static const char *noPortsMessage() { return "[No input ports available]"; }
};

struct QuickCircuitBuilder::OutputPortTraits
{
    static int count(GraphicElement *elm) { return elm->outputSize(); }
    static Port *port(GraphicElement *elm, int idx) { return elm->outputPort(idx); }
    static const char *portType() { return "output"; }
    static const char *noPortsMessage() { return "[No output ports available]"; }
};

template<typename PortTraits>
int QuickCircuitBuilder::portByLabelImpl(GraphicElement *element, const QString &label)
{
    if (!element) {
        qFatal("FATAL ERROR in QuickCircuitBuilder::portByLabel(): Element is nullptr");
    }

    const int portCount = PortTraits::count(element);
    for (int i = 0; i < portCount; ++i) {
        auto *port = PortTraits::port(element, i);
        if (port && port->name() == label) {
            return i;
        }
    }

    const QString errorMsg = QString(
        "FATAL ERROR in QuickCircuitBuilder::portByLabel():\n"
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
QString QuickCircuitBuilder::availablePortsImpl(GraphicElement *element) const
{
    if (!element) {
        return "ERROR: Element is nullptr";
    }

    QStringList ports;
    const int portCount = PortTraits::count(element);

    if (portCount == 0) {
        return PortTraits::noPortsMessage();
    }

    for (int i = 0; i < portCount; ++i) {
        auto *port = PortTraits::port(element, i);
        if (port) {
            const QString portName = port->name().isEmpty() ? "(unnamed)" : port->name();
            ports.append(QString("[%1] %2").arg(i).arg(portName));
        } else {
            ports.append(QString("[%1] (port is null)").arg(i));
        }
    }

    return ports.join("\n    ");
}

int QuickCircuitBuilder::inputPortByLabel(GraphicElement *element, const QString &label)
{
    return portByLabelImpl<InputPortTraits>(element, label);
}

int QuickCircuitBuilder::outputPortByLabel(GraphicElement *element, const QString &label)
{
    return portByLabelImpl<OutputPortTraits>(element, label);
}

Connection *QuickCircuitBuilder::connect(GraphicElement *from, int fromPort, GraphicElement *to, int toPort)
{
    if (!from) {
        qFatal("FATAL ERROR in QuickCircuitBuilder::connect(): 'from' element is nullptr");
    }
    if (!to) {
        qFatal("FATAL ERROR in QuickCircuitBuilder::connect(): 'to' element is nullptr");
    }

    auto *fromOutput = from->outputPort(fromPort);
    if (!fromOutput) {
        qFatal("FATAL ERROR in QuickCircuitBuilder::connect():\n"
               "  'from' element %s (type: %s) has no output port %d\n"
               "  Check that the port index is within valid range for this element type",
               qPrintable(from->objectName()),
               from->metaObject()->className(),
               fromPort);
    }

    auto *toInput = to->inputPort(toPort);
    if (!toInput) {
        qFatal("FATAL ERROR in QuickCircuitBuilder::connect():\n"
               "  'to' element %s (type: %s) has no input port %d\n"
               "  Check that the port index is within valid range for this element type",
               qPrintable(to->objectName()),
               to->metaObject()->className(),
               toPort);
    }

    auto *conn = new Connection();
    conn->setStartPort(fromOutput);
    conn->setEndPort(toInput);
    registerId(conn);
    return conn;
}

Connection *QuickCircuitBuilder::connect(GraphicElement *from, const QString &fromLabel, GraphicElement *to, int toPort)
{
    const int fromPort = outputPortByLabel(from, fromLabel);
    return connect(from, fromPort, to, toPort);
}

Connection *QuickCircuitBuilder::connect(GraphicElement *from, int fromPort, GraphicElement *to, const QString &toLabel)
{
    const int toPortIndex = inputPortByLabel(to, toLabel);
    return connect(from, fromPort, to, toPortIndex);
}

Connection *QuickCircuitBuilder::connect(GraphicElement *from, const QString &fromLabel, GraphicElement *to, const QString &toLabel)
{
    const int fromPort = outputPortByLabel(from, fromLabel);
    const int toPort = inputPortByLabel(to, toLabel);
    return connect(from, fromPort, to, toPort);
}

Simulation *QuickCircuitBuilder::simulation()
{
    if (!m_simulation) {
        m_simulation = std::make_unique<Simulation>(this);
    }
    return m_simulation.get();
}

Simulation *QuickCircuitBuilder::initSimulation()
{
    auto *sim = simulation();
    sim->initialize();
    return sim;
}

QList<ItemWithId *> QuickCircuitBuilder::simulationItems() const
{
    QList<ItemWithId *> items;
    items.reserve(m_elements.size());
    for (auto *elm : m_elements) {
        items.append(elm);
    }
    return items;
}

void QuickCircuitBuilder::setMuted(bool)
{
    // No audio device in headless tests.
}
