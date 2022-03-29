// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "elementfactory.h"

#include "and.h"
#include "buzzer.h"
#include "clock.h"
#include "common.h"
#include "demux.h"
#include "dflipflop.h"
#include "display.h"
#include "display_14.h"
#include "dlatch.h"
#include "graphicelement.h"
#include "ic.h"
#include "inputbutton.h"
#include "inputgnd.h"
#include "inputrotary.h"
#include "inputswitch.h"
#include "inputvcc.h"
#include "itemwithid.h"
#include "jkflipflop.h"
#include "jklatch.h"
#include "led.h"
#include "line.h"
#include "mux.h"
#include "nand.h"
#include "node.h"
#include "nor.h"
#include "not.h"
#include "or.h"
#include "qneconnection.h"
#include "srflipflop.h"
#include "text.h"
#include "tflipflop.h"
#include "xnor.h"
#include "xor.h"

#include <QDebug>

ElementFactory *ElementFactory::instance = new ElementFactory();

int ElementFactory::getLastId() const
{
    return m_lastId;
}

ElementType ElementFactory::textToType(QString text)
{
    qCDebug(four) << "Creating Element Type conversion text to type.";
    text = text.toUpper();

    return text == "AND"        ? ElementType::And
         : text == "BUTTON"     ? ElementType::Button
         : text == "BUZZER"     ? ElementType::Buzzer
         : text == "CLOCK"      ? ElementType::Clock
         : text == "DEMUX"      ? ElementType::Demux
         : text == "DFLIPFLOP"  ? ElementType::DFlipFlop
         : text == "DISPLAY"    ? ElementType::Display
         : text == "DISPLAY14"  ? ElementType::Display14
         : text == "DLATCH"     ? ElementType::DLatch
         : text == "GND"        ? ElementType::Gnd
         : text == "IC"         ? ElementType::IC
         : text == "JKFLIPFLOP" ? ElementType::JKFlipFlop
         : text == "JKLATCH"    ? ElementType::JKLatch
         : text == "LED"        ? ElementType::Led
         : text == "LINE"       ? ElementType::Line
         : text == "MUX"        ? ElementType::Mux
         : text == "NAND"       ? ElementType::Nand
         : text == "NODE"       ? ElementType::Node
         : text == "NOR"        ? ElementType::Nor
         : text == "NOT"        ? ElementType::Not
         : text == "OR"         ? ElementType::Or
         : text == "ROTARY"     ? ElementType::Rotary
         : text == "SRFLIPFLOP" ? ElementType::SRFlipFlop
         : text == "SWITCH"     ? ElementType::Switch
         : text == "TEXT"       ? ElementType::Text
         : text == "TFLIPFLOP"  ? ElementType::TFlipFlop
         : text == "VCC"        ? ElementType::Vcc
         : text == "XNOR"       ? ElementType::XNor
         : text == "XOR"        ? ElementType::Xor
                                : ElementType::Unknown;
}

QString ElementFactory::typeToText(ElementType type) // TODO: replace with QMetaEnum
{
    qCDebug(four) << "Creating Element Type conversion type to text.";
    switch (type) {
    case ElementType::And:        return "AND";
    case ElementType::Button:     return "BUTTON";
    case ElementType::Buzzer:     return "BUZZER";
    case ElementType::Clock:      return "CLOCK";
    case ElementType::DFlipFlop:  return "DFLIPFLOP";
    case ElementType::DLatch:     return "DLATCH";
    case ElementType::Demux:      return "DEMUX";
    case ElementType::Display14:  return "DISPLAY14";
    case ElementType::Display:    return "DISPLAY";
    case ElementType::Gnd:        return "GND";
    case ElementType::IC:         return "IC";
    case ElementType::JKFlipFlop: return "JKFLIPFLOP";
    case ElementType::JKLatch:    return "JKLATCH";
    case ElementType::Led:        return "LED";
    case ElementType::Line:       return "LINE";
    case ElementType::Mux:        return "MUX";
    case ElementType::Nand:       return "NAND";
    case ElementType::Node:       return "NODE";
    case ElementType::Nor:        return "NOR";
    case ElementType::Not:        return "NOT";
    case ElementType::Or:         return "OR";
    case ElementType::Rotary:     return "ROTARY";
    case ElementType::SRFlipFlop: return "SRFLIPFLOP";
    case ElementType::Switch:     return "SWITCH";
    case ElementType::TFlipFlop:  return "TFLIPFLOP";
    case ElementType::Text:       return "TEXT";
    case ElementType::Vcc:        return "VCC";
    case ElementType::XNor:       return "XNOR";
    case ElementType::Xor:        return "XOR";
    default:                      return "UNKNOWN";
    }
}

