// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QHash>
#include <QSet>
#include <QString>
#include <QVector>

#include "App/Core/Enums.h"

class Element;
class Port;
class Scene;

/// How an input's driver resolves: a computing element's output, the port's default, Unknown for a loop of aliases with
/// no node in it or a receiver with no transmitter, or Error for a net with two drivers.
enum class NetKind { Slot, Default, Unknown, Error };

/// The driver of one input, after the aliases are followed.
struct NetDriver
{
    NetKind kind = NetKind::Unknown;
    const Element *element = nullptr; ///< Slot: the computing element that drives the input.
    int port = -1;                    ///< Slot: that element's output index.
    Status status = Status::Unknown;  ///< Default: the port's default.
};

/**
 * \brief The reference's reading of a circuit's wiring: each input's driver, with junctions, transmitters and receivers
 * followed to the computing element that drives it.
 * \details Written to be read, not to be fast. It reads the elements and their wires directly, in one place, so no
 * other reader of the circuit can share an error with it. Aliases are junctions (a buffer with no wireless mode), a
 * transmitter, and a receiver, which reads its scope's transmitter's driver. Two wires into one input, a label on more
 * than one transmitter, and a receiver that a wire also reaches are nets with two drivers and read Error; each such net
 * is counted once. Only the top-level scope is read so far; ICs are flattened by a later part of the plan.
 */
class ReferenceNetlist
{
public:
    /// Reads \a scene's wiring. Nothing is kept of the scene, so it may change after construction.
    explicit ReferenceNetlist(const Scene *scene);

    /// Returns the driver of \a element's input \a port.
    [[nodiscard]] NetDriver driver(const Element *element, int port) const;

    /// Returns how many nets with two drivers the circuit has, each counted once.
    [[nodiscard]] int errorNets() const { return static_cast<int>(m_errorNets.size()); }

private:
    NetDriver resolveInput(const Element *element, int port, QSet<const Element *> path);
    NetDriver follow(const Element *element, int outPort, QSet<const Element *> path);
    void recordError(const QString &net);

    QHash<const Element *, QVector<NetDriver>> m_drivers;
    QHash<QString, QVector<const Element *>> m_transmitters;
    QSet<QString> m_errorNets;
};
