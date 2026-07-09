// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ElementFactory.h"

#include <QCoreApplication>
#include <QMetaEnum>

#include "App/Core/Common.h"
#include "App/Element/ElementMetadata.h"
#include "App/Element/GraphicElement.h"

ElementType ElementFactory::textToType(const QString &text)
{
    // QMetaEnum provides compile-time-safe string<->int conversion for Q_ENUM
    // types; it returns -1 on failure, which is NOT a valid ElementType —
    // map it to Unknown explicitly so callers' `== ElementType::Unknown`
    // checks catch bad input (Unknown is 0, not -1).
    qCDebug(four) << text;
    const int value = QMetaEnum::fromType<ElementType>().keyToValue(text.toLatin1());
    return (value == -1) ? ElementType::Unknown : static_cast<ElementType>(value);
}

QString ElementFactory::typeToText(const ElementType type)
{
    qCDebug(four) << type;

    if (type == ElementType::Unknown) {
        return "UNKNOWN";
    }

    // QVariant leverages the Q_ENUM registration to produce the enum key name.
    return QVariant::fromValue(type).toString();
}

QString ElementFactory::typeToTitleText(const ElementType type)
{
    qCDebug(four) << type;

    if (type == ElementType::Unknown) {
        return tr("MULTIPLE TYPES");
    }

    const auto &meta = ElementMetadataRegistry::metadata(type);
    return QCoreApplication::translate(meta.trContext, meta.titleText);
}

QString ElementFactory::translatedName(const ElementType type)
{
    if (type == ElementType::Unknown) {
        return tr("Unknown");
    }

    const auto &meta = ElementMetadataRegistry::metadata(type);
    return QCoreApplication::translate(meta.trContext, meta.translatedName);
}

QString ElementFactory::description(const ElementType type)
{
    switch (type) {
    case ElementType::InputVcc:    return tr("Constant logic HIGH (1) source.");
    case ElementType::InputGnd:    return tr("Constant logic LOW (0) source.");
    case ElementType::InputButton: return tr("Push button: outputs HIGH only while pressed.");
    case ElementType::InputSwitch: return tr("Toggle switch: click to flip between 0 and 1.");
    case ElementType::InputRotary: return tr("Rotary switch with several selectable positions.");
    case ElementType::Clock:       return tr("Clock: a square wave at a configurable frequency.");
    case ElementType::Led:         return tr("LED: lights up while its input is HIGH.");
    case ElementType::Display7:    return tr("7-segment display for a single digit.");
    case ElementType::Display14:   return tr("14-segment alphanumeric display.");
    case ElementType::Display16:   return tr("16-segment alphanumeric display.");
    case ElementType::Buzzer:      return tr("Buzzer: plays a tone while its input is HIGH.");
    case ElementType::AudioBox:    return tr("Audio box: plays an audio file while its input is HIGH.");
    case ElementType::And:         return tr("AND gate: output is HIGH only when every input is HIGH.");
    case ElementType::Or:          return tr("OR gate: output is HIGH when any input is HIGH.");
    case ElementType::Not:         return tr("NOT gate: inverts its input.");
    case ElementType::Nand:        return tr("NAND gate: LOW only when every input is HIGH.");
    case ElementType::Nor:         return tr("NOR gate: HIGH only when every input is LOW.");
    case ElementType::Xor:         return tr("XOR gate: HIGH when an odd number of inputs are HIGH.");
    case ElementType::Xnor:        return tr("XNOR gate: HIGH when the inputs match.");
    case ElementType::Node:        return tr("Node: a wire junction / branch point.");
    case ElementType::TruthTable:  return tr("Truth table: custom logic you define, output by output.");
    case ElementType::Mux:         return tr("Multiplexer: routes the selected input to the output.");
    case ElementType::Demux:       return tr("Demultiplexer: routes the input to the selected output.");
    case ElementType::DLatch:      return tr("D latch: stores the D input while Enable is HIGH.");
    case ElementType::SRLatch:     return tr("SR latch: a Set/Reset storage element.");
    case ElementType::DFlipFlop:   return tr("D flip-flop: stores D on the active clock edge.");
    case ElementType::JKFlipFlop:  return tr("JK flip-flop: set, reset or toggle on the active clock edge.");
    case ElementType::TFlipFlop:   return tr("T flip-flop: toggles on the active clock edge when T is HIGH.");
    case ElementType::Text:        return tr("Text: a free-text annotation on the canvas.");
    case ElementType::Line:        return tr("Line: a decorative annotation on the canvas.");
    default:                       return {};
    }
}

QPixmap ElementFactory::pixmap(const ElementType type)
{
    if (type == ElementType::Unknown) {
        return {};
    }

    return QPixmap{ElementMetadataRegistry::metadata(type).pixmapPath()};
}

GraphicElement *ElementFactory::buildElement(const ElementType type)
{
    qCDebug(four) << type;

    if (type == ElementType::Unknown) {
        throw PANDACEPTION("Unknown element type: %1", typeToText(type));
    }

    const auto it = instance().m_creatorMap.constFind(type);

    if (it == instance().m_creatorMap.constEnd()) {
        throw PANDACEPTION("Unknown type: %1", typeToText(type));
    }

    return it.value()();
}

void ElementFactory::registerCreator(ElementType type, std::function<GraphicElement *()> creator)
{
    instance().m_creatorMap[type] = std::move(creator);
}

bool ElementFactory::hasCreator(ElementType type)
{
    return instance().m_creatorMap.contains(type);
}
