// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>
#include <vector>

#include <QHash>
#include <QSet>
#include <QString>
#include <QVector>

#include "App/Core/Enums.h"

class Element;
class IC;
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
 * \brief The reference's reading of a circuit's wiring: each input's driver, with junctions, transmitters, receivers
 * and IC boundaries followed to the computing element that drives it.
 * \details Written to be read, not to be fast. It reads the elements and their wires directly, in one place, so no
 * other reader of the circuit can share an error with it. Aliases are junctions (a buffer with no wireless mode), a
 * transmitter, and a receiver, which reads its scope's transmitter's driver. Two wires into one input, a label on more
 * than one transmitter in one scope, and a receiver that a wire also reaches are nets with two drivers and read Error;
 * each such net is counted once.
 *
 * Each IC instance is flattened into its own scope: the definition is loaded again from the instance's file, so the
 * reference never reads the instance's internal elements, and each instance keeps its own nets, wireless labels and
 * boundary. An IC's output reads the driver of its output boundary inside, and a wire into an inside boundary input
 * reads the instance's input pin outside.
 */
class ReferenceNetlist
{
public:
    /// Reads \a scene's wiring, its ICs loaded from the scene's context directory. Nothing is kept of the scene, so it
    /// may change after construction.
    explicit ReferenceNetlist(const Scene *scene);
    ~ReferenceNetlist();

    ReferenceNetlist(const ReferenceNetlist &) = delete;
    ReferenceNetlist &operator=(const ReferenceNetlist &) = delete;

    /// Returns the driver of \a element's input \a port.
    [[nodiscard]] NetDriver driver(const Element *element, int port) const;

    /// Returns how many nets with two drivers the circuit has, each counted once.
    [[nodiscard]] int errorNets() const { return static_cast<int>(m_errorNets.size()); }

private:
    /// One level of the circuit: the top level, or the inside of one IC instance.
    struct Scope
    {
        const Scope *outer = nullptr;       ///< The scope the IC instance stands in; null at the top.
        const Element *instance = nullptr;  ///< The IC instance this scope is the inside of; null at the top.
        QVector<const Element *> elements;  ///< The elements of this scope, boundary proxies included.
        QHash<QString, QVector<const Element *>> transmitters; ///< The Tx nodes of this scope, by label.
        QHash<const Element *, int> inputPins;  ///< The boundary input proxies of an IC's inside, by pin.
        QVector<const Element *> outputPins;    ///< The boundary output proxies of an IC's inside, by pin.
    };

    const Scope *buildScope(const QVector<Element *> &elements, const Element *instance, const Scope *outer,
                            const IC *definition);
    const IC *loadDefinition(const Element *instance);
    NetDriver resolveInput(const Element *element, int port, const Scope *scope, QSet<const Element *> path);
    NetDriver follow(const Element *element, int outPort, const Scope *scope, QSet<const Element *> path);
    void recordError(const QString &net);

    QString m_contextDir;
    std::vector<std::unique_ptr<Scope>> m_scopes;
    std::vector<std::unique_ptr<IC>> m_definitions;
    QHash<const Element *, const Scope *> m_insides;
    QHash<const Element *, QVector<NetDriver>> m_drivers;
    QSet<QString> m_errorNets;
};
