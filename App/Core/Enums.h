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

    /// Wireless routing mode for Node elements.
    enum class WirelessMode { None = 0, Tx = 1, Rx = 2 };
    Q_ENUM(WirelessMode)

    /// Numeric identifiers for every concrete element type.
    enum class ElementType { // last element is 33
        And = 5,
        AudioBox = 31,
        Buzzer = 26,
        Clock = 9,
        DFlipFlop = 17,
        DLatch = 15,
        Demux = 25,
        Display14 = 27,
        Display16 = 33,
        Display7 = 14,
        IC = 22,
        InputButton = 1,
        InputGnd = 13,
        InputRotary = 21,
        InputSwitch = 2,
        InputVcc = 12,
        JKFlipFlop = 18,
        JKLatch = 16, // DEPRECATED: JKLatch element no longer exists, kept for backward compatibility
        Led = 3,
        Line = 29,
        Mux = 24,
        Nand = 7,
        Node = 23,
        Nor = 8,
        Not = 4,
        Or = 6,
        SRFlipFlop = 19,
        SRLatch = 32,
        TFlipFlop = 20,
        Text = 28,
        TruthTable = 30,
        Unknown = 0,
        Xnor = 11,
        Xor = 10,
    };
    Q_ENUM(ElementType)

    /// Logical grouping used to organise elements in the UI palette.
    enum class ElementGroup {
        Gate = 4,
        IC = 2,
        Input = 3,
        Memory = 5,
        Mux = 7,
        Other = 1,
        Output = 6,
        StaticInput = 8,
        Unknown = 0,
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
using WirelessMode = Enums::WirelessMode;

/// Pre-increment operator; advances \a type to the next ElementType in sequence.
ElementType &operator++(ElementType &type);
/// Deserializes an ElementType from \a stream into \a type.
QDataStream &operator>>(QDataStream &stream, ElementType &type);
/// Serializes \a type into \a stream.
QDataStream &operator<<(QDataStream &stream, const ElementType &type);

