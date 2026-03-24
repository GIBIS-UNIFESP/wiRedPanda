// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Central enumeration types for element types, groups, and signal status.
 */

#pragma once

#include <QObject>

/**
 * \brief Preferred way to indicate the element types and groups (input, output, etc).
 */
class Enums
{
    Q_GADGET

public:
    Enums() = delete;

    /// Signal status on a port or connection.
    ///
    /// Four-state logic (inspired by VHDL std_logic):
    ///   Unknown  — undriven / floating / not yet resolved (U)
    ///   Inactive — logic low (0)
    ///   Active   — logic high (1)
    ///   Error    — bus conflict / indeterminate (X)
    enum class Status { Unknown = -1, Inactive = 0, Active = 1, Error = 2 };
    Q_ENUM(Status)

    /// Numeric identifiers for every concrete element type.
    enum class ElementType { // last element is 33
        And = 5,          ///< AND gate.
        AudioBox = 31,    ///< Audio playback element.
        Buzzer = 26,      ///< Buzzer output element.
        Clock = 9,        ///< Clock signal generator with configurable frequency.
        DFlipFlop = 17,   ///< D (data) flip-flop — edge-triggered.
        DLatch = 15,      ///< D (data) latch — level-sensitive.
        Demux = 25,       ///< Demultiplexer — routes one input to one of N outputs.
        Display14 = 27,   ///< 14-segment alphanumeric display.
        Display16 = 33,   ///< 16-segment alphanumeric display.
        Display7 = 14,    ///< 7-segment numeric display.
        IC = 22,          ///< Integrated circuit (sub-circuit loaded from a .panda file).
        InputButton = 1,  ///< Momentary push-button input (active while pressed).
        InputGnd = 13,    ///< Constant logic low (GND) source.
        InputRotary = 21, ///< Rotary switch input with multiple positions.
        InputSwitch = 2,  ///< Toggle switch input (latched on/off).
        InputVcc = 12,    ///< Constant logic high (VCC) source.
        JKFlipFlop = 18,  ///< JK flip-flop — edge-triggered.
        JKLatch = 16,     ///< \deprecated JKLatch no longer exists; kept for backward compatibility.
        Led = 3,          ///< LED output indicator.
        Line = 29,        ///< Decorative line (no logical function).
        Mux = 24,         ///< Multiplexer — selects one of N inputs to output.
        Nand = 7,         ///< NAND gate.
        Node = 23,        ///< Wire junction node (also used for wireless Tx/Rx).
        Nor = 8,          ///< NOR gate.
        Not = 4,          ///< NOT gate (inverter).
        Or = 6,           ///< OR gate.
        SRFlipFlop = 19,  ///< SR flip-flop — edge-triggered.
        SRLatch = 32,     ///< SR latch — level-sensitive.
        TFlipFlop = 20,   ///< T (toggle) flip-flop — edge-triggered.
        Text = 28,        ///< Text annotation (no logical function).
        TruthTable = 30,  ///< User-defined truth table element.
        Unknown = 0,      ///< Placeholder for unrecognised or uninitialised types.
        Xnor = 11,        ///< XNOR gate (exclusive NOR).
        Xor = 10,         ///< XOR gate (exclusive OR).
    };
    Q_ENUM(ElementType)

    /// Logical grouping used to organise elements in the UI palette.
    enum class ElementGroup {
        Gate = 4,        ///< Combinational logic gates (AND, OR, NOT, etc.).
        IC = 2,          ///< Integrated circuits (sub-circuit files).
        Input = 3,       ///< User-interactive inputs (buttons, switches, rotary).
        Memory = 5,      ///< Sequential elements (flip-flops, latches).
        Mux = 7,         ///< Multiplexers and demultiplexers.
        Other = 1,       ///< Miscellaneous elements (nodes, lines, text, truth tables).
        Output = 6,      ///< Output indicators (LEDs, displays, buzzers).
        StaticInput = 8, ///< Constant-value inputs (VCC, GND).
        Unknown = 0,     ///< Placeholder for unrecognised groups.
    };
    Q_ENUM(ElementGroup)

    /**
     * \brief Returns the ElementType that follows \a type in the defined sequence.
     * \param type Current element type.
     */
    static ElementType nextElmType(ElementType type);

    /**
     * \brief Returns the ElementType that precedes \a type in the defined sequence.
     * \param type Current element type.
     */
    static ElementType prevElmType(ElementType type);
};

using Status = Enums::Status;
using ElementType = Enums::ElementType;
using ElementGroup = Enums::ElementGroup;

/// Pre-increment operator; advances \a type to the next ElementType in sequence.
ElementType &operator++(ElementType &type);
/// Deserializes an ElementType from \a stream into \a type.
QDataStream &operator>>(QDataStream &stream, ElementType &type);
/// Serializes \a type into \a stream.
QDataStream &operator<<(QDataStream &stream, const ElementType &type);

