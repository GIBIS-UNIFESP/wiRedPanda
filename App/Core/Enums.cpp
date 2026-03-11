// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/Enums.h"

#include <QDataStream>

// nextElmType / prevElmType implement per-group circular "morph" cycles so the
// user can press a key to rotate through related element types (e.g. AND→OR→NAND…).
// Types in different groups are intentionally kept separate and do NOT cycle into
// each other; Unknown is returned as a sentinel for unhandled types.
ElementType Enums::nextElmType(ElementType type)
{
    switch (type) {
        // Not and Node
        case ElementType::Not:         return ElementType::Node;
        case ElementType::Node:        return ElementType::Not;

        // Gates
        case ElementType::And:         return ElementType::Or;
        case ElementType::Or:          return ElementType::Nand;
        case ElementType::Nand:        return ElementType::Nor;
        case ElementType::Nor:         return ElementType::Xor;
        case ElementType::Xor:         return ElementType::Xnor;
        case ElementType::Xnor:        return ElementType::And;

        // StaticInput and Input
        case ElementType::InputVcc:    return ElementType::InputGnd;
        case ElementType::InputGnd:    return ElementType::InputButton;
        case ElementType::InputButton: return ElementType::InputSwitch;
        case ElementType::InputSwitch: return ElementType::InputRotary;
        case ElementType::InputRotary: return ElementType::Clock;
        case ElementType::Clock:       return ElementType::InputVcc;

        // FipFlops
        case ElementType::DFlipFlop:   return ElementType::TFlipFlop;
        case ElementType::TFlipFlop:   return ElementType::DFlipFlop;

        case ElementType::JKFlipFlop:  return ElementType::SRFlipFlop;
        case ElementType::SRFlipFlop:  return ElementType::JKFlipFlop;

        // Output
        case ElementType::Led:         return ElementType::Buzzer;
        case ElementType::Buzzer:      return ElementType::AudioBox;
        case ElementType::AudioBox:    return ElementType::Led;

        default:                       return ElementType::Unknown;
    }
}

ElementType Enums::prevElmType(ElementType type)
{
    switch (type) {
        // Not and Node
        case ElementType::Not:         return ElementType::Node;
        case ElementType::Node:        return ElementType::Not;

        // Gates
        case ElementType::And:         return ElementType::Xnor;
        case ElementType::Xnor:        return ElementType::Xor;
        case ElementType::Xor:         return ElementType::Nor;
        case ElementType::Nor:         return ElementType::Nand;
        case ElementType::Nand:        return ElementType::Or;
        case ElementType::Or:          return ElementType::And;

        // StaticInput and Input
        case ElementType::InputVcc:    return ElementType::Clock;
        case ElementType::Clock:       return ElementType::InputRotary;
        case ElementType::InputRotary: return ElementType::InputSwitch;
        case ElementType::InputSwitch: return ElementType::InputButton;
        case ElementType::InputButton: return ElementType::InputGnd;
        case ElementType::InputGnd:    return ElementType::InputVcc;

        // FipFlops
        case ElementType::DFlipFlop:   return ElementType::TFlipFlop;
        case ElementType::TFlipFlop:   return ElementType::DFlipFlop;

        case ElementType::JKFlipFlop:  return ElementType::SRFlipFlop;
        case ElementType::SRFlipFlop:  return ElementType::JKFlipFlop;

        // Output
        case ElementType::Led:         return ElementType::AudioBox;
        case ElementType::AudioBox:    return ElementType::Buzzer;
        case ElementType::Buzzer:      return ElementType::Led;

        default:                       return ElementType::Unknown;
    }
}

// Pre-increment used to iterate over all enum values sequentially (e.g. in loops
// that walk the full ElementType range); does not wrap around.
ElementType &operator++(ElementType &type)
{
    return type = static_cast<ElementType>(static_cast<int>(type) + 1);
}

// Serialise as quint64 so the on-disk representation is stable even if the enum
// underlying type changes or new values are inserted in future versions.
QDataStream &operator>>(QDataStream &stream, ElementType &type)
{
    // Read into a temporary so that a partially-read stream doesn't leave 'type'
    // in an indeterminate state if the cast would produce an out-of-range value
    quint64 temp; stream >> temp;
    type = static_cast<ElementType>(temp);
    return stream;
}

QDataStream &operator<<(QDataStream &stream, const ElementType &type)
{
    stream << static_cast<quint64>(type);
    return stream;
}
