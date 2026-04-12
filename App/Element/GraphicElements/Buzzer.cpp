// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Buzzer.h"

#include <QAudioSink>

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/Element/GraphicElements/ToneGenerator.h"
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
        .hasFrequency = true,
        .hasLabel = true,
        .hasVolume = true,
        .rotatable = false,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Output/Buzzer/BuzzerOff.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("Buzzer", "BUZZER");
        meta.translatedName = QT_TRANSLATE_NOOP("Buzzer", "Buzzer");
        meta.trContext = "Buzzer";
        meta.defaultAppearances = QStringList({
            ":/Components/Output/Buzzer/BuzzerOff.svg",
            ":/Components/Output/Buzzer/BuzzerOn.svg",
        });
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new Buzzer(); });
        return true;
    }();
};

Buzzer::Buzzer(QGraphicsItem *parent)
    : AudioOutputElement(ElementType::Buzzer, parent)
{
    // Buzzer default volume is lower than AudioBox to avoid harsh tones
    m_volume = 0.35f;

    if (m_hasOutputDevice) {
        m_generator = new ToneGenerator(this);
        m_generator->setFrequency(m_frequency);
        m_sink = new QAudioSink(ToneGenerator::format(), this);
        m_sink->setVolume(static_cast<qreal>(m_volume));
    }
}

double Buzzer::frequency() const
{
    return m_frequency;
}

void Buzzer::setFrequency(double freq)
{
    m_frequency = freq;

    if (!m_hasOutputDevice) {
        return;
    }

    m_generator->setFrequency(freq);

    // If already playing, restart so the new frequency takes effect immediately
    if (m_isPlaying) {
        m_sink->stop();
        m_generator->start();
        m_sink->start(m_generator);
    }
}

void Buzzer::setAudio(const QString &note)
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
        m_sink->setVolume(static_cast<qreal>(m_volume));
    }
}

void Buzzer::applyMute()
{
    m_sink->setVolume(m_muted ? 0.0 : static_cast<qreal>(m_volume));
}

int Buzzer::noteToFrequency(const QString &note)
{
    static const QHash<QString, int> map = {
        {"C6", 1047}, {"D6", 1175}, {"E6", 1319}, {"F6", 1397},
        {"G6", 1568}, {"A7", 3520}, {"B7", 3951}, {"C7", 2093},
    };

    return map.value(note, 1047);
}

void Buzzer::save(QDataStream &stream) const
{
    GraphicElement::save(stream);

    QMap<QString, QVariant> map;
    map.insert("frequency", m_frequency);
    map.insert("volume", static_cast<double>(m_volume));

    stream << map;
}

void Buzzer::load(QDataStream &stream, SerializationContext &context)
{
    GraphicElement::load(stream, context);

    if (!VersionInfo::hasAudio(context.version)) {
        return;
    }

    if (!VersionInfo::hasQMapFormat(context.version)) {
        // v2.4–4.0 stored the note name as a bare QString
        QString note; stream >> note;
        setAudio(note);
    }

    if (VersionInfo::hasQMapFormat(context.version)) {
        QMap<QString, QVariant> map; stream >> map;

        // New format: frequency in Hz
        if (map.contains("frequency")) {
            setFrequency(map.value("frequency").toDouble());
        } else if (map.contains("note")) {
            // Old format: note name → convert to frequency
            setAudio(map.value("note").toString());
        }

        if (map.contains("volume")) {
            setVolume(map.value("volume").toFloat());
        }
    }
}

