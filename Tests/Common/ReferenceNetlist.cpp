// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Common/ReferenceNetlist.h"

#include "App/Element/Element.h"
#include "App/Element/WirelessNode.h"
#include "App/Scene/Scene.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

namespace {

/// The index of \a port among \a element's outputs, or -1 when it is none of them.
int outputIndex(const Element *element, const Port *port)
{
    for (int index = 0; index < element->outputSize(); ++index) {
        if (element->outputPort(index) == port) {
            return index;
        }
    }
    return -1;
}

} // namespace

ReferenceNetlist::ReferenceNetlist(const Scene *scene)
{
    const QVector<Element *> elements = scene->elements();
    for (const Element *element : elements) {
        if ((element->elementType() == ElementType::Node) && (WirelessNode::mode(*element) == WirelessMode::Tx)) {
            m_transmitters[element->label()].append(element);
        }
    }
    for (const Element *element : elements) {
        QVector<NetDriver> drivers;
        for (int port = 0; port < element->inputSize(); ++port) {
            drivers.append(resolveInput(element, port, {}));
        }
        m_drivers.insert(element, drivers);
    }
}

NetDriver ReferenceNetlist::driver(const Element *element, const int port) const
{
    return m_drivers.value(element).value(port);
}

NetDriver ReferenceNetlist::resolveInput(const Element *element, const int port, QSet<const Element *> path)
{
    const Port *input = element->inputPort(port);
    const QList<Connection *> &wires = input->connections();
    if (wires.size() > 1) {
        recordError(QStringLiteral("input %1:%2")
                        .arg(QString::number(reinterpret_cast<quintptr>(element), 16))
                        .arg(port));
        return {NetKind::Error};
    }
    if (wires.isEmpty()) {
        return {NetKind::Default, nullptr, -1, input->defaultValue()};
    }
    const Connection *wire = wires.first();
    const Element *driver = wire->startPort()->element();
    return follow(driver, outputIndex(driver, wire->startPort()), path);
}

NetDriver ReferenceNetlist::follow(const Element *element, const int outPort, QSet<const Element *> path)
{
    if (path.contains(element)) {
        // A loop of aliases with no node in it reads Unknown.
        return {NetKind::Unknown};
    }
    path.insert(element);
    if (element->elementType() != ElementType::Node) {
        return {NetKind::Slot, element, outPort, Status::Unknown};
    }

    if (WirelessNode::mode(*element) == WirelessMode::Rx) {
        // A receiver's input takes no wire: one that has one is a net with a wire and the channel as drivers.
        if (!element->inputPort(0)->connections().isEmpty()) {
            recordError(QStringLiteral("receiver %1").arg(element->label()));
            return {NetKind::Error};
        }
        const QVector<const Element *> &transmitters = m_transmitters.value(element->label());
        if (transmitters.size() > 1) {
            recordError(QStringLiteral("label %1").arg(element->label()));
            return {NetKind::Error};
        }
        if (transmitters.isEmpty()) {
            return {NetKind::Unknown};
        }
        return resolveInput(transmitters.first(), 0, path);
    }

    // A junction or a transmitter passes on the driver of its input.
    return resolveInput(element, 0, path);
}

void ReferenceNetlist::recordError(const QString &net)
{
    m_errorNets.insert(net);
}
