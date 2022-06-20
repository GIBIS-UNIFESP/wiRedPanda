// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "led.h"

#include "qneport.h"

#include <bitset>

namespace
{
int id = qRegisterMetaType<Led>();
}

/* Color pallets:
 *
 * 2 bit
 *     00: dark gray,
 *     01: light blue,
 *     10: light green,
 *     11: light red
 *
 * 3 bit
 *     000: dark gray,
 *     001: light blue,
 *     010: light green,
 *     011: yellow,
 *     100: light red,
 *     101: magenta,
 *     110: cyan,
 *     111: white
 *
 * 4 bit
 *     0000: black,
 *     0001: blue,
 *     0010: green,
 *     0011: teal,
 *     0100: red,
 *     0101: magenta,
 *     0110: orange,
 *     0111: light gray
 *     1000: dark gray,
 *     1001: light blue,
 *     1010: light green,
 *     1011: cyan,
 *     1100: light red,
 *     1101: pink,
 *     1110: yellow,
 *     1111: white
 */

Led::Led(QGraphicsItem *parent)
    : GraphicElement(ElementType::Led, ElementGroup::Output, 1, 4, 0, 0, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins = QStringList{
        ":/output/WhiteLedOff.png",             // Single input values: 0
        ":/output/WhiteLedOn.png",              // 1
        ":/output/RedLedOff.png",               // 2
        ":/output/RedLedOn.png",                // 3
        ":/output/GreenLedOff.png",             // 4
        ":/output/GreenLedOn.png",              // 5
        ":/output/BlueLedOff.png",              // 6
        ":/output/BlueLedOn.png",               // 7
        ":/output/PurpleLedOff.png",            // 8
        ":/output/PurpleLedOn.png",             // 9
        ":/output/16colors/BlackLedOn.png",     // Multiple input values: 10
        ":/output/16colors/NavyBlueLedOn.png",  // 11
        ":/output/16colors/GreenLedOn.png",     // 12
        ":/output/16colors/TealLedOn.png",      // 13
        ":/output/16colors/DarkRedLedOn.png",   // 14
        ":/output/16colors/MagentaLedOn.png",   // 15
        ":/output/16colors/OrangeLedOn.png",    // 16
        ":/output/16colors/LightGrayLedOn.png", // 17
        ":/output/16colors/DarkGrayLedOn.png",  // 18
        ":/output/16colors/RoyalLedOn.png",     // 19
        ":/output/16colors/LimeGreenLedOn.png", // 20
        ":/output/16colors/AquaLightLedOn.png", // 21
        ":/output/16colors/RedLedOn.png",       // 22
        ":/output/16colors/HotPinkLedOn.png",   // 23
        ":/output/16colors/YellowLedOn.png",    // 24
        ":/output/16colors/WhiteLedOn.png",     // 25
    };
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setRotatable(false);
    setHasColors(true);
    Led::updatePorts();
    setHasLabel(true);
    setCanChangeSkin(true);
    setPortName(m_translatedName);
    setToolTip(m_translatedName);
}

void Led::refresh()
{
    int index = 0;

    if (isValid()) {
        std::bitset<4> indexBit;
        for (int i = 0; i < inputSize(); ++i) {
            indexBit[i] = static_cast<int>(inputPort(inputSize() - i - 1)->value());
        }
        index = static_cast<int>(indexBit.to_ulong());
    }

    // TODO: add option to select dark/light colors according to the theme.
    switch (inputSize()) {
    case 1: {
        setPixmap(m_colorNumber + index);
        inputPort(0)->setName("");
        break;
    }
    case 2: {
        setPixmap((index == 3) ? 22 : 18 + index);
        inputPort(0)->setName("0");
        inputPort(1)->setName("1");
        break;
    }
    case 3: {
        setPixmap(18 + index);
        inputPort(0)->setName("0");
        inputPort(1)->setName("1");
        inputPort(2)->setName("2");
        break;
    }
    case 4: {
        setPixmap(10 + index);
        inputPort(0)->setName("0");
        inputPort(1)->setName("1");
        inputPort(2)->setName("2");
        inputPort(3)->setName("3");
        break;
    }
    }
}

void Led::setColor(const QString &color)
{
    m_color = color;
    if (color == "White")  { m_colorNumber = 0; }
    if (color == "Red")    { m_colorNumber = 2; }
    if (color == "Green")  { m_colorNumber = 4; }
    if (color == "Blue")   { m_colorNumber = 6; }
    if (color == "Purple") { m_colorNumber = 8; }
}

