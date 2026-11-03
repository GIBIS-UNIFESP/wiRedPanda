// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>

#include <QHash>
#include <QSet>
#include <QString>
#include <QVector>

#include "App/Core/Enums.h"

class Element;
class Port;
class Scene;
class WorkSpace;

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
 * Each IC instance is flattened into its own scope. Its definition is read from its file by PandaFile, a reader that
 * shares no code with the app's loader, and built into a scene of its own, so the reference never reads the instance's
 * internal elements, and its pins are the file's pin order. An IC with no file (an embedded one) is not flattened: its
 * outputs read Unknown and it is counted as an error.
 */
class ReferenceNetlist
{
public:
    /// Reads \a scene's wiring. Nothing is kept of the scene, so it may change after construction.
    explicit ReferenceNetlist(const Scene *scene);
    ~ReferenceNetlist();

    ReferenceNetlist(const ReferenceNetlist &) = delete;
    ReferenceNetlist &operator=(const ReferenceNetlist &) = delete;

    /// Returns the driver of \a element's input \a port.
    [[nodiscard]] NetDriver driver(const Element *element, int port) const;

    /// Returns how many nets with two drivers the circuit has, each counted once.
    [[nodiscard]] int errorNets() const { return static_cast<int>(m_errorNets.size()); }

private:
    /// A pin of an IC's inside: the element and the port of it the pin stands for.
    struct PinSlot
    {
        const Element *element = nullptr;
        int port = -1;
    };

    /// An IC definition read from its file: its own scene, and its pins in pin order.
    struct Definition
    {
        std::unique_ptr<WorkSpace> workspace;
        QVector<PinSlot> inputs;   ///< Pin k: the instance's input k drives this output port of an element inside.
        QVector<PinSlot> outputs;  ///< Pin k: the instance's output k reads this input port of an element inside.
    };

    /// One level of the circuit: the top level, or the inside of one IC instance.
    struct Scope
    {
        const Scope *outer = nullptr;       ///< The scope the IC instance stands in; null at the top.
        const Element *instance = nullptr;  ///< The IC instance this scope is the inside of; null at the top.
        QVector<const Element *> elements;  ///< The elements of this scope.
        QHash<QString, QVector<const Element *>> transmitters; ///< The Tx nodes of this scope, by label.
        QVector<PinSlot> inputPins;   ///< The pins of an IC's inside, by pin index.
        QVector<PinSlot> outputPins;  ///< The output pins of an IC's inside, by pin index.
    };

    const Scope *buildScope(const QVector<Element *> &elements, const Element *instance, const Scope *outer,
                            const Definition *definition);
    const Definition *loadDefinition(const QString &path);
    NetDriver resolveInput(const Element *element, int port, const Scope *scope, QSet<const Element *> path);
    NetDriver follow(const Element *element, int outPort, const Scope *scope, QSet<const Element *> path);
    void recordError(const QString &net);

    QString m_contextDir;
    std::vector<std::unique_ptr<Scope>> m_scopes;
    std::vector<std::unique_ptr<Definition>> m_definitions;
    QHash<const Element *, QString> m_icFiles;        ///< Each IC element's definition file, as an absolute path.
    QHash<const Element *, const Scope *> m_insides;  ///< Each IC instance's inside; null when it has none.
    QHash<const Element *, QVector<NetDriver>> m_drivers;
    QSet<QString> m_errorNets;
};
