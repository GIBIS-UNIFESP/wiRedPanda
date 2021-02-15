// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "led.h"

#include "qneport.h"

#include <bitset>

int Led::current_id_number = 0;

/* Color pallets:
 * 2 bit -   00: dark gray,   01: light blue,   10: light green,   11: light red
 * 3 bit -  000: dark gray,  001: light blue,  010: light green,  011: yellow,  100: light red,  101: magenta,  110: cyan,    111: white
 * 4 bit - 0000: black,     0001: blue,       0010: green,       0011: teal,   0100: red,       0101: magenta, 0110: orange, 0111: light gray
 *         1000: dark gray, 1001: light blue, 1010: light green, 1011: cyan,   1100: light red, 1101: pink,    1110: yellow, 1111: white
 */
Led::Led(QGraphicsItem *parent)
    : GraphicElement(ElementType::LED, ElementGroup::OUTPUT, 1, 4, 0, 0, parent)
{
    m_pixmapSkinName = {
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

    setOutputsOnTop(true);
    setRotatable(false);
    setHasColors(true);
    setColor("White");
    setPixmap(m_pixmapSkinName[0]);
    updatePorts();
    setHasLabel(true);
    setCanChangeSkin(true);
    setPortName("Led");
}

void Led::refresh()
{
    int idx = 0;
    if (isValid()) {
        std::bitset<4> index;
        for (int i = 0; i < inputSize(); ++i) {
            index[i] = input(inputSize() - i - 1)->value();
        }
        idx = index.to_ulong();
    }
    switch (inputSize()) {
    case 1: { /* 1 bit */
        setPixmap(m_pixmapSkinName[m_colorNumber + idx]);
        break;
    }
    case 2: { /* 2 bits */ // TODO: add option to select dark/light colors according to the theme.
        if (idx == 3) {
            setPixmap(m_pixmapSkinName[22]);
        } else {
            setPixmap(m_pixmapSkinName[18 + idx]);
        }
        break;
    }
    case 3: { /* 3 bits */ // TODO: add option to select dark/light colors according to the theme.
        setPixmap(m_pixmapSkinName[18 + idx]);
        break;
    }
    case 4: { /* 4 bits */
        setPixmap(m_pixmapSkinName[10 + idx]);
        break;
    }
    }
}

void Led::setColor(const QString &color)
{
    m_color = color;
    if (color == "White") {
        m_colorNumber = 0;
    } else if (color == "Red") {
        m_colorNumber = 2;
    } else if (color == "Green") {
        m_colorNumber = 4;
    } else if (color == "Blue") {
        m_colorNumber = 6;
    } else if (color == "Purple") {
        m_colorNumber = 8;
    }
    refresh();
}

QString Led::getColor() const
{
    return m_color;
}

void Led::save(QDataStream &ds) const
{
    GraphicElement::save(ds);
    ds << getColor();
}

void Led::load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version)
{
    GraphicElement::load(ds, portMap, version);
    if (version >= 1.1) {
        QString clr;
        ds >> clr;
        setColor(clr);
    }
}

QString Led::genericProperties()
{
    return getColor();
}

void Led::updatePorts()
{
    setHasColors(inputSize() == 1);

    GraphicElement::updatePorts();
}

void Led::setSkin(bool defaultSkin, const QString &filename)
{
    int idx = 0;
    if (isValid()) {
        std::bitset<4> index;
        for (int i = 0; i < inputSize(); ++i) {
            index[i] = input(inputSize() - i - 1)->value();
        }
        idx = index.to_ulong();
    }
    int value_idx = 0;
    switch (inputSize()) {
    case 1: /* 1 bit */
        value_idx = m_colorNumber + idx;
    break;
    case 2: /* 2 bits */ // TODO: add option to select dark/light colors according to the theme.
        value_idx = idx == 3 ? 22 : 18 + idx;
    break;

    case 3:  /* 3 bits */ // TODO: add option to select dark/light colors according to the theme.
        value_idx = 18 + idx;
    break;

    case 4:  /* 4 bits */
        value_idx = 10 + idx;
    break;
    }

    if (defaultSkin) {
        resetLedPixmapName(value_idx);
    } else {
        m_pixmapSkinName[value_idx] = filename;
    }
    setPixmap(m_pixmapSkinName[value_idx]);
}

void Led::resetLedPixmapName(int ledNumber)
{
    switch (ledNumber) {
    case 0:
        m_pixmapSkinName[0] = ":/output/WhiteLedOff.png";
        break;
    case 1:
        m_pixmapSkinName[1] = ":/output/WhiteLedOn.png";
        break;
    case 2:
        m_pixmapSkinName[2] = ":/output/RedLedOff.png";
        break;
    case 3:
        m_pixmapSkinName[3] = ":/output/RedLedOn.png";
        break;
    case 4:
        m_pixmapSkinName[4] = ":/output/GreenLedOff.png";
        break;
    case 5:
        m_pixmapSkinName[5] = ":/output/GreenLedOn.png";
        break;
    case 6:
        m_pixmapSkinName[6] = ":/output/BlueLedOff.png";
        break;
    case 7:
        m_pixmapSkinName[7] = ":/output/BlueLedOn.png";
        break;
    case 8:
        m_pixmapSkinName[8] = ":/output/PurpleLedOff.png";
        break;
    case 9:
        m_pixmapSkinName[9] = ":/output/PurpleLedOn.png";
        break;
    case 10:
        m_pixmapSkinName[10] = ":/output/16colors/BlackLedOn.png";
        break;
    case 11:
        m_pixmapSkinName[11] = ":/output/16colors/NavyBlueLedOn.png";
        break;
    case 12:
        m_pixmapSkinName[12] = ":/output/16colors/GreenLedOn.png";
        break;
    case 13:
        m_pixmapSkinName[13] = ":/output/16colors/TealLedOn.png";
        break;
    case 14:
        m_pixmapSkinName[14] = ":/output/16colors/DarkRedLedOn.png";
        break;
    case 15:
        m_pixmapSkinName[15] = ":/output/16colors/MagentaLedOn.png";
        break;
    case 16:
        m_pixmapSkinName[16] = ":/output/16colors/OrangeLedOn.png";
        break;
    case 17:
        m_pixmapSkinName[17] = ":/output/16colors/LightGrayLedOn.png";
        break;
    case 18:
        m_pixmapSkinName[18] = ":/output/16colors/DarkGrayLedOn.png";
        break;
    case 19:
        m_pixmapSkinName[19] = ":/output/16colors/RoyalLedOn.png";
        break;
    case 20:
        m_pixmapSkinName[20] = ":/output/16colors/LimeGreenLedOn.png";
        break;
    case 21:
        m_pixmapSkinName[21] = ":/output/16colors/AquaLightLedOn.png";
        break;
    case 22:
        m_pixmapSkinName[22] = ":/output/16colors/RedLedOn.png";
        break;
    case 23:
        m_pixmapSkinName[23] = ":/output/16colors/HotPinkLedOn.png";
        break;
    case 24:
        m_pixmapSkinName[24] = ":/output/16colors/YellowLedOn.png";
        break;
    case 25:
        m_pixmapSkinName[25] = ":/output/16colors/WhiteLedOn.png";
        break;
    }
}
