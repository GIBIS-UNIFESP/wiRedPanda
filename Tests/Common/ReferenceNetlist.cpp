// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Common/ReferenceNetlist.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/IC.h"
#include "App/Element/WirelessNode.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/Goldens.h"
#include "Tests/Common/PandaFile.h"
#include "Tests/Common/TestUtils.h"

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
    const QVector<Element *> elements = scene->elements();
    for (const Element *element : elements) {
        if (element->elementType() == ElementType::IC) {
            const auto *ic = static_cast<const IC *>(element);
            if (!ic->isEmbedded() && !ic->file().isEmpty()) {
                m_icFiles.insert(element, QDir(m_contextDir).absoluteFilePath(ic->file()));
            }
        }
    }
    buildScope(elements, nullptr, nullptr, nullptr);
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
                                                            const Scope *outer, const Definition *definition)
{
    m_scopes.push_back(std::make_unique<Scope>());
    Scope *scope = m_scopes.back().get();
    scope->outer = outer;
    scope->instance = instance;
    for (const Element *element : elements) {
        scope->elements.append(element);
    }
    if (definition) {
        scope->inputPins = definition->inputs;
        scope->outputPins = definition->outputs;
    }

    const QVector<const Element *> members = scope->elements;
    for (const Element *element : members) {
        if ((element->elementType() == ElementType::Node) && (WirelessNode::mode(*element) == WirelessMode::Tx)) {
            scope->transmitters[element->label()].append(element);
        }
        if (element->elementType() != ElementType::IC) {
            continue;
        }
        const Definition *inside = nullptr;
        if (!m_icFiles.contains(element)) {
            // An embedded IC has no file to read its definition from, so it is not flattened: it is counted as an error
            recordError(QStringLiteral("embedded ic %1").arg(address(element)));
        } else if (m_loading.contains(m_icFiles.value(element))) {
            // A file that holds itself, through any chain of files, has no inside to read: the loop is counted, not
            // read
            recordError(QStringLiteral("circular ic %1").arg(m_icFiles.value(element)));
        } else {
            inside = loadDefinition(m_icFiles.value(element));
        }
        if (inside) {
            m_loading.append(m_icFiles.value(element));
            const Scope *child = buildScope(inside->workspace->scene()->elements(), element, scope, inside);
            m_loading.removeLast();
            m_insides.insert(element, child);
        } else {
            m_insides.insert(element, nullptr);
        }
    }
    return scope;
}

const ReferenceNetlist::Definition *ReferenceNetlist::loadDefinition(const QString &path)
{
    QFile file(path);
    const std::optional<PandaFile> read = file.open(QIODevice::ReadOnly) ? readPandaFile(file.readAll())
                                                                          : std::nullopt;
    if (!read) {
        recordError(QStringLiteral("definition %1").arg(path));
        return nullptr;
    }

    // The definition is a scene of its own, built from the file's records: each element by its type, id, position and
    // label, its ports counted as the file counts them, and its wires joined by the serial ids the file names them by.
    struct Slot
    {
        Element *element = nullptr;
        int port = -1;
    };
    auto definition = std::make_unique<Definition>();
    definition->workspace = TestUtils::createWorkspace();
    Scene *scene = definition->workspace->scene();
    const QString folder = QFileInfo(path).absolutePath();
    scene->setContextDir(folder);
    CircuitBuilder builder(scene);

    QHash<quint64, Slot> inputSlots;
    QHash<quint64, Slot> outputSlots;
    for (const PandaElement &record : read->elements) {
        Element *element = ElementFactory::buildElement(record.type);
        if (!element) {
            recordError(QStringLiteral("element %1 in %2").arg(record.id).arg(path));
            continue;
        }
        element->setId(record.id);
        element->setPos(record.pos);
        element->setLabel(record.label);
        TestUtils::applyPortCounts(element, static_cast<int>(record.inputKeys.size()),
                                   static_cast<int>(record.outputKeys.size()));
        if ((record.type == ElementType::Node) && record.parameters.contains(QStringLiteral("wirelessMode"))) {
            const auto mode = record.parameters.value(QStringLiteral("wirelessMode")).toInt();
            WirelessNode::setMode(*element, static_cast<WirelessMode>(mode));
        }
        builder.addElement(element);
        for (int port = 0; port < record.inputKeys.size(); ++port) {
            inputSlots.insert(record.inputKeys.at(port), {element, port});
        }
        for (int port = 0; port < record.outputKeys.size(); ++port) {
            outputSlots.insert(record.outputKeys.at(port), {element, port});
        }
        if (record.type == ElementType::IC) {
            m_icFiles.insert(element, QDir(folder).absoluteFilePath(record.icName));
        }
    }

    // A wire runs from an output port to an input port; the file names its start and end by serial id
    for (const PandaConnection &wire : read->connections) {
        Slot from = outputSlots.value(wire.startKey);
        Slot to = inputSlots.value(wire.endKey);
        if (!from.element || !to.element) {
            from = outputSlots.value(wire.endKey);
            to = inputSlots.value(wire.startKey);
        }
        if (!from.element || !to.element) {
            recordError(QStringLiteral("wire in %1").arg(path));
            continue;
        }
        builder.connect(from.element, from.port, to.element, to.port);
    }

    // The pins are the file's, in its pin order: an input pin is an output port of an input element, an output pin an
    // input port of an output element
    for (const quint64 key : read->pinOrder(true)) {
        const Slot slot = outputSlots.value(key);
        definition->inputs.append({slot.element, slot.port});
    }
    for (const quint64 key : read->pinOrder(false)) {
        const Slot slot = inputSlots.value(key);
        definition->outputs.append({slot.element, slot.port});
    }

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
        // An IC's output reads the driver of its output pin, inside the IC.
        const Scope *inside = m_insides.value(element);
        if (!inside || (outPort < 0) || (outPort >= inside->outputPins.size())) {
            return {NetKind::Unknown};
        }
        const PinSlot &pin = inside->outputPins.at(outPort);
        return resolveInput(pin.element, pin.port, inside, path);
    }

    // A pin inside an IC reads the instance's input of the same pin, as the instance is wired outside it.
    for (int pin = 0; pin < scope->inputPins.size(); ++pin) {
        const PinSlot &slot = scope->inputPins.at(pin);
        if ((slot.element == element) && (slot.port == outPort)) {
            if (pin >= scope->instance->inputSize()) {
                return {NetKind::Unknown};
            }
            return resolveInput(scope->instance, pin, scope->outer, path);
        }
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
