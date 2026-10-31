// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Led.h"

#include <bitset>

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"
#include "App/Wiring/Port.h"

template<>
struct ElementInfo<Led> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Led,
        .group = ElementGroup::Output,
        .minInputSize = 1,
        .maxInputSize = 4,
        .canChangeAppearance = true,
        .hasLabel = true,
        .rotatesGraphic = false,
    };
    static_assert(validate(constraints));

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.pixmapPath = []{ return QStringLiteral(":/Components/Output/Led/LedOff.svg"); };
        descriptor.titleText = QT_TRANSLATE_NOOP("Led", "LED");
        descriptor.translatedName = QT_TRANSLATE_NOOP("Led", "LED");
        descriptor.trContext = "Led";
        descriptor.defaultAppearances = QStringList({
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
        // Any name is kept; one not in the list is drawn as White. With more than one input the inputs' code picks
        // the colour, so only a single-input LED offers it
        descriptor.parameters = {{
            .id = ParamId::Colour,
            .defaultValue = QString(Led::kDefaultColor),
            .key = "color",
            .offered = [](const Element &element) { return element.inputSize() == 1; },
        }};
        descriptor.readBeforeMap = [](Element &element, QDataStream &stream, const SerializationContext &context) {
            // From 1.1 to 4.0 the colour was a bare string
            if (VersionInfo::hasClock(context.version)) {
                element.setParam(ParamId::Colour, Serialization::readBoundedString(stream));
            }
        };
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Led(); });
        return true;
    }();
};

Led::Led(QGraphicsItem *parent)
    : Element(ElementType::Led, parent)
{
    // The appearance list is indexed by colorIndex(). For a 1-input LED the index is
    // m_colorIndex + input_value, where m_colorIndex is set by parameterChanged() to the
    // base offset for the chosen color (0=White, 2=Red, 4=Green, 6=Blue, 8=Purple).
    // Even indices are the Off state; odd indices are the On state.
    // Indices 10-25 cover multi-input (2/3/4-bit) color palettes (see comment below).
    m_appearance.seed(ElementTypeDescriptor::of(ElementType::Led).defaultAppearances, {}, {});
    setPixmap(0);

    updatePortsProperties();
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

    // Map the encoded value to an appearance list index depending on how many inputs
    // the LED has. Multi-input LEDs use a fixed color palette (indices 10-25);
    // single-input LEDs offset by m_colorIndex to pick the user-selected color.
    // For 2-input, index 3 (both on) maps to index 25 (white "on") rather than
    // index 21 (aqua) to preserve backward-compatible color behavior.
    switch (inputSize()) {
    case 1: index2 = m_colorIndex + index;           break;
    case 2: index2 = (index == 3) ? 25 : 18 + index; break;
    case 3: index2 = 18 + index;                     break;
    case 4: index2 = 10 + index;                     break;

    default: // LCOV_EXCL_LINE
        // Unreachable: Element::setPortSize() rejects any inputSize() request
        // outside [minInputSize(), maxInputSize()] = [1, 4] as a no-op, so inputSize()
        // can never legitimately be anything this switch doesn't already handle.
        index2 = index; // LCOV_EXCL_LINE
        break; // LCOV_EXCL_LINE
    }

    return index2;
}

void Led::refresh()
{
    setPixmap(colorIndex());
}

void Led::parameterChanged(const ParamId id, const ParamValue &previous)
{
    Element::parameterChanged(id, previous);
    if (id == ParamId::Colour) {
        // m_colorIndex is the base offset into the single-input appearance list.
        // Each color occupies two consecutive entries: [base]=Off, [base+1]=On.
        m_colorIndex = 2 * colorNameToIndex(color());
    }
}

QString Led::genericProperties()
{
    return color();
}

std::optional<Element::PortAttributes> Led::inputAttributes(const int index) const
{
    // The inputs are numbered from 1, optional, and off when unconnected
    return PortAttributes{.name = QString::number(index + 1), .required = false, .defaultStatus = Status::Inactive};
}

void Led::applyAppearance(const bool useDefaultAppearance, const QString &fileName)
{
    const int index = colorIndex();

    if (useDefaultAppearance) {
        m_appearance.resetAlternativeToDefault();
    } else {
        // Replace only the appearance for the currently active color/state, so other
        // color states continue to use their default images.
        m_appearance.setAlternativeAppearanceAt(index, fileName);
    }

    m_appearance.setUsingDefaultAppearance(useDefaultAppearance);
    setPixmap(index);
}

QList<std::pair<int, QString>> Led::appearanceStates() const
{
    QList<std::pair<int, QString>> states;

    // Builds "Port 1=0, Port 2=1, ..." from the bits of `code` (bit 0 = port 1, matching
    // colorIndex()'s bit ordering and the "1"/"2"/... port names set in updatePortsProperties()).
    auto portStateLabel = [](const int numPorts, const int code) {
        QStringList parts;
        for (int port = 0; port < numPorts; ++port) {
            parts << tr("Port %1=%2").arg(port + 1).arg((code >> port) & 1);
        }
        return parts.join(QStringLiteral(", "));
    };

    switch (inputSize()) {
    case 1:
        states.append({m_colorIndex, tr("Off")});
        states.append({m_colorIndex + 1, tr("On")});
        break;

    case 2:
        for (int i = 0; i < 4; ++i) {
            const int listIndex = (i == 3) ? 25 : 18 + i; // preserves colorIndex()'s special-case mapping
            states.append({listIndex, portStateLabel(2, i)});
        }
        break;

    case 3:
        for (int i = 0; i < 8; ++i) {
            states.append({18 + i, portStateLabel(3, i)});
        }
        break;

    case 4:
        for (int i = 0; i < 16; ++i) {
            states.append({10 + i, portStateLabel(4, i)});
        }
        break;

    default: // LCOV_EXCL_LINE
        // Unreachable for the same reason as colorIndex()'s default case above:
        // inputSize() can never be outside [1, 4].
        states.append({0, tr("Default")}); // LCOV_EXCL_LINE
        break; // LCOV_EXCL_LINE
    }

    return states;
} // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned QList<std::pair<int, QString>>, never reached after the return above.

void Led::updateLogic()
{
    if (!simUpdateInputs()) {
        return;
    }
    for (int i = 0; i < simInputs().size(); ++i) {
        setOutputValue(i, simInputs().at(i));
    }
}
