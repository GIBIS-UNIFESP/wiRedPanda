#ifndef ELEMENTTYPE_H
#define ELEMENTTYPE_H

#include <cstdint>

enum class ElementType : uint_fast8_t {
    UNKNOWN,
    BUTTON,
    SWITCH,
    LED,
    NOT,
    AND,
    OR,
    NAND,
    NOR,
    CLOCK,
    XOR,
    XNOR,
    VCC,
    GND,
    DISPLAY,
    DLATCH,
    JKLATCH,
    DFLIPFLOP,
    JKFLIPFLOP,
    SRFLIPFLOP,
    TFLIPFLOP,
    ROTARY,
    IC,
    NODE,
    MUX,
    DEMUX,
    BUZZER,
    DISPLAY14,
    TEXT,
    LINE,
    REMOTE
};

#endif