QString ElementFactory::typeToTitleText(ElementType type)
{
    qCDebug(four) << "Creating Element Type conversion type to text.";
    switch (type) {
    case ElementType::And:        return tr("<b>AND</b>");
    case ElementType::Button:     return tr("<b>PUSH BUTTON</b>");
    case ElementType::Buzzer:     return tr("<b>BUZZER</b>");
    case ElementType::Clock:      return tr("<b>CLOCK SIGNAL</b>");
    case ElementType::DFlipFlop:  return tr("<b>D-FLIPFLOP</b>");
    case ElementType::DLatch:     return tr("<b>D-LATCH</b>");
    case ElementType::Demux:      return tr("<b>DEMULTIPLEXER</b>");
    case ElementType::Display14:  return tr("<b>14-SEGMENT DISPLAY</b>");
    case ElementType::Display:    return tr("<b>7-SEGMENT DISPLAY</b>");
    case ElementType::Gnd:        return tr("<b>GROUND</b>");
    case ElementType::IC:         return tr("<b>INTEGRATED CIRCUIT</b>");
    case ElementType::JKFlipFlop: return tr("<b>JK-FLIPFLOP</b>");
    case ElementType::JKLatch:    return tr("<b>JK-LATCH</b>");
    case ElementType::Led:        return tr("<b>LED</b>");
    case ElementType::Line:       return tr("<b>LINE</b>");
    case ElementType::Mux:        return tr("<b>MULTIPLEXER</b>");
    case ElementType::Nand:       return tr("<b>NAND</b>");
    case ElementType::Node:       return tr("<b>NODE</b>");
    case ElementType::Nor:        return tr("<b>NOR</b>");
    case ElementType::Not:        return tr("<b>NOT</b>");
    case ElementType::Or:         return tr("<b>OR</b>");
    case ElementType::Rotary:     return tr("<b>ROTARY SWITCH</b>");
    case ElementType::SRFlipFlop: return tr("<b>SR-FLIPFLOP</b>");
    case ElementType::Switch:     return tr("<b>SWITCH</b>");
    case ElementType::TFlipFlop:  return tr("<b>T-FLIPFLOP</b>");
    case ElementType::Text:       return tr("<b>TEXT</b>");
    case ElementType::Vcc:        return tr("<b>VCC</b>");
    case ElementType::XNor:       return tr("<b>XNOR</b>");
    case ElementType::Xor:        return tr("<b>XOR</b>");
    default:                      return tr("<b>MULTIPLE TYPES</b>");
    }
}

QString ElementFactory::translatedName(ElementType type)
{
    switch (type) {
    case ElementType::And:        return tr("And");
    case ElementType::Button:     return tr("Button");
    case ElementType::Buzzer:     return tr("Buzzer");
    case ElementType::Clock:      return tr("Clock");
    case ElementType::DFlipFlop:  return tr("D-flipflop");
    case ElementType::DLatch:     return tr("D-latch");
    case ElementType::Demux:      return tr("Demux");
    case ElementType::Display14:  return tr("Display14");
    case ElementType::Display:    return tr("Display");
    case ElementType::Gnd:        return tr("GND");
    case ElementType::IC:         return tr("IC");
    case ElementType::JKFlipFlop: return tr("JK-flipflop");
    case ElementType::JKLatch:    return tr("JK-latch");
    case ElementType::Led:        return tr("Led");
    case ElementType::Line:       return tr("Line");
    case ElementType::Mux:        return tr("Mux");
    case ElementType::Nand:       return tr("Nand");
    case ElementType::Node:       return tr("Node");
    case ElementType::Nor:        return tr("Nor");
    case ElementType::Not:        return tr("Not");
    case ElementType::Or:         return tr("Or");
    case ElementType::Rotary:     return tr("Rotary switch");
    case ElementType::SRFlipFlop: return tr("SR-flipflop");
    case ElementType::Switch:     return tr("Switch");
    case ElementType::TFlipFlop:  return tr("T-flipflop");
    case ElementType::Text:       return tr("Text");
    case ElementType::Vcc:        return tr("VCC");
    case ElementType::XNor:       return tr("Xnor");
    case ElementType::Xor:        return tr("Xor");
    default:                      return tr("Unknown");
    }
}

