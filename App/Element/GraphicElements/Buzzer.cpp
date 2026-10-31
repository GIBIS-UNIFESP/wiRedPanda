// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Buzzer.h"

#include <cmath>

#include <QAudioSink>

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/Element/GraphicElements/ToneGenerator.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"

template<>
struct ElementInfo<Buzzer> {
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
        descriptor.pixmapPath = []{ return QStringLiteral(":/Components/Output/Buzzer/BuzzerOff.svg"); };
        descriptor.titleText = QT_TRANSLATE_NOOP("Buzzer", "BUZZER");
        descriptor.translatedName = QT_TRANSLATE_NOOP("Buzzer", "Buzzer");
        descriptor.trContext = "Buzzer";
        descriptor.defaultAppearances = QStringList({
            ":/Components/Output/Buzzer/BuzzerOff.svg",
            ":/Components/Output/Buzzer/BuzzerOn.svg",
        });
        descriptor.parameters = {
            {
                // A file can hold a non-finite or non-positive value; ToneGenerator::readData() casts a NaN-derived
                // sample to qint16, which is undefined behaviour, and a non-positive frequency has no physical meaning
                // for a buzzer
                .id = ParamId::Frequency,
                .defaultValue = Buzzer::kDefaultFrequency,
                .key = "frequency",
                .accept = [](const Element &, const ParamValue &value) -> std::optional<ParamValue> {
                    const double frequency = std::get<double>(value);
                    return (std::isfinite(frequency) && (frequency > 0.0)) ? std::optional<ParamValue>(value)
                                                                           : std::nullopt;
                },
            },
            AudioOutputElement::volumeSpec(Buzzer::kDefaultVolume),
        };
        descriptor.readBeforeMap = [](Element &element, QDataStream &stream, const SerializationContext &context) {
            // From 2.4 to 4.0 the tone was a bare note name
            if (VersionInfo::hasAudio(context.version)) {
                static_cast<Buzzer &>(element).setNote(Serialization::readBoundedString(stream));
            }
        };
        // A map without a frequency can hold the note name the bare format had
        descriptor.readOlderKeys = [](Element &element, const QMap<QString, QVariant> &map) {
            if (!map.contains("frequency") && map.contains("note")) {
                static_cast<Buzzer &>(element).setNote(map.value("note").toString());
            }
        };
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Buzzer(); });
        return true;
    }();
};

Buzzer::Buzzer(QGraphicsItem *parent)
    : AudioOutputElement(ElementType::Buzzer, parent)
{
    if (m_hasOutputDevice) {
        m_generator = new ToneGenerator(this);
        m_generator->setFrequency(frequency());
        m_sink = new QAudioSink(ToneGenerator::format(), this);
        m_sink->setVolume(static_cast<qreal>(volume()));
    }
}

Buzzer::~Buzzer()
{
    delete m_sink;
    m_sink = nullptr;
}

void Buzzer::parameterChanged(const ParamId id, const ParamValue &previous)
{
    AudioOutputElement::parameterChanged(id, previous);
    if ((id != ParamId::Frequency) || !m_hasOutputDevice) {
        return;
    }

    m_generator->setFrequency(frequency());

    // If already playing, restart so the new frequency takes effect immediately
    if (m_isPlaying) {
        m_sink->stop();
        m_generator->start();
        m_sink->start(m_generator);
    }
}

void Buzzer::setNote(const QString &note)
{
    if (note.isEmpty()) {
        return;
    }

    setFrequency(noteToFrequency(note));
}

void Buzzer::startAudio()
{
    m_generator->start();
    m_sink->start(m_generator);
}

void Buzzer::stopAudio()
{
    m_sink->stop();
    m_generator->close();
}

void Buzzer::applyVolume()
{
    if (!m_muted) {
        m_sink->setVolume(static_cast<qreal>(volume()));
    }
}

void Buzzer::applyMute()
{
    m_sink->setVolume(m_muted ? 0.0 : static_cast<qreal>(volume()));
}

int Buzzer::noteToFrequency(const QString &note)
{
    // Backward-compatible aliases: the scale historically listed A7/B7 (an octave above
    // their neighbours); pre-4.x files persist the note *name*, so keep resolving these
    // to their true pitches. Kept on one physical line: gcov misattributes per-entry hit
    // counts across a multi-line brace-init list (see CodeGen's chained-.arg() precedent).
    static const QHash<QString, int> map = {{"C6", 1047}, {"D6", 1175}, {"E6", 1319}, {"F6", 1397}, {"G6", 1568}, {"A6", 1760}, {"B6", 1976}, {"C7", 2093}, {"A7", 3520}, {"B7", 3951}};

    return map.value(note, 1047);
}
