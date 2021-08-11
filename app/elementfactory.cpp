// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QDebug>

#include "element/and.h"
#include "element/buzzer.h"
#include "element/clock.h"
#include "element/demux.h"
#include "element/dflipflop.h"
#include "element/display.h"
#include "element/display_14.h"
#include "element/dlatch.h"
#include "element/inputbutton.h"
#include "element/inputgnd.h"
#include "element/inputswitch.h"
#include "element/inputvcc.h"
#include "element/inputrotary.h"
#include "element/jkflipflop.h"
#include "element/jklatch.h"
#include "element/led.h"
#include "element/line.h"
#include "element/nand.h"
#include "element/nor.h"
#include "element/not.h"
#include "element/or.h"
#include "element/srflipflop.h"
#include "element/tflipflop.h"
#include "element/text.h"
#include "element/remotedevice.h"
#include "ic.h"
#include "itemwithid.h"
#include "element/mux.h"
#include "element/node.h"
#include "element/xnor.h"
#include "element/xor.h"
#include "common.h"
#include "elementfactory.h"
#include "graphicelement.h"
#include "qneconnection.h"

ElementFactory *ElementFactory::instance = new ElementFactory();

size_t ElementFactory::getLastId() const
{
    return m_lastId;
}

ElementType ElementFactory::textToType(QString text)
{
    COMMENT("Creating Element Type conversion text to type.", 4);
    text = text.toUpper();
    ElementType type;
    type = text == "BUTTON"    ? ElementType::BUTTON
        : text == "LED"        ? ElementType::LED
        : text == "AND"        ? ElementType::AND
        : text == "OR"         ? ElementType::OR
        : text == "CLOCK"      ? ElementType::CLOCK
        : text == "SWITCH"     ? ElementType::SWITCH
        : text == "NOT"        ? ElementType::NOT
        : text == "NAND"       ? ElementType::NAND
        : text == "NOR"        ? ElementType::NOR
        : text == "XOR"        ? ElementType::XOR
        : text == "XNOR"       ? ElementType::XNOR
        : text == "VCC"        ? ElementType::VCC
        : text == "GND"        ? ElementType::GND
        : text == "DFLIPFLOP"  ? ElementType::DFLIPFLOP
        : text == "DLATCH"     ? ElementType::DLATCH
        : text == "JKFLIPFLOP" ? ElementType::JKFLIPFLOP
        : text == "JKLATCH"    ? ElementType::JKLATCH
        : text == "SRFLIPFLOP" ? ElementType::SRFLIPFLOP
        : text == "ROTARY"     ? ElementType::ROTARY
        : text == "TFLIPFLOP"  ? ElementType::TFLIPFLOP
        : text == "DISPLAY"    ? ElementType::DISPLAY
        : text == "DISPLAY14"  ? ElementType::DISPLAY14
        : text == "IC"         ? ElementType::IC
        : text == "MUX"        ? ElementType::MUX
        : text == "DEMUX"      ? ElementType::DEMUX
        : text == "NODE"       ? ElementType::NODE
        : text == "BUZZER"     ? ElementType::BUZZER
        : text == "TEXT"       ? ElementType::TEXT
        : text == "LINE"       ? ElementType::LINE
        : text == "REMOTE"     ? ElementType::REMOTE
                               : ElementType::UNKNOWN;
    return type;
}

