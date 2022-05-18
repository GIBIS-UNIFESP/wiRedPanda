#pragma once

#include <QObject>
#include <cstdint>

class Enums
{
    Q_GADGET

public:
    Enums() = delete;

    enum class ElementType : uint_fast8_t {
        And = 5,
        Buzzer = 26,
        Clock = 9,
        DFlipFlop = 17,
        DLatch = 15,
        Demux = 25,
        Display = 14,
        Display14 = 27,
        IC = 22,
        InputButton = 1,
        InputGnd = 13,
        InputRotary = 21,
        InputSwitch = 2,
        InputVcc = 12,
        JKFlipFlop = 18,
        JKLatch = 16,
        Led = 3,
        Line = 29,
        Mux = 24,
        Nand = 7,
        Node = 23,
        Nor = 8,
        Not = 4,
        Or = 6,
        SRFlipFlop = 19,
        TFlipFlop = 20,
        Text = 28,
        Unknown = 0,
        Xnor = 11,
        Xor = 10,
    };
    Q_ENUM(ElementType);
};

using ElementType = Enums::ElementType;

ElementType &operator++(ElementType &e);
QDataStream &operator>>(QDataStream &stream, ElementType &type);
QDataStream &operator<<(QDataStream &stream, const ElementType &type);
