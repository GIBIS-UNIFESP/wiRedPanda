#pragma once

#include <cstdint>

enum class ElementType : uint_fast8_t {
    And = 5,
    Button = 1,
    Buzzer = 26,
    Clock = 9,
    Demux = 25,
    DFlipFlop = 17,
    Display = 14,
    Display14 = 27,
    DLatch = 15,
    Gnd = 13,
    IC = 22,
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
    Rotary = 21,
    SRFlipFlop = 19,
    Switch = 2,
    Text = 28,
    TFlipFlop = 20,
    Unknown = 0,
    Vcc = 12,
    XNor = 11,
    Xor = 10,
};

inline ElementType &operator++(ElementType &e)
{
    return e = static_cast<ElementType>(static_cast<int>(e) + 1);
}

// TODO: wrap in a class to register enum and be able to convert to/from string
// Q_ENUM(ElementType)
