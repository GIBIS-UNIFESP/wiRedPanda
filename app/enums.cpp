#include "enums.h"

#include <QDataStream>

ElementType Enums::nextElmType(ElementType type) {
    switch (type) {
        // Not and Node
        case ElementType::Not:
            return ElementType::Node;
        case ElementType::Node:
            return ElementType::Not;

        // Gates
        case ElementType::And:
            return ElementType::Or;
        case ElementType::Or:
            return ElementType::Nand;
        case ElementType::Nand:
            return ElementType::Nor;
        case ElementType::Nor:
            return ElementType::Xor;
        case ElementType::Xor:
            return ElementType::Xnor;
        case ElementType::Xnor:
            return ElementType::And;

        // StaticInput and Input
        case ElementType::InputVcc:
            return ElementType::InputGnd;
        case ElementType::InputGnd:
            return ElementType::InputButton;
        case ElementType::InputButton:
            return ElementType::InputSwitch;
        case ElementType::InputSwitch:
            return ElementType::InputRotary;
        case ElementType::InputRotary:
            return ElementType::Clock;
        case ElementType::Clock:
            return ElementType::InputVcc;

        // FipFlops
        case ElementType::DFlipFlop:
            return ElementType::TFlipFlop;
        case ElementType::TFlipFlop:
            return ElementType::DFlipFlop;

        case ElementType::JKFlipFlop:
            return ElementType::SRFlipFlop;
        case ElementType::SRFlipFlop:
            return ElementType::JKFlipFlop;

        // Output
        case ElementType::Buzzer:
            return ElementType::Led;
        case ElementType::Led:
            return ElementType::Buzzer;

        // Other Cases (falltrought)
        default:
            return ElementType::Unknown;
    }
}

ElementType Enums::prevElmType(ElementType type) {
    switch (type) {
        // Not and Node
        case ElementType::Not:
            return ElementType::Node;
        case ElementType::Node:
            return ElementType::Not;

        // Gates
        case ElementType::And:
            return ElementType::Xnor;
        case ElementType::Xnor:
            return ElementType::Xor;
        case ElementType::Xor:
            return ElementType::Nor;
        case ElementType::Nor:
            return ElementType::Nand;
        case ElementType::Nand:
            return ElementType::Or;
        case ElementType::Or:
            return ElementType::And;

        // StaticInput and Input
        case ElementType::InputVcc:
            return ElementType::Clock;
        case ElementType::Clock:
            return ElementType::InputRotary;
        case ElementType::InputRotary:
            return ElementType::InputSwitch;
        case ElementType::InputSwitch:
            return ElementType::InputButton;
        case ElementType::InputButton:
            return ElementType::InputGnd;
        case ElementType::InputGnd:
            return ElementType::InputVcc;

        // FipFlops
        case ElementType::DFlipFlop:
            return ElementType::TFlipFlop;
        case ElementType::TFlipFlop:
            return ElementType::DFlipFlop;

        case ElementType::JKFlipFlop:
            return ElementType::SRFlipFlop;
        case ElementType::SRFlipFlop:
            return ElementType::JKFlipFlop;

        // Output
        case ElementType::Led:
            return ElementType::Buzzer;
        case ElementType::Buzzer:
            return ElementType::Led;

        // Other Cases (falltrought)
        default:
            return ElementType::Unknown;
    }
}

ElementType &operator++(ElementType &type)
{
    return type = static_cast<ElementType>(static_cast<int>(type) + 1);
}

QDataStream &operator>>(QDataStream &stream, ElementType &type)
{
    quint64 temp; stream >> temp;
    type = static_cast<ElementType>(temp);
    return stream;
}

QDataStream &operator<<(QDataStream &stream, const ElementType &type)
{
    stream << static_cast<quint64>(type);
    return stream;
}
