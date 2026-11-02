// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QHash>
#include <QList>
#include <QPointF>

#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/Element/InputLevel.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"

namespace InputRotaryDetail {

/// Whether output \a port of \a element is on: the one its position selects.
bool positionLevel(const Element &element, const int port)
{
    return element.param<int>(ParamId::Position) == port;
}

/// Selects output \a port of \a element on a high \a level; a low one is no selection, since the rotary selects one
/// output at a time and deselects the others itself.
void selectOnLevel(Element &element, const bool level, const int port)
{
    if (level) {
        element.setParam(ParamId::Position, port);
    }
}

/// Sets the selection of \a element again from its outputs' values, one-hot, as a restore of its simulation state put
/// them back; no output on, the selection stays.
void selectionFromOutputs(Element &element)
{
    for (int port = 0; port < element.outputSize(); ++port) {
        if (element.outputValue(port) == Status::Active) {
            element.restoreParam(ParamId::Position, port);
            return;
        }
    }
}

/// The attributes of output \a index of \a element: its place around the dial and its name, for the output count.
std::optional<PortAttributes> outputAttributes(const Element &element, const int index)
{
    // Ports are placed around the perimeter of the 64x64 bounding box,
    // evenly distributed to match the visual rotary positions on the SVG.
    // Only specific output counts (2, 3, 4, 6, 8, 10, 12, 16) have defined
    // layouts; these correspond to standard rotary switch configurations.
    // Ports >= 10 use hex labels (A, B, ...) to stay single-character.
    struct Place {
        int x;
        int y;
        const char *name;
    };
    static const QHash<int, QList<Place>> layouts{
        {2, {{32, 0, "0"}, {32, 64, "1"}}},
        {3, {{32, 0, "0"}, {64, 48, "1"}, {0, 48, "2"}}},
        {4, {{32, 0, "0"}, {64, 32, "1"}, {32, 64, "2"}, {0, 32, "3"}}},
        {6, {{32, 0, "0"}, {64, 16, "1"}, {64, 48, "2"}, {32, 64, "3"}, {0, 48, "4"}, {0, 16, "5"}}},
        {8, {{32, 0, "0"}, {64, 0, "1"}, {64, 32, "2"}, {64, 64, "3"}, {32, 64, "4"}, {0, 64, "5"}, {0, 32, "6"}, {0, 0, "7"}}},
        {10, {{32, 0, "0"}, {56, 0, "1"}, {64, 24, "2"}, {64, 40, "3"}, {56, 64, "4"}, {32, 64, "5"}, {8, 64, "6"}, {0, 40, "7"}, {0, 24, "8"}, {8, 0, "9"}}},
        {12, {{32, 0, "0"}, {48, 0, "1"}, {64, 16, "2"}, {64, 32, "3"}, {64, 48, "4"}, {48, 64, "5"}, {32, 64, "6"}, {16, 64, "7"}, {0, 48, "8"}, {0, 32, "9"}, {0, 16, "A"}, {16, 0, "B"}}},
        {16, {{32, 0, "0"}, {48, 0, "1"}, {64, 0, "2"}, {64, 16, "3"}, {64, 32, "4"}, {64, 48, "5"}, {64, 64, "6"}, {48, 64, "7"}, {32, 64, "8"}, {16, 64, "9"}, {0, 64, "A"}, {0, 48, "B"}, {0, 32, "C"}, {0, 16, "D"}, {0, 0, "E"}, {16, 0, "F"}}},
    };

    if (const auto layout = layouts.constFind(element.outputSize()); layout != layouts.constEnd()) {
        if ((index < 0) || (index >= layout->size())) {
            return std::nullopt;
        }
        const Place &place = layout->at(index);
        return PortAttributes{.name = QString::fromLatin1(place.name), .position = QPointF(place.x, place.y)};
    }

    // Handle unexpected output sizes gracefully: for unhandled sizes, use a simple default positioning
    return PortAttributes{.name = QString::number(index + 1), .position = QPointF(32, index * 16)};
}

} // namespace InputRotaryDetail

template<>
struct ElementInfo<ElementType::InputRotary> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::InputRotary,
        .group = ElementGroup::Input,
        .minOutputSize = 2,
        .maxOutputSize = 16,
        .canChangeAppearance = true,
        .hasTrigger = true,
        .hasLabel = true,
    };
    static_assert(validate(constraints));

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.kernel = KernelKind::Input;
        descriptor.kernelParameters = {ParamId::Position};
        descriptor.sourceLevel = InputRotaryDetail::positionLevel;
        descriptor.setLevel = InputRotaryDetail::selectOnLevel;
        descriptor.levelsRestored = InputRotaryDetail::selectionFromOutputs;
        descriptor.outputAttributes = InputRotaryDetail::outputAttributes;
        descriptor.pixmapPath = QStringLiteral(":/Components/Input/rotary_icon.svg");
        descriptor.titleText = QT_TRANSLATE_NOOP("InputRotary", "ROTARY SWITCH");
        descriptor.translatedName = QT_TRANSLATE_NOOP("InputRotary", "Rotary Switch");
        descriptor.trContext = "InputRotary";
        // The arrow over the selected position is drawn by the presenter; an older file's second position named it
        descriptor.appearanceSlots = {{"body", ":/Components/Input/rotary.svg"}};
        descriptor.appearancePositions = {"body", "arrow"};
        // A selection out of the outputs' range is the first output, as a file's loads and as fewer outputs leave it;
        // the outputs drive it. It is the simulation's state too: a reset of that selects the first output again
        descriptor.parameters = {
            {
                .id = ParamId::Position,
                .defaultValue = 0,
                .key = "currentPort",
                .accept = [](const Element &element, const ParamValue &value) -> std::optional<ParamValue> {
                    const int position = std::get<int>(value);
                    return ((position >= 0) && (position < element.outputSize())) ? value : ParamValue(0);
                },
                .drivesOutputs = true,
                .followsPorts = true,
                .resetsWithSimulation = true,
            },
            InputLevel::lockSpec(),
        };
        descriptor.readBeforeMap = [](Element &element, QDataStream &stream, const SerializationContext &context) {
            // Before 4.1 the selection was a bare int, then from 3.1 the lock a bare bool
            int position = 0;
            stream >> position;
            element.setParam(ParamId::Position, position);
            if (VersionInfo::hasLockState(context.version)) {
                bool locked = false;
                stream >> locked;
                element.setParam(ParamId::Locked, locked);
            }
        };
        // A press steps the dial to the next output, the last one back to the first
        descriptor.pressOperation = [](Element &element) {
            element.setParam(ParamId::Position, (element.param<int>(ParamId::Position) + 1) % element.outputSize());
        };
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Element(constraints.type); });
        return true;
    }();
};
