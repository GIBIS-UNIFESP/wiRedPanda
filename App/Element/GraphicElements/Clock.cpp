// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <cmath>

#include <QCoreApplication>

#include "App/Element/ClockTiming.h"
#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/Element/InputLevel.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"

template<>
struct ElementInfo<ElementType::Clock> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Clock,
        .group = ElementGroup::Input,
        .minOutputSize = 1,
        .maxOutputSize = 1,
        .canChangeAppearance = true,
        .hasLabel = true,
        .rotatesGraphic = false,
    };
    static_assert(validate(constraints));

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.kernel = KernelKind::Clock;
        descriptor.kernelParameters = {ParamId::Frequency, ParamId::Delay, ParamId::Locked};
        descriptor.sourceLevel = InputLevel::onLevel;
        descriptor.setLevel = InputLevel::setOnLevel;
        descriptor.shownSlot = InputLevel::levelSlot;
        descriptor.pixmapPath = QStringLiteral(":/Components/Input/clock1.svg");
        descriptor.titleText = QT_TRANSLATE_NOOP("Clock", "CLOCK SIGNAL");
        descriptor.translatedName = QT_TRANSLATE_NOOP("Clock", "Clock");
        descriptor.trContext = "Clock";
        descriptor.appearanceSlots = {
            {"off", ":/Components/Input/clock0.svg"},
            {"on", ":/Components/Input/clock1.svg"},
        };
        // A clock's level follows the simulation's time: no state of the circuit
        descriptor.parameters = {
            InputLevel::levelSpec(nullptr),
            InputLevel::lockSpec(),
            {
                // A frequency so high that the half-period rounds to zero is refused too
                .id = ParamId::Frequency,
                .defaultValue = ClockTiming::kDefaultFrequency,
                .key = "frequency",
                .accept = [](const Element &, const ParamValue &value) -> std::optional<ParamValue> {
                    const double frequency = std::get<double>(value);
                    const bool takes = std::isfinite(frequency) && !qFuzzyIsNull(frequency)
                                       && (ClockTiming::halfPeriod(frequency).count() > 0);
                    return takes ? std::optional<ParamValue>(value) : std::nullopt;
                },
            },
            {
                // A fraction of the period in [-1, 1]: a finite value beyond it would multiply with the period in
                // ClockTiming::reset() to a magnitude beyond microseconds::rep, which is undefined on the cast
                .id = ParamId::Delay,
                .defaultValue = 0.0,
                .key = "delay",
                .range = {-1.0, 1.0},
                .accept = [](const Element &, const ParamValue &value) -> std::optional<ParamValue> {
                    const double delay = std::get<double>(value);
                    if (!std::isfinite(delay)) {
                        return std::nullopt;
                    }
                    return ParamValue(std::clamp(delay, -1.0, 1.0));
                },
                // The delay before 4.3 (VersionInfo::hasDelayFix()) had another meaning and is dropped
                .since = QVersionNumber(4, 3),
            },
        };
        descriptor.appearanceStates = [](const Element &) -> QList<std::pair<int, QString>> {
            return {{0, QCoreApplication::translate("Clock", "Low")}, {1, QCoreApplication::translate("Clock", "High")}};
        };
        descriptor.readBeforeMap = [](Element &element, QDataStream &stream, const SerializationContext &context) {
            // From 1.1 to 4.0 the frequency was a bare float, then from 3.1 the lock a bare bool
            if (!VersionInfo::hasClock(context.version)) {
                return;
            }
            float frequency = 0.0F;
            stream >> frequency;
            element.setParam(ParamId::Frequency, static_cast<double>(frequency));
            if (VersionInfo::hasLockState(context.version)) {
                bool locked = false;
                stream >> locked;
                element.setParam(ParamId::Locked, locked);
            }
        };
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Element(constraints.type); });
        return true;
    }();
};
