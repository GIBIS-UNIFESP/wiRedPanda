// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief An element type's descriptor, and the registry of every type's, filled at start-up.
 */

#pragma once

#include <functional>
#include <optional>
#include <utility>

#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVersionNumber>

#include "App/Core/Enums.h"
#include "App/Element/Parameters.h"
#include "App/Element/PortAttributes.h"

class QDataStream;
struct SerializationContext;

/// What the simulation core computes for an element of the type; the parameters it reads are the descriptor's
/// kernelParameters. Every type but an IC has one kernel; a type the core does not compute has None.
enum class KernelKind : quint8 {
    None,       ///< Not computed: an output, which reads its inputs, or a decoration.
    Instance,   ///< An IC: the core flattens its definition into the circuit.
    Constant,   ///< A constant source: its output ports' defaults.
    Input,      ///< A source the user operates: the levels ElementTypeDescriptor::sourceLevel reads.
    Clock,      ///< A clock source: its frequency, phase delay and lock.
    And,        ///< The gates fold their inputs.
    Or,
    Nand,
    Nor,
    Xor,
    Xnor,
    Not,
    Node,       ///< A buffer, which is also a wireless Tx or Rx by its mode, on the channel its label names; an empty
                ///< label leaves it unpaired.
    Mux,        ///< Data inputs, then select lines.
    Demux,      ///< The data input, then select lines.
    TruthTable, ///< Its table, a column of rows per output.
    DLatch,     ///< The memory kernels sample their data at their control inputs' levels or edges.
    SRLatch,
    DFlipFlop,
    TFlipFlop,
    JKFlipFlop,
    SRFlipFlop,
};

/// Which of a type's operations: what a press does (a click, a trigger key going down), or what its release does.
enum class Operation : quint8 {
    Press,
    Release,
};

/// What an operation does to an element: it sets the element's parameters (Element::operate()).
using ElementOperation = std::function<void(Element &element)>;

/// One slot of a type's appearance: the key a file saves the slot's picture by, and the slot's built-in picture.
struct AppearanceSlot {
    QString key;     ///< The slot's name in a file, unique among the type's slots.
    QString picture; ///< The slot's built-in picture, a resource path.
};

/**
 * \struct ElementTypeDescriptor
 * \brief What one element type is: its group, names, ports, pictures, simulation kernel and capabilities.
 *
 * \details Each element .cpp adds its type's descriptor through its ElementInfo<Type>
 * specialization's describe(); of(), add() and has() are the registry of every
 * type's. All fields are populated at static-initialization time (before
 * QApplication).
 */
struct ElementTypeDescriptor {
    /// Returns the descriptor of \a type. Asserts if \a type has none; a release build warns and returns an empty one.
    static const ElementTypeDescriptor &of(ElementType type);
    /// Adds \a descriptor to the registry (called once per element type at startup).
    static void add(const ElementTypeDescriptor &descriptor);
    /// Returns \c true if \a type has a descriptor.
    static bool has(ElementType type);

    ElementType type = ElementType::Unknown;   ///< Unique element type identifier.
    ElementGroup group = ElementGroup::Unknown; ///< UI palette group this element belongs to.

    // --- Display properties ---

    QString pixmapPath;                  ///< The type's picture, in the Light theme where it has one per theme.
    const char *titleText = nullptr;     ///< Untranslated title shown in the element palette (QT_TRANSLATE_NOOP).
    const char *translatedName = nullptr; ///< Untranslated element name used for tooltips (QT_TRANSLATE_NOOP).
    const char *trContext = nullptr;     ///< Translation context string for QCoreApplication::translate().

    // --- Simulation ---

    KernelKind kernel = KernelKind::None; ///< What the simulation core computes for an element of the type.
    QList<ParamId> kernelParameters;      ///< The parameters the kernel reads, each one in the type's schema.
    bool kernelReadsLabel = false; ///< Whether the kernel reads the element's label, outside the schema: a node's channel.
    /// The history the kernel keeps between evaluations, as it is at power-on: a flip-flop's last clock, then the data
    /// inputs it last sampled. Each element keeps its own (Element::kernelState()), which a reset puts back to this.
    QList<Status> kernelState;

    /// For a KernelKind::Input type: whether output \a port of \a element is on, read from its parameters.
    bool (*sourceLevel)(const Element &element, int port) = nullptr;

    // --- Parameters ---

    QList<ParamSpec> parameters; ///< The type's parameters (its schema), each element holding a value of each.

    // --- Older files ---

    /// The version from which a file holds the type's keyed parameters as a map after the common part, 4.1
    /// (VersionInfo::hasQMapFormat()) unless the type's map came later; an older one holds what readBeforeMap reads, or
    /// nothing. A literal, not Versions', which descriptors built at static initialization cannot rely on.
    QVersionNumber mapSince{4, 1};

