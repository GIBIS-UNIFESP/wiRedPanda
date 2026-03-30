// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Buzzer.h"

#include <QAudioDevice>
#include <QAudioSink>
#include <QMediaDevices>

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/Element/GraphicElements/ToneGenerator.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"
#include "App/Nodes/QNEPort.h"

template<>
struct ElementInfo<Buzzer> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Buzzer,
        .group = ElementGroup::Output,
        .minInputSize = 1,
        .maxInputSize = 1,
        .canChangeSkin = true,
        .hasAudio = true,
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
        meta.defaultSkins = QStringList({
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
    : GraphicElement(ElementType::Buzzer, parent)
{
    // 64,34: label sits to the right of the 64×64 body, vertically centred
    m_label->setPos(64, 34);

    // Check for audio hardware once at construction; subsequent audio calls are
    // guarded by m_hasOutputDevice to allow headless/CI operation without warnings.
    static const bool hasOutputDevice = !QMediaDevices::defaultAudioOutput().description().isEmpty();
    m_hasOutputDevice = hasOutputDevice;

    if (m_hasOutputDevice) {
        m_generator = new ToneGenerator(this);
        m_sink = new QAudioSink(ToneGenerator::format(), this);
        m_sink->setVolume(static_cast<qreal>(m_volume));
    }
}

void Buzzer::refresh()
{
    if (!isValid()) {
        stop();
        return;
    }

    const Status inputValue = m_inputPorts.constFirst()->status();

    (inputValue == Status::Active) ? play() : stop();
}

void Buzzer::setAudio(const QString &note)
{
    if (note.isEmpty()) {
        return;
    }

    m_note = note;

    if (!m_hasOutputDevice) {
        return;
    }

    m_generator->setFrequency(noteToFrequency(note));

    // If already playing, restart so the new frequency takes effect immediately
    if (m_isPlaying) {
        m_sink->stop();
        m_generator->start();
        m_sink->start(m_generator);
    }
}

QString Buzzer::audio() const
{
    return m_note;
}

bool Buzzer::isPlaying() const
{
    return m_isPlaying;
}

bool Buzzer::isMuted() const
{
    return m_muted;
}

float Buzzer::volume() const
{
    return m_volume;
}

void Buzzer::setVolume(float vol)
{
    m_volume = vol;
    if (m_hasOutputDevice && !m_muted) {
        m_sink->setVolume(static_cast<qreal>(vol));
    }
}

void Buzzer::mute(const bool mute)
{
    m_muted = mute;
    if (!m_hasOutputDevice) {
        return;
    }

    m_sink->setVolume(mute ? 0.0 : static_cast<qreal>(m_volume));
}

void Buzzer::play()
{
    if (m_isPlaying) {
        return;
    }

    setPixmap(1);

    if (m_hasOutputDevice) {
        if (m_note.isEmpty()) {
            setAudio("C6");
        }

        m_generator->start();
        m_sink->start(m_generator);
    }

    m_isPlaying = true;
}

void Buzzer::stop()
{
    if (!m_isPlaying) {
        return;
    }

    setPixmap(0);

    if (m_hasOutputDevice) {
        m_sink->stop();
        m_generator->close();
    }

    m_isPlaying = false;
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
    map.insert("note", audio());
    map.insert("volume", static_cast<double>(m_volume));

    stream << map;
}

void Buzzer::load(QDataStream &stream, SerializationContext &context)
{
    GraphicElement::load(stream, context);

    if (!VersionInfo::hasAudio(context.version)) {
        // Buzzer audio was added in v2.4; nothing to read for earlier files
        return;
    }

    if (!VersionInfo::hasQMapFormat(context.version)) {
        // v2.4–4.0 stored the note name as a bare QString
        QString note; stream >> note;
        setAudio(note);
    }

    if (VersionInfo::hasQMapFormat(context.version)) {
        // v4.1+ uses a key-value map for forward-compatible extensibility
        QMap<QString, QVariant> map; stream >> map;

        if (map.contains("note")) {
            setAudio(map.value("note").toString());
        }
        if (map.contains("volume")) {
            setVolume(map.value("volume").toFloat());
        }
    }
}

