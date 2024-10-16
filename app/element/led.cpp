
// Copyright 2015 - 2024, GIBIS-UNIFESP and the wiRedPanda contributors



// SPDX-License-Identifier: GPL-3.0-or-later

#include "led.h"

#include "globalproperties.h"
#include "qneport.h"

#include <bitset>

namespace
{
    int id = qRegisterMetaType<Led>();
}

Led::Led(QGraphicsItem *parent)
    : GraphicElement(ElementType::Led, ElementGroup::Output, ":/output/led/LedOff.svg", tr("LED"), tr("LED"), 1, 4, 0, 0, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins = QStringList{
        // Single input values:
        ":/output/led/LedOff.svg",        // 0
        ":/output/led/WhiteLed.svg",      // 1
        ":/output/led/LedOff.svg",        // 2
        ":/output/led/RedLed.svg",        // 3
        ":/output/led/LedOff.svg",        // 4
        ":/output/led/GreenLed.svg",      // 5
        ":/output/led/LedOff.svg",        // 6
        ":/output/led/BlueLed.svg",       // 7
        ":/output/led/LedOff.svg",        // 8
        ":/output/led/PurpleLed.svg",     // 9
        // Multiple input values:
        ":/output/led//BlackLed.png",     // 10
        ":/output/led//NavyBlueLed.png",  // 11
        ":/output/led//GreenLed.png",     // 12
        ":/output/led//TealLed.png",      // 13
        ":/output/led//DarkRedLed.png",   // 14
        ":/output/led//MagentaLed.png",   // 15
        ":/output/led//OrangeLed.png",    // 16
        ":/output/led//LightGrayLed.png", // 17

        ":/output/led/LedOff.svg",        // 18
        ":/output/led/RoyalLed.png",      // 19
        ":/output/led/LimeGreenLed.png",  // 20
        ":/output/led/AquaLightLed.png",  // 21
        ":/output/led/RedLed.png",        // 22
        ":/output/led/HotPinkLed.png",    // 23
        ":/output/led/YellowLed.png",     // 24
        ":/output/led/WhiteLed.png",      // 25
    };
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);
    setHasColors(true);
    setHasLabel(true);
    setRotatable(false);

    Led::updatePortsProperties();
}

/* Color pallets:
 *
 * 4 bit                    3 bit                    2 bit
 *     0000: black,             000: dark gray,          00: dark gray,
 *     0001: blue,              001: light blue,         01: light blue,
 *     0010: green,             010: light green,        10: light green,
 *     0011: teal,              011: yellow,             11: light red
 *     0100: red,               100: light red,
 *     0101: magenta,           101: magenta,
 *     0110: orange,            110: cyan,
 *     0111: light gray         111: white
 *     1000: dark gray,
 *     1001: light blue,
 *     1010: light green,
 *     1011: cyan,
 *     1100: light red,
 *     1101: pink,
 *     1110: yellow,
 *     1111: white
 */

int Led::colorIndex()
{
    if (!isValid()) {
        return 0;
    }

    std::bitset<4> indexBit;

    for (int i = 0; i < inputSize(); ++i) {
        indexBit[i] = static_cast<bool>(inputPort(i)->status() == Status::Active);
    }

    const int index = static_cast<int>(indexBit.to_ulong());

    int index2 = 0;

    switch (inputSize()) {
    case 1: index2 = m_colorIndex + index;           break;
    case 2: index2 = (index == 3) ? 25 : 18 + index; break;
    case 3: index2 = 18 + index;                     break;
    case 4: index2 = 10 + index;                     break;
    }

    return index2;
}

void Led::refresh()
{
    setPixmap(colorIndex());
}

void Led::setColor(const QString &color)
{
    m_color = color;

    if (color == "White")  { m_colorIndex = 0; }
    if (color == "Red")    { m_colorIndex = 2; }
    if (color == "Green")  { m_colorIndex = 4; }
    if (color == "Blue")   { m_colorIndex = 6; }
    if (color == "Purple") { m_colorIndex = 8; }
}

QString Led::color() const
{
    return m_color;
}

void Led::save(QDataStream &stream) const
{
    GraphicElement::save(stream);

    QMap<QString, QVariant> map;
    map.insert("color", color());

    stream << map;
}

void Led::load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version)
{
    GraphicElement::load(stream, portMap, version);

    if ((VERSION("1.1") <= version) && (version < VERSION("4.1"))) {
        QString color_; stream >> color_;
        setColor(color_);
    }

    if (version >= VERSION("4.1")) {
        QMap<QString, QVariant> map; stream >> map;

        if (map.contains("color")) {
            setColor(map.value("color").toString());
        }
    }
}

QString Led::genericProperties()
{
    return color();
}

void Led::updatePortsProperties()
{
    setHasColors(inputSize() == 1);

    for (auto *port : qAsConst(m_inputPorts)) {
        port->setName(QString::number(m_inputPorts.indexOf(port)));
        port->setRequired(false);
    }

    GraphicElement::updatePortsProperties();
}

void Led::setSkin(const bool useDefaultSkin, const QString &fileName)
{
    const int index = colorIndex();

    if (useDefaultSkin) {
        m_alternativeSkins = m_defaultSkins;
    } else {
        m_alternativeSkins[index] = fileName;
    }

    m_usingDefaultSkin = useDefaultSkin;
    setPixmap(index);
}
