// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <bitset>

#include <QCoreApplication>

#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"
#include "App/Wiring/Port.h"

template<>
struct ElementInfo<ElementType::Led> {
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

    /* Colour palettes, one picture for each code of the inputs, the first input its lowest bit:
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

    /// The slot \a element shows: a single input's off or on picture of its colour (an even slot and the next), or the
    /// code of two to four inputs in the palettes' slots.
    static int shownSlot(const Element &element)
    {
        const QVector<InputPort *> &inputs = element.inputs();
        if (!std::all_of(inputs.cbegin(), inputs.cend(), [](const InputPort *input) { return input->isValid(); })) {
            return 0;
        }

        // The inputs' levels as a binary number, the first input its lowest bit
        std::bitset<4> indexBit;
        for (int i = 0; i < element.inputSize(); ++i) {
            indexBit[static_cast<std::size_t>(i)] = (element.inputPort(i)->status() == Status::Active);
        }
        const int index = static_cast<int>(indexBit.to_ulong());

        // Each colour has two slots, off then on: White's at 0, Red's at 2, Green's at 4, Blue's at 6, Purple's at 8.
        // The palettes are slots 10 to 25; two inputs both on show slot 25 (white), not 21, as they always have.
        switch (element.inputSize()) {
        case 1: return (2 * Element::colorNameToIndex(element.color())) + index;
        case 2: return (index == 3) ? 25 : 18 + index;
        case 3: return 18 + index;
        case 4: return 10 + index;

        default: // LCOV_EXCL_LINE
            // Unreachable: Element::setPortSize() keeps inputSize() within [1, 4]
            return index; // LCOV_EXCL_LINE
        }
    }

    /// The states a custom picture can be chosen for: a single input's off and on, or each code of the inputs, named
    /// by the ports' levels ("Port 1=0, Port 2=1"), at the slots shownSlot() gives.
    static QList<std::pair<int, QString>> appearanceStates(const Element &element)
    {
        QList<std::pair<int, QString>> states;

        auto portStateLabel = [](const int numPorts, const int code) {
            QStringList parts;
            for (int port = 0; port < numPorts; ++port) {
                parts << QCoreApplication::translate("Led", "Port %1=%2").arg(port + 1).arg((code >> port) & 1);
            }
            return parts.join(QStringLiteral(", "));
        };

        switch (element.inputSize()) {
        case 1: {
            const int offset = 2 * Element::colorNameToIndex(element.color());
            states.append({offset, QCoreApplication::translate("Led", "Off")});
            states.append({offset + 1, QCoreApplication::translate("Led", "On")});
            break;
        }

        case 2:
            for (int i = 0; i < 4; ++i) {
                const int slot = (i == 3) ? 25 : 18 + i; // shownSlot()'s mapping
                states.append({slot, portStateLabel(2, i)});
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
            // Unreachable, as in shownSlot()
            states.append({0, QCoreApplication::translate("Led", "Default")}); // LCOV_EXCL_LINE
            break; // LCOV_EXCL_LINE
        }

        return states;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned QList, never reached after the return above.

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.pixmapPath = QStringLiteral(":/Components/Output/Led/LedOff.svg");
        descriptor.titleText = QT_TRANSLATE_NOOP("Led", "LED");
        descriptor.translatedName = QT_TRANSLATE_NOOP("Led", "LED");
        descriptor.trContext = "Led";
        // One input: an off and an on picture for each colour. More inputs: a picture for each code, the first input its
        // lowest bit; four inputs show their code, three their code plus 8, two their code plus 8 but both on code 15
        descriptor.appearanceSlots = {
            {"white-off", ":/Components/Output/Led/LedOff.svg"},
            {"white-on", ":/Components/Output/Led/WhiteLed.svg"},
            {"red-off", ":/Components/Output/Led/LedOff.svg"},
            {"red-on", ":/Components/Output/Led/RedLed.svg"},
            {"green-off", ":/Components/Output/Led/LedOff.svg"},
            {"green-on", ":/Components/Output/Led/GreenLed.svg"},
            {"blue-off", ":/Components/Output/Led/LedOff.svg"},
            {"blue-on", ":/Components/Output/Led/BlueLed.svg"},
            {"purple-off", ":/Components/Output/Led/LedOff.svg"},
            {"purple-on", ":/Components/Output/Led/PurpleLed.svg"},
            {"code-0", ":/Components/Output/Led//BlackLed.png"},
            {"code-1", ":/Components/Output/Led//NavyBlueLed.png"},
            {"code-2", ":/Components/Output/Led//GreenLed.png"},
            {"code-3", ":/Components/Output/Led//TealLed.png"},
            {"code-4", ":/Components/Output/Led//DarkRedLed.png"},
            {"code-5", ":/Components/Output/Led//MagentaLed.png"},
            {"code-6", ":/Components/Output/Led//OrangeLed.png"},
            {"code-7", ":/Components/Output/Led//LightGrayLed.png"},
            {"code-8", ":/Components/Output/Led/LedOff.svg"},
            {"code-9", ":/Components/Output/Led/RoyalLed.png"},
            {"code-10", ":/Components/Output/Led/LimeGreenLed.png"},
            {"code-11", ":/Components/Output/Led/AquaLightLed.png"},
            {"code-12", ":/Components/Output/Led/RedLed.png"},
            {"code-13", ":/Components/Output/Led/HotPinkLed.png"},
            {"code-14", ":/Components/Output/Led/YellowLed.png"},
            {"code-15", ":/Components/Output/Led/WhiteLed.png"},
        };
        // A file keeps any name, one not among the choices drawn as White; MCP sets the choices alone. With more than
        // one input the inputs' code picks the colour, so only a single-input LED offers it
        descriptor.parameters = {
            {
                .id = ParamId::Colour,
                .defaultValue = QStringLiteral("White"),
                .key = "color",
                .choices = Element::colorNames(),
                .offered = [](const Element &element) { return element.inputSize() == 1; },
            },
            // The slot shown changes as refresh() reads the inputs, not as the colour or the inputs change
            {.id = ParamId::Lit, .defaultValue = 0, .showsPicture = true},
        };
        descriptor.readBeforeMap = [](Element &element, QDataStream &stream, const SerializationContext &context) {
            // From 1.1 to 4.0 the colour was a bare string
            if (VersionInfo::hasClock(context.version)) {
                element.setParam(ParamId::Colour, Serialization::readBoundedString(stream));
            }
        };
        descriptor.inputAttributes = [](const Element &element, const int index) {
            Q_UNUSED(element)
            // The inputs are numbered from 1, optional, and off when unconnected
            return std::optional<PortAttributes>(PortAttributes{
                .name = QString::number(index + 1),
                .required = false,
                .defaultStatus = Status::Inactive,
            });
        };
        descriptor.shownSlot = shownSlot;
        descriptor.slotFollowsInputs = true;
        descriptor.appearanceStates = appearanceStates;
        descriptor.genericProperties = [](const Element &element) { return element.color(); };
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Element(constraints.type); });
        return true;
    }();
};
