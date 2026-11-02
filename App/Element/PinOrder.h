// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief PinOrder: the order of an IC's pins, as the file it is saved in gives it.
 */

#pragma once

#include <QHash>
#include <QMap>
#include <QPointF>
#include <QStringList>
#include <QVariant>
#include <QVector>

class Element;
class Port;

/**
 * \struct PinRef
 * \brief One pin of an IC: a port of a boundary element, with the pin's key and where its element stood.
 *
 * \details The key is the port's serial id, the number a file names the port by: the element's id and the port's index
 * on it. A file's connections name ports the same way, so the key is the one a saved order can name a pin by.
 */
struct PinRef
{
    Port *port = nullptr;     ///< The port the pin stands for.
    quint64 key = 0;    ///< The port's serial id, in the key space of the caller (the file's or the circuit's).
    QPointF elementPos;       ///< The element's position when the pin was collected.
};

/**
 * \class PinOrder
 * \brief The order of an IC's pins, as a file gives it: one key per pin, in pin order.
 *
 * \details A load snaps every element to the grid, and a snap can move two close pins past one another, so a file's
 * order is taken before the snap. A file that holds no order gets the order its positions give, read before any snap,
 * as the positions always gave it. A pin the saved order does not name, a new one, takes its place by position among
 * the others.
 */
class PinOrder
{
public:
    /// Returns the pins on one side of \a elements: the outputs of the input elements (\a inputs), or the inputs of the
    /// output elements. Each pin's key is its port's serial id in this circuit.
    static QVector<PinRef> pinsOf(const QVector<Element *> &elements, bool inputs);

    /// Returns the pins of one side of \a elements in the order \a metadata saves under \a key, read against the file's
    /// keys in \a portMap, each pin keyed in this circuit. Call it before any element is snapped.
    static QVector<PinRef> loadedOrder(const QVector<Element *> &elements, bool inputs,
                                       const QMap<QString, QVariant> &metadata, const QString &key,
                                       const QHash<quint64, Port *> &portMap);

    /// Returns \a pins in order. The pins \a saved names come first, in its order; the others follow by position, each
    /// before the first pin it precedes. A pin is named by its key.
    static QVector<PinRef> ordered(const QVector<PinRef> &pins, const QVector<quint64> &saved);

    /// Returns the key of each of \a pins, in order.
    static QVector<quint64> keysOf(const QVector<PinRef> &pins);

    /// Returns the order \a metadata holds under \a key, empty when it holds none.
    static QVector<quint64> savedOrder(const QMap<QString, QVariant> &metadata, const QString &key);

    /// Returns \a keys as a metadata value, as text: the metadata reader takes no list of numbers.
    static QStringList toStringList(const QVector<quint64> &keys);

    /// Returns the key \a port has in this circuit: its element's current id and its index.
    static quint64 liveKey(Port *port);

    /// Returns whether \a port1 comes before \a port2 by position: top to bottom by its element, then left to right, and
    /// then by its own position on the element.
    static bool comparePorts(Port *port1, Port *port2);
};