QString ElementFactory::typeToText(ElementType type)
{
    COMMENT("Creating Element Type conversion type to text.", 4);
    switch (type) {
    case ElementType::BUTTON:
        return "BUTTON";
    case ElementType::LED:
        return "LED";
    case ElementType::AND:
        return "AND";
    case ElementType::OR:
        return "OR";
    case ElementType::CLOCK:
        return "CLOCK";
    case ElementType::SWITCH:
        return "SWITCH";
    case ElementType::NOT:
        return "NOT";
    case ElementType::NAND:
        return "NAND";
    case ElementType::NOR:
        return "NOR";
    case ElementType::XOR:
        return "XOR";
    case ElementType::XNOR:
        return "XNOR";
    case ElementType::VCC:
        return "VCC";
    case ElementType::GND:
        return "GND";
    case ElementType::DFLIPFLOP:
        return "DFLIPFLOP";
    case ElementType::DLATCH:
        return "DLATCH";
    case ElementType::JKFLIPFLOP:
        return "JKFLIPFLOP";
    case ElementType::JKLATCH:
        return "JKLATCH";
    case ElementType::SRFLIPFLOP:
        return "SRFLIPFLOP";
    case ElementType::ROTARY:
        return  "ROTARY" ;
    case ElementType::TFLIPFLOP:
        return "TFLIPFLOP";
    case ElementType::DISPLAY:
        return "DISPLAY";
    case ElementType::DISPLAY14:
        return "DISPLAY14";
    case ElementType::IC:
        return "IC";
    case ElementType::MUX:
        return "MUX";
    case ElementType::DEMUX:
        return "DEMUX";
    case ElementType::NODE:
        return "NODE";
    case ElementType::BUZZER:
        return "BUZZER";
    case ElementType::TEXT:
        return "TEXT";
    case ElementType::LINE:
        return "LINE";
    case ElementType::REMOTE:
        return "REMOTE";
    case ElementType::UNKNOWN:
    default:
        return "UNKNOWN";
    }
}

QString ElementFactory::typeToTitleText(ElementType type)
{
    COMMENT("Creating Element Type conversion type to text.", 4);
    switch (type) {
    case ElementType::BUTTON:
        return tr("<b>PUSH BUTTON</b>");
    case ElementType::LED:
        return tr("<b>LED</b>");
    case ElementType::AND:
        return tr("<b>AND</b>");
    case ElementType::OR:
        return tr("<b>OR</b>");
    case ElementType::CLOCK:
        return tr("<b>CLOCK SIGNAL</b>");
    case ElementType::SWITCH:
        return tr("<b>SWITCH</b>");
    case ElementType::NOT:
        return tr("<b>NOT</b>");
    case ElementType::NAND:
        return tr("<b>NAND</b>");
    case ElementType::NOR:
        return tr("<b>NOR</b>");
    case ElementType::XOR:
        return tr("<b>XOR</b>");
    case ElementType::XNOR:
        return tr("<b>XNOR</b>");
    case ElementType::VCC:
        return tr("<b>VCC</b>");
    case ElementType::GND:
        return tr("<b>GROUND</b>");
    case ElementType::DFLIPFLOP:
        return tr("<b>D-FLIPFLOP</b>");
    case ElementType::DLATCH:
        return tr("<b>D-LATCH</b>");
    case ElementType::JKFLIPFLOP:
        return tr("<b>JK-FLIPFLOP</b>");
    case ElementType::JKLATCH:
        return tr("<b>JK-LATCH</b>");
    case ElementType::SRFLIPFLOP:
        return tr("<b>SR-FLIPFLOP</b>");
    case ElementType::ROTARY:
        return tr("<b>ROTARY SWITCH</b>");
    case ElementType::TFLIPFLOP:
        return tr("<b>T-FLIPFLOP</b>");
    case ElementType::DISPLAY:
        return tr("<b>7-SEGMENT DISPLAY</b>");
    case ElementType::DISPLAY14:
        return tr("<b>14-SEGMENT DISPLAY</b>");
    case ElementType::IC:
        return tr("<b>INTEGRATED CIRCUIT</b>");
    case ElementType::MUX:
        return tr("<b>MULTIPLEXER</b>");
    case ElementType::DEMUX:
        return tr("<b>DEMULTIPLEXER</b>");
    case ElementType::NODE:
        return tr("<b>NODE</b>");
    case ElementType::BUZZER:
        return tr("<b>BUZZER</b>");
    case ElementType::TEXT:
        return tr("<b>TEXT</b>");
    case ElementType::LINE:
        return tr("<b>LINE</b>");
    case ElementType::REMOTE:
        return tr("<b>REMOTE</b>");
    case ElementType::UNKNOWN:
    default:
        return tr("<b>MULTIPLE TYPES</b>");
    }
}

