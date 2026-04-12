// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/Enums.h"

#include <QDataStream>

namespace {

// Single-source-of-truth morph cycle table.
// nextElmType() follows From→To; prevElmType() follows To→From (inverse lookup).
// Each group is a closed cycle; types in different groups do NOT cross-cycle.
struct MorphStep { ElementType from; ElementType to; };
constexpr MorphStep kMorphCycle[] = {
    // Not / Node (symmetric pair)
    { ElementType::Not,         ElementType::Node        },
    { ElementType::Node,        ElementType::Not         },

    // Gates
    { ElementType::And,         ElementType::Or          },
    { ElementType::Or,          ElementType::Nand        },
    { ElementType::Nand,        ElementType::Nor         },
    { ElementType::Nor,         ElementType::Xor         },
    { ElementType::Xor,         ElementType::Xnor        },
    { ElementType::Xnor,        ElementType::And         },

    // Inputs
    { ElementType::InputVcc,    ElementType::InputGnd    },
    { ElementType::InputGnd,    ElementType::InputButton },
    { ElementType::InputButton, ElementType::InputSwitch },
    { ElementType::InputSwitch, ElementType::InputRotary },
    { ElementType::InputRotary, ElementType::Clock       },
    { ElementType::Clock,       ElementType::InputVcc    },

    // Flip-flops (symmetric pairs)
    { ElementType::DFlipFlop,   ElementType::TFlipFlop   },
    { ElementType::TFlipFlop,   ElementType::DFlipFlop   },
    { ElementType::JKFlipFlop,  ElementType::SRFlipFlop  },
    { ElementType::SRFlipFlop,  ElementType::JKFlipFlop  },

    // Outputs
    { ElementType::Led,         ElementType::Buzzer      },
    { ElementType::Buzzer,      ElementType::AudioBox    },
    { ElementType::AudioBox,    ElementType::Led         },
};

} // namespace

ElementType Enums::nextElmType(ElementType type)
{
    for (const auto &step : kMorphCycle) {
        if (step.from == type) return step.to;
    }
    return ElementType::Unknown;
}

ElementType Enums::prevElmType(ElementType type)
{
    for (const auto &step : kMorphCycle) {
        if (step.to == type) return step.from;
    }
    return ElementType::Unknown;
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