QString Led::color() const
{
    return m_color;
}

void Led::save(QDataStream &stream) const
{
    GraphicElement::save(stream);
    stream << color();
}

void Led::load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const double version)
{
    GraphicElement::load(stream, portMap, version);
    if (version >= 1.1) {
        QString color;
        stream >> color;
        setColor(color);
    }
}

QString Led::genericProperties()
{
    return color();
}

void Led::updatePorts()
{
    setHasColors(inputSize() == 1);
    GraphicElement::updatePorts();
}

void Led::setSkin(const bool defaultSkin, const QString &fileName)
{
    m_usingDefaultSkin = defaultSkin;

    int index = 0;

    if (isValid()) {
        std::bitset<4> indexBit;
        for (int i = 0; i < inputSize(); ++i) {
            indexBit[i] = static_cast<int>(inputPort(inputSize() - i - 1)->value());
        }
        index = static_cast<int>(indexBit.to_ulong());
    }

    int index2 = 0;

    // TODO: add option to select dark/light colors according to the theme.
    switch (inputSize()) {
    case 1: index2 = m_colorNumber + index;          break;
    case 2: index2 = (index == 3) ? 22 : 18 + index; break;
    case 3: index2 = 18 + index;                     break;
    case 4: index2 = 10 + index;                     break;
    }

    m_alternativeSkins[index2] = fileName;
    setPixmap(index2);
}

void Led::resetSkin(const int ledNumber)
{
    switch (ledNumber) {
    case 0:  m_defaultSkins[0]  = ":/output/WhiteLedOff.png";             break;
    case 1:  m_defaultSkins[1]  = ":/output/WhiteLedOn.png";              break;
    case 2:  m_defaultSkins[2]  = ":/output/RedLedOff.png";               break;
    case 3:  m_defaultSkins[3]  = ":/output/RedLedOn.png";                break;
    case 4:  m_defaultSkins[4]  = ":/output/GreenLedOff.png";             break;
    case 5:  m_defaultSkins[5]  = ":/output/GreenLedOn.png";              break;
    case 6:  m_defaultSkins[6]  = ":/output/BlueLedOff.png";              break;
    case 7:  m_defaultSkins[7]  = ":/output/BlueLedOn.png";               break;
    case 8:  m_defaultSkins[8]  = ":/output/PurpleLedOff.png";            break;
    case 9:  m_defaultSkins[9]  = ":/output/PurpleLedOn.png";             break;
    case 10: m_defaultSkins[10] = ":/output/16colors/BlackLedOn.png";     break;
    case 11: m_defaultSkins[11] = ":/output/16colors/NavyBlueLedOn.png";  break;
    case 12: m_defaultSkins[12] = ":/output/16colors/GreenLedOn.png";     break;
    case 13: m_defaultSkins[13] = ":/output/16colors/TealLedOn.png";      break;
    case 14: m_defaultSkins[14] = ":/output/16colors/DarkRedLedOn.png";   break;
    case 15: m_defaultSkins[15] = ":/output/16colors/MagentaLedOn.png";   break;
    case 16: m_defaultSkins[16] = ":/output/16colors/OrangeLedOn.png";    break;
    case 17: m_defaultSkins[17] = ":/output/16colors/LightGrayLedOn.png"; break;
    case 18: m_defaultSkins[18] = ":/output/16colors/DarkGrayLedOn.png";  break;
    case 19: m_defaultSkins[19] = ":/output/16colors/RoyalLedOn.png";     break;
    case 20: m_defaultSkins[20] = ":/output/16colors/LimeGreenLedOn.png"; break;
    case 21: m_defaultSkins[21] = ":/output/16colors/AquaLightLedOn.png"; break;
    case 22: m_defaultSkins[22] = ":/output/16colors/RedLedOn.png";       break;
    case 23: m_defaultSkins[23] = ":/output/16colors/HotPinkLedOn.png";   break;
    case 24: m_defaultSkins[24] = ":/output/16colors/YellowLedOn.png";    break;
    case 25: m_defaultSkins[25] = ":/output/16colors/WhiteLedOn.png";     break;
    default:
        throw Pandaception(tr("Invalid skin number!"));
    }
}