QPixmap ElementFactory::getPixmap(ElementType type)
{
    switch (type) {
    case ElementType::And:        return {":/basic/and.png"};
    case ElementType::Button:     return {":/input/buttonOff.png"};
    case ElementType::Buzzer:     return {":/output/BuzzerOff.png"};
    case ElementType::Clock:      return {":/input/clock1.png"};
    case ElementType::DFlipFlop:  return {":/memory/light/D-flipflop.png"};
    case ElementType::DLatch:     return {":/memory/light/D-latch.png"};
    case ElementType::Demux:      return {":/basic/demux.png"};
    case ElementType::Display14:  return {":/output/counter/counter_14_on.png"};
    case ElementType::Display:    return {":/output/counter/counter_on.png"};
    case ElementType::Gnd:        return {":/input/0.png"};
    case ElementType::IC:         return {":/basic/box.png"};
    case ElementType::JKFlipFlop: return {":/memory/light/JK-flipflop.png"};
    case ElementType::JKLatch:    return {":/memory/light/JK-latch.png"};
    case ElementType::Led:        return {":/output/WhiteLedOff.png"};
    case ElementType::Line:       return {":/line.png"};
    case ElementType::Mux:        return {":/basic/mux.png"};
    case ElementType::Nand:       return {":/basic/nand.png"};
    case ElementType::Node:       return {":/basic/node.png"};
    case ElementType::Nor:        return {":/basic/nor.png"};
    case ElementType::Not:        return {":/basic/not.png"};
    case ElementType::Or:         return {":/basic/or.png"};
    case ElementType::Rotary:     return {":/input/rotary/rotary_icon.png"};
    case ElementType::SRFlipFlop: return {":/memory/light/SR-flipflop.png"};
    case ElementType::Switch:     return {":/input/switchOn.png"};
    case ElementType::TFlipFlop:  return {":/memory/light/T-flipflop.png"};
    case ElementType::Text:       return {":/text.png"};
    case ElementType::Vcc:        return {":/input/1.png"};
    case ElementType::XNor:       return {":/basic/xnor.png"};
    case ElementType::Xor:        return {":/basic/xor.png"};
    default:                      return {};
    }

    return {};
}

ElementFactory::ElementFactory()
{
    clear();
    m_map.clear();
    m_lastId = 0;
}

GraphicElement *ElementFactory::buildElement(ElementType type, QGraphicsItem *parent)
{
    qCDebug(four) << "Creating Element. Building it.";

    return type == ElementType::And        ? new And(parent)
         : type == ElementType::Button     ? new InputButton(parent)
         : type == ElementType::Buzzer     ? new Buzzer(parent)
         : type == ElementType::Clock      ? new Clock(parent)
         : type == ElementType::DFlipFlop  ? new DFlipFlop(parent)
         : type == ElementType::DLatch     ? new DLatch(parent)
         : type == ElementType::Demux      ? new Demux(parent)
         : type == ElementType::Display    ? new Display(parent)
         : type == ElementType::Display14  ? new Display14(parent)
         : type == ElementType::Gnd        ? new InputGnd(parent)
         : type == ElementType::IC         ? new IC(parent)
         : type == ElementType::JKFlipFlop ? new JKFlipFlop(parent)
         : type == ElementType::JKLatch    ? new JKLatch(parent)
         : type == ElementType::Led        ? new Led(parent)
         : type == ElementType::Line       ? new Line(parent)
         : type == ElementType::Mux        ? new Mux(parent)
         : type == ElementType::Nand       ? new Nand(parent)
         : type == ElementType::Node       ? new Node(parent)
         : type == ElementType::Nor        ? new Nor(parent)
         : type == ElementType::Not        ? new Not(parent)
         : type == ElementType::Or         ? new Or(parent)
         : type == ElementType::Rotary     ? new InputRotary(parent)
         : type == ElementType::SRFlipFlop ? new SRFlipFlop(parent)
         : type == ElementType::Switch     ? new InputSwitch(parent)
         : type == ElementType::TFlipFlop  ? new TFlipFlop(parent)
         : type == ElementType::Text       ? new Text(parent)
         : type == ElementType::Vcc        ? new InputVcc(parent)
         : type == ElementType::XNor       ? new Xnor(parent)
         : type == ElementType::Xor        ? new Xor(parent)
                                           : static_cast<GraphicElement *>(nullptr);
}

QNEConnection *ElementFactory::buildConnection(QGraphicsItem *parent)
{
    return new QNEConnection(parent);
}

ItemWithId *ElementFactory::getItemById(int id)
{
    if (instance->m_map.contains(id)) {
        return instance->m_map[id];
    }
    return nullptr;
}

bool ElementFactory::contains(int id)
{
    return instance->m_map.contains(id);
}

void ElementFactory::addItem(ItemWithId *item)
{
    if (item) {
        int newId = instance->next_id();
        instance->m_map[newId] = item;
        item->setId(newId);
    }
}

void ElementFactory::removeItem(ItemWithId *item)
{
    instance->m_map.remove(item->id());
}

void ElementFactory::updateItemId(ItemWithId *item, int newId)
{
    instance->m_map.remove(item->id());
    instance->m_map[newId] = item;
    item->setId(newId);
}

int ElementFactory::next_id()
{
    return m_lastId++;
}

void ElementFactory::clear()
{
    // qCDebug(zero) << "Element Factory clear.";
    // m_map.clear();
    // m_lastId = 1;
}
