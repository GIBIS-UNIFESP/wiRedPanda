// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Common/ReferenceNetlist.h"

#include "App/Element/Element.h"
#include "App/Element/IC.h"
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

/// \a pointer as the hexadecimal text an error names its net by.
QString address(const void *pointer)
{
    return QString::number(reinterpret_cast<quintptr>(pointer), 16);
}

} // namespace

ReferenceNetlist::ReferenceNetlist(const Scene *scene)
    : m_contextDir(scene->contextDir())
{
    buildScope(scene->elements(), nullptr, nullptr, nullptr);
    for (const auto &scope : m_scopes) {
        for (const Element *element : scope->elements) {
            QVector<NetDriver> drivers;
            for (int port = 0; port < element->inputSize(); ++port) {
                drivers.append(resolveInput(element, port, scope.get(), {}));
            }
            m_drivers.insert(element, drivers);
        }
    }
}

ReferenceNetlist::~ReferenceNetlist() = default;

NetDriver ReferenceNetlist::driver(const Element *element, const int port) const
{
    return m_drivers.value(element).value(port);
}

const ReferenceNetlist::Scope *ReferenceNetlist::buildScope(const QVector<Element *> &elements, const Element *instance,
                                                            const Scope *outer, const IC *definition)
{
    m_scopes.push_back(std::make_unique<Scope>());
    Scope *scope = m_scopes.back().get();
    scope->outer = outer;
    scope->instance = instance;
    for (const Element *element : elements) {
        scope->elements.append(element);
    }
    if (definition) {
        // The boundary of an IC's inside is one proxy node per pin, and each proxy is one of the inside's elements.
        for (int pin = 0; pin < definition->internalInputs().size(); ++pin) {
            const Element *proxy = definition->internalInputs().at(pin)->element();
            if (!proxy) {
                continue;
            }
            scope->inputPins.insert(proxy, pin);
            if (!scope->elements.contains(proxy)) {
                scope->elements.append(proxy);
            }
        }
        for (const Port *output : definition->internalOutputs()) {
            const Element *proxy = output->element();
            scope->outputPins.append(proxy);
            if (proxy && !scope->elements.contains(proxy)) {
                scope->elements.append(proxy);
            }
        }
    }

    const QVector<const Element *> members = scope->elements;
    for (const Element *element : members) {
        if ((element->elementType() == ElementType::Node) && (WirelessNode::mode(*element) == WirelessMode::Tx)) {
            scope->transmitters[element->label()].append(element);
        }
        if (element->elementType() == ElementType::IC) {
            const IC *inside = loadDefinition(element);
            m_insides.insert(element, buildScope(inside->internalElements(), element, scope, inside));
        }
    }
    return scope;
}

const IC *ReferenceNetlist::loadDefinition(const Element *instance)
{
    const auto *ic = static_cast<const IC *>(instance);
    if (ic->file().isEmpty()) {
        // An embedded IC has no file to load again, so its own inside is read; a file-backed one never is.
        return ic;
    }
    auto definition = std::make_unique<IC>();
    definition->loadFile(ic->file(), m_contextDir);
    m_definitions.push_back(std::move(definition));
    return m_definitions.back().get();
}

NetDriver ReferenceNetlist::resolveInput(const Element *element, const int port, const Scope *scope,
                                         QSet<const Element *> path)
{
    const Port *input = element->inputPort(port);
    const QList<Connection *> &wires = input->connections();
    if (wires.size() > 1) {
        recordError(QStringLiteral("input %1:%2").arg(address(element)).arg(port));
        return {NetKind::Error};
    }
    if (wires.isEmpty()) {
        return {NetKind::Default, nullptr, -1, input->defaultValue()};
    }
    const Connection *wire = wires.first();
    const Element *driver = wire->startPort()->element();
    return follow(driver, outputIndex(driver, wire->startPort()), scope, path);
}

NetDriver ReferenceNetlist::follow(const Element *element, const int outPort, const Scope *scope,
                                   QSet<const Element *> path)
{
    if (path.contains(element)) {
        // A loop of aliases with no node in it reads Unknown.
        return {NetKind::Unknown};
    }
    path.insert(element);

    if (element->elementType() == ElementType::IC) {
        // An IC's output reads the driver of its output boundary, inside the IC.
        const Scope *inside = m_insides.value(element);
        if (!inside || outPort < 0 || outPort >= inside->outputPins.size() || !inside->outputPins.at(outPort)) {
            return {NetKind::Unknown};
        }
        return resolveInput(inside->outputPins.at(outPort), 0, inside, path);
    }

    if (scope->inputPins.contains(element)) {
        // A boundary input inside an IC reads the instance's input pin, as the instance is wired outside it.
        const int pin = scope->inputPins.value(element);
        if (pin >= scope->instance->inputSize()) {
            return {NetKind::Unknown};
        }
        return resolveInput(scope->instance, pin, scope->outer, path);
    }

    if (element->elementType() != ElementType::Node) {
        return {NetKind::Slot, element, outPort, Status::Unknown};
    }

    if (WirelessNode::mode(*element) == WirelessMode::Rx) {
        // A receiver's input takes no wire: one that has one is a net with a wire and the channel as drivers.
        if (!element->inputPort(0)->connections().isEmpty()) {
            recordError(QStringLiteral("receiver %1").arg(address(element)));
            return {NetKind::Error};
        }
        const QVector<const Element *> &transmitters = scope->transmitters.value(element->label());
        if (transmitters.size() > 1) {
            recordError(QStringLiteral("label %1 in %2").arg(element->label(), address(scope)));
            return {NetKind::Error};
        }
        if (transmitters.isEmpty()) {
            return {NetKind::Unknown};
        }
        return resolveInput(transmitters.first(), 0, scope, path);
    }

    // A junction or a transmitter passes on the driver of its input.
    return resolveInput(element, 0, scope, path);
}

void ReferenceNetlist::recordError(const QString &net)
{
    m_errorNets.insert(net);
}
