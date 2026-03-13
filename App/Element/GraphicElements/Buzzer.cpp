// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Buzzer.h"

#include <QAudio>
#include <QDebug>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QAudioDeviceInfo>
#else
#include <QAudioDevice>
#include <QMediaDevices>
#endif

#include "App/Element/ElementInfo.h"
#include "App/Element/LogicElements/LogicOutput.h"
#include "App/Nodes/QNEPort.h"
#include "App/Versions.h"

template<>
struct ElementInfo<Buzzer> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Buzzer,
        .group = ElementGroup::Output,
        .minInputSize = 1,
        .maxInputSize = 1,
        .minOutputSize = 0,
        .maxOutputSize = 0,
        .canChangeSkin = true,
        .hasColors = false,
        .hasAudio = true,
        .hasAudioBox = false,
        .hasTrigger = false,
        .hasFrequency = false,
        .hasDelay = false,
        .hasLabel = true,
        .hasTruthTable = false,
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
        meta.logicCreator = [](GraphicElement *elm) { return std::make_shared<LogicOutput>(elm->inputSize()); };
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_hasOutputDevice = !QAudioDeviceInfo::defaultOutputDevice().deviceName().isEmpty();
#else
    m_hasOutputDevice = !QMediaDevices::defaultAudioOutput().description().isEmpty();
#endif

    if (m_hasOutputDevice) {
        m_audio = new QSoundEffect(this);
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

    // Store the note name (e.g. "C6") separately from the file path so it
    // can be serialized and displayed in the properties panel
    m_note = note;

    if (!m_hasOutputDevice) {
        return;
    }

    // Volume at 35% to avoid startling users; WAV files reside in Qt resources
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_audio->setVolume(0.35);
#else
    m_audio->setVolume(0.35f);
#endif

    m_audio->setSource(QUrl::fromLocalFile(":/Components/Output/Audio/" + note + ".wav"));
    m_audio->setLoopCount(QSoundEffect::Infinite); // TODO: fix audio clipping when repeating
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

void Buzzer::mute(const bool mute)
{
    m_muted = mute;
    if (!m_hasOutputDevice) {
        return;
    }

    m_audio->setMuted(mute);
}

void Buzzer::play()
{
    if (m_isPlaying) {
        return;
    }

    setPixmap(1);

    if (m_hasOutputDevice) {
        // Default to middle-C octave 6 if no note was configured
        if (m_audio->source().isEmpty()) {
            setAudio("C6");
        }

        m_audio->play();
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
        m_audio->stop();
    }

    m_isPlaying = false;
}

void Buzzer::save(QDataStream &stream) const
{
    GraphicElement::save(stream);

    QMap<QString, QVariant> map;
    map.insert("note", audio());

    stream << map;
}

void Buzzer::load(QDataStream &stream, SerializationContext &context)
{
    GraphicElement::load(stream, context);

    if (context.version < Versions::V_2_4) {
        // Buzzer audio was added in v2.4; nothing to read for earlier files
        return;
    }

    if (context.version < Versions::V_4_1) {
        // v2.4–4.0 stored the note name as a bare QString
        QString note; stream >> note;
        setAudio(note);
    }

    if (context.version >= Versions::V_4_1) {
        // v4.1+ uses a key-value map for forward-compatible extensibility
        QMap<QString, QVariant> map; stream >> map;

        if (map.contains("note")) {
            setAudio(map.value("note").toString());
        }
    }
}
