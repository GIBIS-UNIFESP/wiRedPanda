// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/AudioBox.h"
#include "App/Element/GraphicElements/Buzzer.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/GraphicElements/Demux.h"
#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/Element/GraphicElements/Display14.h"
#include "App/Element/GraphicElements/Display16.h"
#include "App/Element/GraphicElements/Display7.h"
#include "App/Element/GraphicElements/DLatch.h"
#include "App/Element/GraphicElements/InputButton.h"
#include "App/Element/GraphicElements/InputGND.h"
#include "App/Element/GraphicElements/InputRotary.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/InputVCC.h"
#include "App/Element/GraphicElements/JKFlipFlop.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Line.h"
#include "App/Element/GraphicElements/Mux.h"
#include "App/Element/GraphicElements/Nand.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Element/GraphicElements/Nor.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/GraphicElements/Or.h"
#include "App/Element/GraphicElements/SRFlipFlop.h"
#include "App/Element/GraphicElements/SRLatch.h"
#include "App/Element/GraphicElements/Text.h"
#include "App/Element/GraphicElements/TFlipFlop.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/Element/GraphicElements/Xnor.h"
#include "App/Element/GraphicElements/Xor.h"
#include "App/Element/IC.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Nodes/QNEPort.h"

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
