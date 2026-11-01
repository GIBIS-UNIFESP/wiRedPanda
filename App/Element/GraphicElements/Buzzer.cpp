// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cmath>

#include "App/Element/AudioOutput.h"
#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"

template<>
struct ElementInfo<ElementType::Buzzer> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Buzzer,
        .group = ElementGroup::Output,
        .minInputSize = 1,
        .maxInputSize = 1,
        .canChangeAppearance = true,
        .hasLabel = true,
        .rotatesGraphic = false,
    };
    static_assert(validate(constraints));

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.pixmapPath = QStringLiteral(":/Components/Output/Buzzer/BuzzerOff.svg");
        descriptor.titleText = QT_TRANSLATE_NOOP("Buzzer", "BUZZER");
        descriptor.translatedName = QT_TRANSLATE_NOOP("Buzzer", "Buzzer");
        descriptor.trContext = "Buzzer";
        descriptor.appearanceSlots = {
            {"off", ":/Components/Output/Buzzer/BuzzerOff.svg"},
            {"on", ":/Components/Output/Buzzer/BuzzerOn.svg"},
        };
        descriptor.parameters = {
            {
                // A file can hold a non-finite or non-positive value; ToneGenerator::readData() casts a NaN-derived
                // sample to qint16, which is undefined behaviour, and a non-positive frequency has no physical meaning
                // for a buzzer
                .id = ParamId::Frequency,
                .defaultValue = AudioOutput::kBuzzerFrequency,
                .key = "frequency",
                .accept = [](const Element &, const ParamValue &value) -> std::optional<ParamValue> {
                    const double frequency = std::get<double>(value);
                    return (std::isfinite(frequency) && (frequency > 0.0)) ? std::optional<ParamValue>(value)
                                                                           : std::nullopt;
                },
            },
            AudioOutput::volumeSpec(AudioOutput::kBuzzerVolume),
            AudioOutput::playingSpec(),
        };
        descriptor.readBeforeMap = [](Element &element, QDataStream &stream, const SerializationContext &context) {
            // From 2.4 to 4.0 the tone was a bare note name
            if (VersionInfo::hasAudio(context.version)) {
                AudioOutput::setNote(element, Serialization::readBoundedString(stream));
            }
        };
        // A map without a frequency can hold the note name the bare format had
        descriptor.readOlderKeys = [](Element &element, const QMap<QString, QVariant> &map) {
            if (!map.contains("frequency") && map.contains("note")) {
                AudioOutput::setNote(element, map.value("note").toString());
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
