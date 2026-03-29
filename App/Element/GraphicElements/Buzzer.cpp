// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Buzzer.h"

#include <QAudio>
#include <QAudioDevice>
#include <QDebug>
#include <QMediaDevices>

#include "App/GlobalProperties.h"
#include "App/Nodes/QNEPort.h"

Buzzer::Buzzer(QGraphicsItem *parent)
    : GraphicElement(ElementType::Buzzer, ElementGroup::Output, ":/Components/Output/Buzzer/BuzzerOff.svg", tr("BUZZER"), tr("Buzzer"), 1, 1, 0, 0, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins = QStringList{
        ":/Components/Output/Buzzer/BuzzerOff.svg",
        ":/Components/Output/Buzzer/BuzzerOn.svg"
    };
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    // 64,34: label sits to the right of the 64×64 body, vertically centred
    m_label->setPos(64, 34);

    // Check for audio hardware once at construction; subsequent audio calls are
    // guarded by m_hasOutputDevice to allow headless/CI operation without warnings.
    setCanChangeSkin(true);
    setHasAudio(true);
    setHasLabel(true);
    setRotatable(false);

    m_hasOutputDevice = !QMediaDevices::defaultAudioOutput().description().isEmpty();

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
    m_audio->setVolume(0.35f);

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

void Buzzer::load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version)
{
    GraphicElement::load(stream, portMap, version);

    if (version < VERSION("2.4")) {
        // Buzzer audio was added in v2.4; nothing to read for earlier files
        return;
    }

    if (version < VERSION("4.1")) {
        // v2.4–4.0 stored the note name as a bare QString
        QString note; stream >> note;
        setAudio(note);
    }

    if (version >= VERSION("4.1")) {
        // v4.1+ uses a key-value map for forward-compatible extensibility
        QMap<QString, QVariant> map; stream >> map;

        if (map.contains("note")) {
            setAudio(map.value("note").toString());
        }
    }
}

