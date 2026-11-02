// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/PinOrder.h"

#include <algorithm>
#include <utility>

#include <QPointF>
#include <QStringList>

#include "App/Element/Element.h"
#include "App/Wiring/Port.h"

QVector<PinRef> PinOrder::pinsOf(const QVector<Element *> &elements, const bool inputs)
{
    QVector<PinRef> pins;
    const ElementGroup group = inputs ? ElementGroup::Input : ElementGroup::Output;
    for (auto *elm : elements) {
        if (elm->elementGroup() != group) {
            continue;
        }
        const int count = inputs ? elm->outputSize() : elm->inputSize();
        for (int p = 0; p < count; ++p) {
            PinRef pin;
            if (inputs) {
                pin.port = elm->outputPort(p);
            } else {
                pin.port = elm->inputPort(p);
            }
            pin.key = liveKey(pin.port);
            pin.elementPos = elm->pos();
            pins.append(pin);
        }
    }
    return pins;
}

QVector<PinRef> PinOrder::loadedOrder(const QVector<Element *> &elements, const bool inputs,
                                      const QMap<QString, QVariant> &metadata, const QString &key,
                                      const QHash<quint64, Port *> &portMap)
{
    // The file names each pin by its port's serial id, so the pins are matched by the ids the file's ports were given
    QHash<const Port *, quint64> fileKeys;
    for (auto it = portMap.cbegin(); it != portMap.cend(); ++it) {
        fileKeys.insert(it.value(), it.key());
    }
    auto pins = pinsOf(elements, inputs);
    for (auto &pin : pins) {
        pin.key = fileKeys.value(pin.port, pin.key);
    }

    auto result = ordered(pins, savedOrder(metadata, key));
    for (auto &pin : result) {
        pin.key = liveKey(pin.port);
    }
    return result;
}

QVector<PinRef> PinOrder::ordered(const QVector<PinRef> &pins, const QVector<quint64> &saved)
{
    // By position, as a file without an order always gives them: a stable sort keeps the pins of one element in port order
    QVector<PinRef> positional(pins);
    std::stable_sort(positional.begin(), positional.end(),
                     [](const PinRef &a, const PinRef &b) { return comparePorts(a.port, b.port); });
    if (saved.isEmpty()) {
        return positional;
    }

    // Each key names one pin; a key no pin has is skipped, and a key named twice takes its pin once
    QHash<quint64, int> indexOf;
    for (int i = 0; i < positional.size(); ++i) {
        if (!indexOf.contains(positional.at(i).key)) {
            indexOf.insert(positional.at(i).key, i);
        }
    }
    QVector<bool> taken(positional.size(), false);
    QVector<PinRef> result;
    for (const quint64 key : saved) {
        const auto found = indexOf.constFind(key);
        if (found == indexOf.cend() || taken[found.value()]) {
            continue;
        }
        taken[found.value()] = true;
        result.append(positional.at(found.value()));
    }

    // A pin the saved order does not name goes before the first pin it precedes, taking its place by position
    for (int i = 0; i < positional.size(); ++i) {
        if (taken[i]) {
            continue;
        }
        const PinRef &pin = positional.at(i);
        const auto place = std::find_if(result.begin(), result.end(), [&pin](const PinRef &other) {
            return comparePorts(pin.port, other.port);
        });
        result.insert(place, pin);
    }
    return result;
}

QVector<quint64> PinOrder::keysOf(const QVector<PinRef> &pins)
{
    QVector<quint64> keys;
    keys.reserve(pins.size());
    for (const auto &pin : pins) {
        keys.append(pin.key);
    }
    return keys;
}

QVector<quint64> PinOrder::savedOrder(const QMap<QString, QVariant> &metadata, const QString &key)
{
    QVector<quint64> keys;
    const QStringList list = metadata.value(key).toStringList();
    keys.reserve(list.size());
    for (const QString &entry : list) {
        keys.append(entry.toULongLong());
    }
    return keys;
}

QStringList PinOrder::toStringList(const QVector<quint64> &keys)
{
    QStringList list;
    list.reserve(keys.size());
    for (const quint64 key : keys) {
        list.append(QString::number(key));
    }
    return list;
}

quint64 PinOrder::liveKey(Port *port)
{
    // The key a save writes for the port: the element's id as it stands, and the port's index
    return Port::makeSerialId(static_cast<quint64>(port->element()->id()), port->globalIndex());
}

bool PinOrder::comparePorts(Port *port1, Port *port2)
{
    auto *elem1 = port1->element();
    auto *elem2 = port2->element();
    if (!elem1 || !elem2) {
        return false; // LCOV_EXCL_LINE — ElementPorts::addPort() unconditionally calls setElement() right after constructing a port, so every port reachable here already has a non-null owner.
    }

    // Primary sort: top-to-bottom by parent element Y, then left-to-right by X.
    // This gives an intuitive pin order that matches the visual layout in the sub-circuit.
    QPointF p1 = elem1->pos();
    QPointF p2 = elem2->pos();

    if (p1 != p2) {
        return (p1.y() < p2.y()) || (qFuzzyCompare(p1.y(), p2.y()) && (p1.x() < p2.x()));
    }

    // Secondary sort: when two ports share the same parent element position, sort by
    // the port's own local coordinates (left-to-right, top-to-bottom)
    p1 = port1->pos();
    p2 = port2->pos();

    return (p1.x() < p2.x()) || (qFuzzyCompare(p1.x(), p2.x()) && (p1.y() < p2.y()));
}