    /// Reads what a file older than mapSince holds in place of the map (bare values, Display7's input permutations).
    std::function<void(Element &element, QDataStream &stream, const SerializationContext &context)> readBeforeMap{};

    /// Reads the keys an older map holds in place of a parameter's (the buzzer's note name).
    std::function<void(Element &element, const QMap<QString, QVariant> &map)> readOlderKeys{};

    /// Run at the end of every load of an element of the type, its ports and parameters read: throws a Pandaception for
    /// a state the type cannot hold (a Demux's inputs that disagree with its outputs).
    std::function<void(const Element &element)> checkLoad{};

    /// Applied at the end of every load of the type's parameters, whatever the file held (a switch drives the level it
    /// has, a rotary its selection).
    std::function<void(Element &element)> loaded{};

    // --- Operations ---

    ElementOperation pressOperation{};   ///< What a press does: a switch toggles, a button goes down, a rotary steps.
    ElementOperation releaseOperation{}; ///< What the press's release does: a button goes up.

    // --- Port configuration ---

    quint64 minInputSize = 0;  ///< Minimum number of input ports.
    quint64 maxInputSize = 0;  ///< Maximum number of input ports.
    quint64 minOutputSize = 0; ///< Minimum number of output ports.
    quint64 maxOutputSize = 0; ///< Maximum number of output ports.

    /// The attributes the type sets on input port \a index of \a element (Element::inputAttributes()), or nothing when
    /// the port keeps its own; none, every port keeps its own.
    std::function<std::optional<PortAttributes>(const Element &element, int index)> inputAttributes{};
    /// As inputAttributes, for output port \a index.
    std::function<std::optional<PortAttributes>(const Element &element, int index)> outputAttributes{};
    /// For a type whose inputs follow its outputs (a Demux: its data input and the select lines its outputs need), the
    /// inputs \a outputs take. Setting the outputs sets the inputs with them, and the inputs are never set alone; its
    /// checkLoad refuses a file whose counts disagree.
    int (*inputsForOutputs)(int outputs) = nullptr;
    /// Whether the body is measured from the port counts (a Mux, a Demux, a truth table), so a layout of the ports
    /// measures it again.
    bool bodyFollowsPorts = false;
    /// Whether the picture shows the inputs as they are (a display's lit segments), so a refresh redraws it.
    bool pictureFollowsInputs = false;

    // --- Appearance configuration ---

    /// The type's appearance slots, in the order its presenter picks them by; an element's appearance list starts as
    /// their pictures. A file names each slot it saves by its key.
    QList<AppearanceSlot> appearanceSlots;

    /// The key of the slot each position of an appearance list named in a file written before slots had keys, in
    /// order; empty while that is the slots' keys in order. A slot dropped or moved since keeps its old position here,
    /// so an older file reads as it did, and a dropped slot's key, which no slot has any longer, is skipped.
    QStringList appearancePositions;

    /// For a type with a slot per state (an LED's colours and codes): the slot \a element shows now, the one a custom
    /// appearance replaces alone (Element::applyAppearance()). None: a custom appearance replaces the first slot.
    int (*shownSlot)(const Element &element) = nullptr;
    /// Whether the inputs pick the slot shown (an LED's), so a refresh keeps ParamId::Lit, which the presenter shows,
    /// at shownSlot.
    bool slotFollowsInputs = false;
    /// The states a custom appearance can be chosen for, each its slot and its name (Element::appearanceStates());
    /// none, the one slot, named "Default".
    std::function<QList<std::pair<int, QString>>(const Element &element)> appearanceStates{};
    /// What the element is set to, for the name of an IC's port it is the boundary of (Element::genericProperties()):
    /// an LED's colour. None: nothing.
    std::function<QString(const Element &element)> genericProperties{};

    /// Returns each slot's built-in picture, in the slots' order.
    QStringList defaultAppearances() const;

    /// Returns the index of the slot keyed \a key; -1 if the type has no such slot.
    int appearanceSlot(const QString &key) const;

    /// Returns the key of the slot position \a position of an older file's appearance list names; nothing for a
    /// position past the list.
    std::optional<QString> appearanceKeyAt(qsizetype position) const;

    /// Returns the position of the slot keyed \a key in an older file's appearance list; -1 for a slot added since.
    qsizetype appearancePosition(const QString &key) const;

    // --- Feature flags ---

    bool canChangeAppearance = false; ///< True if the user can choose a custom appearance.
    bool hasTrigger = false;    ///< True if the element supports a keyboard trigger shortcut.
    bool hasLabel = false;      ///< True if the element supports a user-editable label.
    bool rotatesGraphic = true; ///< True if rotating/flipping re-orients the graphic itself; false keeps the icon fixed and only repositions ports.

private:
    /// Returns a reference to the singleton registry map.
    static QMap<ElementType, ElementTypeDescriptor> &registry();
};
