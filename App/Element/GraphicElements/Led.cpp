// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Led.h"

#include <bitset>

#include "App/Element/ElementInfo.h"
#include "App/IO/VersionInfo.h"
#include "App/Nodes/QNEPort.h"

template<>
struct ElementInfo<Led> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Led,
        .group = ElementGroup::Output,
        .minInputSize = 1,
        .maxInputSize = 4,
        .canChangeSkin = true,
        .hasColors = true,
        .hasLabel = true,
        .rotatable = false,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Output/Led/LedOff.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("Led", "LED");
        meta.translatedName = QT_TRANSLATE_NOOP("Led", "LED");
        meta.trContext = "Led";
        meta.defaultSkins = QStringList({
            // Single input values:
            ":/Components/Output/Led/LedOff.svg",        // 0
            ":/Components/Output/Led/WhiteLed.svg",      // 1
            ":/Components/Output/Led/LedOff.svg",        // 2
            ":/Components/Output/Led/RedLed.svg",        // 3
            ":/Components/Output/Led/LedOff.svg",        // 4
            ":/Components/Output/Led/GreenLed.svg",      // 5
            ":/Components/Output/Led/LedOff.svg",        // 6
            ":/Components/Output/Led/BlueLed.svg",       // 7
            ":/Components/Output/Led/LedOff.svg",        // 8
            ":/Components/Output/Led/PurpleLed.svg",     // 9
            // Multiple input values:
            ":/Components/Output/Led//BlackLed.png",     // 10
            ":/Components/Output/Led//NavyBlueLed.png",  // 11
            ":/Components/Output/Led//GreenLed.png",     // 12
            ":/Components/Output/Led//TealLed.png",      // 13
            ":/Components/Output/Led//DarkRedLed.png",   // 14
            ":/Components/Output/Led//MagentaLed.png",   // 15
            ":/Components/Output/Led//OrangeLed.png",    // 16
            ":/Components/Output/Led//LightGrayLed.png", // 17

            ":/Components/Output/Led/LedOff.svg",        // 18
            ":/Components/Output/Led/RoyalLed.png",      // 19
            ":/Components/Output/Led/LimeGreenLed.png",  // 20
            ":/Components/Output/Led/AquaLightLed.png",  // 21
            ":/Components/Output/Led/RedLed.png",        // 22
            ":/Components/Output/Led/HotPinkLed.png",    // 23
            ":/Components/Output/Led/YellowLed.png",     // 24
            ":/Components/Output/Led/WhiteLed.png",      // 25
        });
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new Led(); });
        return true;
    }();
};

Led::Led(QGraphicsItem *parent)
    : GraphicElement(ElementType::Led, parent)
{
    // The skin list is indexed by colorIndex(). For a 1-input LED the index is
    // m_colorIndex + input_value, where m_colorIndex is set by setColor() to the
    // base offset for the chosen color (0=White, 2=Red, 4=Green, 6=Blue, 8=Purple).
    // Even indices are the Off state; odd indices are the On state.
    // Indices 10-25 cover multi-input (2/3/4-bit) color palettes (see comment below).
    m_defaultSkins = ElementMetadataRegistry::metadata(ElementType::Led).defaultSkins;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setHasColors(true);

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

    // Encode the state of all input ports as a binary number.
    // Bit 0 = port 0, bit 1 = port 1, etc. (LSB-first ordering).
    std::bitset<4> indexBit;

    for (int i = 0; i < inputSize(); ++i) {
        indexBit[static_cast<std::size_t>(i)] = (inputPort(i)->status() == Status::Active);
    }

    const int index = static_cast<int>(indexBit.to_ulong());

    int index2 = 0;

    // Map the encoded value to a skin list index depending on how many inputs
    // the LED has. Multi-input LEDs use a fixed color palette (indices 10-25);
    // single-input LEDs offset by m_colorIndex to pick the user-selected color.
    // For 2-input, index 3 (both on) maps to index 25 (white "on") rather than
    // index 21 (aqua) to preserve backward-compatible color behavior.
    switch (inputSize()) {
    case 1: index2 = m_colorIndex + index;           break;
    case 2: index2 = (index == 3) ? 25 : 18 + index; break;
    case 3: index2 = 18 + index;                     break;
    case 4: index2 = 10 + index;                     break;

    default:
        index2 = index;
        break;
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

    // m_colorIndex is the base offset into the single-input skin list.
    // Each color occupies two consecutive entries: [base]=Off, [base+1]=On.
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

void Led::load(QDataStream &stream, SerializationContext &context)
{
    GraphicElement::load(stream, context);

    if ((VersionInfo::hasClock(context.version)) && (!VersionInfo::hasQMapFormat(context.version))) {
        // v1.1–4.0 stored color as a bare QString
        QString color_; stream >> color_;
        setColor(color_);
    }

    if (VersionInfo::hasQMapFormat(context.version)) {
        // v4.1+ uses a key-value map
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
    // Color selection is only meaningful for single-input LEDs; with multiple
    // inputs the color is determined entirely by the input bit pattern.
    setHasColors(inputSize() == 1);

    for (auto *port : std::as_const(m_inputPorts)) {
        port->setName(QString::number(m_inputPorts.indexOf(port) + 1));
        // All inputs are optional — an unconnected port is treated as Inactive (0)
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
        // Replace only the skin for the currently active color/state, so other
        // color states continue to use their default images.
        m_alternativeSkins[index] = fileName;
    }

    m_usingDefaultSkin = useDefaultSkin;
    setPixmap(index);
}

void Led::updateLogic()
{
    updateInputs();
}