QString ElementFactory::translatedName(ElementType type)
{
    switch (type) {
    case ElementType::BUTTON:
        return tr("Button");
    case ElementType::LED:
        return tr("Led");
    case ElementType::AND:
        return tr("And");
    case ElementType::OR:
        return tr("Or");
    case ElementType::CLOCK:
        return tr("Clock");
    case ElementType::SWITCH:
        return tr("Switch");
    case ElementType::NOT:
        return tr("Not");
    case ElementType::NAND:
        return tr("Nand");
    case ElementType::NOR:
        return tr("Nor");
    case ElementType::XOR:
        return tr("Xor");
    case ElementType::XNOR:
        return tr("Xnor");
    case ElementType::VCC:
        return tr("VCC");
    case ElementType::GND:
        return tr("GND");
    case ElementType::DFLIPFLOP:
        return tr("D-flipflop");
    case ElementType::DLATCH:
        return tr("D-latch");
    case ElementType::JKFLIPFLOP:
        return tr("JK-flipflop");
    case ElementType::JKLATCH:
        return tr("JK-latch");
    case ElementType::SRFLIPFLOP:
        return tr("SR-flipflop");
    case ElementType::ROTARY:
        return tr("Rotary switch");
    case ElementType::TFLIPFLOP:
        return tr("T-flipflop");
    case ElementType::DISPLAY:
        return tr("Display");
    case ElementType::DISPLAY14:
        return tr("Display14");
    case ElementType::IC:
        return tr("IC");
    case ElementType::MUX:
        return tr("Mux");
    case ElementType::DEMUX:
        return tr("Demux");
    case ElementType::NODE:
        return tr("Node");
    case ElementType::BUZZER:
        return tr("Buzzer");
    case ElementType::TEXT:
        return tr("Text");
    case ElementType::LINE:
        return tr("Line");
    case ElementType::REMOTE:
        return tr("Remote");
    case ElementType::UNKNOWN:
    default:
        return tr("Unknown");
    }
}

QPixmap ElementFactory::getPixmap(ElementType type)
{
    switch (type) {
    case ElementType::BUTTON:
        return QPixmap(":/input/buttonOff.png");
    case ElementType::LED:
        return QPixmap(":/output/WhiteLedOff.png");
    case ElementType::AND:
        return QPixmap(":/basic/and.png");
    case ElementType::OR:
        return QPixmap(":/basic/or.png");
    case ElementType::CLOCK:
        return QPixmap(":/input/clock1.png");
    case ElementType::SWITCH:
        return QPixmap(":/input/switchOn.png");
    case ElementType::NOT:
        return QPixmap(":/basic/not.png");
    case ElementType::NAND:
        return QPixmap(":/basic/nand.png");
    case ElementType::NOR:
        return QPixmap(":/basic/nor.png");
    case ElementType::XOR:
        return QPixmap(":/basic/xor.png");
    case ElementType::XNOR:
        return QPixmap(":/basic/xnor.png");
    case ElementType::VCC:
        return QPixmap(":/input/1.png");
    case ElementType::GND:
        return QPixmap(":/input/0.png");
    case ElementType::DFLIPFLOP:
        return QPixmap(":/memory/light/D-flipflop.png");
    case ElementType::DLATCH:
        return QPixmap(":/memory/light/D-latch.png");
    case ElementType::JKFLIPFLOP:
        return QPixmap(":/memory/light/JK-flipflop.png");
    case ElementType::JKLATCH:
        return QPixmap(":/memory/light/JK-latch.png");
    case ElementType::SRFLIPFLOP:
        return QPixmap(":/memory/light/SR-flipflop.png");
    case ElementType::TFLIPFLOP:
        return QPixmap(":/memory/light/T-flipflop.png");
    case ElementType::ROTARY:
        return QPixmap(":/input/rotary/rotary_icon.png");
    case ElementType::DISPLAY:
        return QPixmap(":/output/counter/counter_on.png");
    case ElementType::DISPLAY14:
        return QPixmap(":/output/counter/counter_14_on.png");
    case ElementType::IC:
        return QPixmap(":/basic/box.png");
    case ElementType::MUX:
        return QPixmap(":/basic/mux.png");
    case ElementType::DEMUX:
        return QPixmap(":/basic/demux.png");
    case ElementType::NODE:
        return QPixmap(":/basic/node.png");
    case ElementType::BUZZER:
        return QPixmap(":/output/BuzzerOff.png");
    case ElementType::TEXT:
        return QPixmap(":/text.png");
    case ElementType::LINE:
        return QPixmap(":/line.png");
    case ElementType::REMOTE:
        return QPixmap(":/remote/remote_device.png");
    case ElementType::UNKNOWN:
        return QPixmap();
    }
    return QPixmap();
}

