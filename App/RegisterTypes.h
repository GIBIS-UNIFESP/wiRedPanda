// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "And.h"
#include "AudioBox.h"
#include "Buzzer.h"
#include "Clock.h"
#include "Demux.h"
#include "DFlipFlop.h"
#include "Display14.h"
#include "Display16.h"
#include "Display7.h"
#include "DLatch.h"
#include "IC.h"
#include "InputButton.h"
#include "InputGND.h"
#include "InputRotary.h"
#include "InputSwitch.h"
#include "InputVCC.h"
#include "JKFlipFlop.h"
#include "Led.h"
#include "Line.h"
#include "Mux.h"
#include "Nand.h"
#include "Node.h"
#include "Nor.h"
#include "Not.h"
#include "Or.h"
#include "QNEConnection.h"
#include "QNEPort.h"
#include "SRFlipFlop.h"
#include "SRLatch.h"
#include "Text.h"
#include "TFlipFlop.h"
#include "TruthTable.h"
#include "Xnor.h"
#include "Xor.h"

inline void registerTypes() {
    qRegisterMetaType<And>();
    qRegisterMetaType<AudioBox>();
    qRegisterMetaType<Buzzer>();
    qRegisterMetaType<Clock>();
    qRegisterMetaType<DFlipFlop>();
    qRegisterMetaType<DLatch>();
    qRegisterMetaType<Demux>();
    qRegisterMetaType<Display14>();
    qRegisterMetaType<Display16>();
    qRegisterMetaType<Display7>();
    qRegisterMetaType<GraphicElement>();
    qRegisterMetaType<IC>();
    qRegisterMetaType<InputButton>();
    qRegisterMetaType<InputGnd>();
    qRegisterMetaType<InputRotary>();
    qRegisterMetaType<InputSwitch>();
    qRegisterMetaType<InputVcc>();
    qRegisterMetaType<JKFlipFlop>();
    qRegisterMetaType<Led>();
    qRegisterMetaType<Line>();
    qRegisterMetaType<Mux>();
    qRegisterMetaType<Nand>();
    qRegisterMetaType<Node>();
    qRegisterMetaType<Nor>();
    qRegisterMetaType<Not>();
    qRegisterMetaType<Or>();
    qRegisterMetaType<QNEConnection>();
    qRegisterMetaType<QNEInputPort>();
    qRegisterMetaType<QNEOutputPort>();
    qRegisterMetaType<SRFlipFlop>();
    qRegisterMetaType<SRLatch>();
    qRegisterMetaType<TFlipFlop>();
    qRegisterMetaType<Text>();
    qRegisterMetaType<TruthTable>();
    qRegisterMetaType<Xnor>();
    qRegisterMetaType<Xor>();
}