ElementFactory::ElementFactory()
{
    clear();
    m_map.clear();
    m_lastId = 0;
}

GraphicElement *ElementFactory::buildElement(ElementType type, QGraphicsItem *parent)
{
    COMMENT("Creating Element. Building it!", 4);
    GraphicElement *elm;
    elm = type == ElementType::BUTTON     ? new InputButton(parent)
        : type == ElementType::SWITCH     ? new InputSwitch(parent)
        : type == ElementType::LED        ? new Led(parent)
        : type == ElementType::NOT        ? new Not(parent)
        : type == ElementType::AND        ? new And(parent)
        : type == ElementType::OR         ? new Or(parent)
        : type == ElementType::NAND       ? new Nand(parent)
        : type == ElementType::NOR        ? new Nor(parent)
        : type == ElementType::CLOCK      ? new Clock(parent)
        : type == ElementType::XOR        ? new Xor(parent)
        : type == ElementType::XNOR       ? new Xnor(parent)
        : type == ElementType::VCC        ? new InputVcc(parent)
        : type == ElementType::GND        ? new InputGnd(parent)
        : type == ElementType::DLATCH     ? new DLatch(parent)
        : type == ElementType::DFLIPFLOP  ? new DFlipFlop(parent)
        : type == ElementType::JKLATCH    ? new JKLatch(parent)
        : type == ElementType::JKFLIPFLOP ? new JKFlipFlop(parent)
        : type == ElementType::SRFLIPFLOP ? new SRFlipFlop(parent)
        : type == ElementType::TFLIPFLOP  ? new TFlipFlop(parent)
        : type == ElementType::ROTARY     ? new InputRotary(parent)
        : type == ElementType::DISPLAY    ? new Display(parent)
        : type == ElementType::DISPLAY14  ? new Display14(parent)
        : type == ElementType::IC         ? new IC(parent)
        : type == ElementType::NODE       ? new Node(parent)
        : type == ElementType::MUX        ? new Mux(parent)
        : type == ElementType::DEMUX      ? new Demux(parent)
        : type == ElementType::BUZZER     ? new Buzzer(parent)
        : type == ElementType::TEXT       ? new Text(parent)
        : type == ElementType::LINE       ? new Line(parent)
        : type == ElementType::REMOTE     ? new RemoteDevice(parent)
                                          : static_cast<GraphicElement *>(nullptr);
    return elm;
}

QNEConnection *ElementFactory::buildConnection(QGraphicsItem *parent)
{
    return new QNEConnection(parent);
}

ItemWithId *ElementFactory::getItemById(size_t id)
{
    if (instance->m_map.contains(id)) {
        return instance->m_map[id];
    }
    return nullptr;
}

bool ElementFactory::contains(size_t id)
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

void ElementFactory::updateItemId(ItemWithId *item, size_t newId)
{
    instance->m_map.remove(item->id());
    instance->m_map[newId] = item;
    item->setId(newId);
}

size_t ElementFactory::next_id()
{
    return m_lastId++;
}

void ElementFactory::clear()
{
    COMMENT("Element Factory clear.", 0);
    //m_map.clear();
    //m_lastId = 1;
}
