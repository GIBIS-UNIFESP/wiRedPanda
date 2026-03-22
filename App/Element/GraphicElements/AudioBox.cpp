// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/AudioBox.h"

#include <QAudioDevice>
#include <QMediaDevices>

#include "App/GlobalProperties.h"
#include "App/Nodes/QNEPort.h"

AudioBox::AudioBox(QGraphicsItem *parent)
    : GraphicElement(ElementType::AudioBox, ElementGroup::Output, ":/Components/Output/AudioBox/audioboxOff.svg", tr("Audio Box"), tr("Audio Box"), 1, 1, 0, 0, parent)
{
    if (GlobalProperties::skipInit)
        return;

    m_defaultSkins = QStringList{
        ":/Components/Output/AudioBox/audioboxOff.svg",
        ":/Components/Output/AudioBox/audioboxOn.svg"
    };
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    // 64,34: label sits to the right of the 64×64 body, vertically centred
    m_label->setPos(64, 34);

    // Detect audio hardware at construction time; all subsequent audio calls are
    // guarded by m_hasOutputDevice so the element works silently in headless/CI environments.
    setCanChangeSkin(true);
    setHasAudioBox(true);
    setHasLabel(true);
    setRotatable(true);

    m_hasOutputDevice = !QMediaDevices::defaultAudioOutput().description().isEmpty();

    if (m_hasOutputDevice) {
        m_player = new QMediaPlayer(this);
        m_audioOutput = new QAudioOutput(this);
        m_audio = new QFileInfo();
        AudioBox::setAudio("qrc:/Components/Output/Audio/wiredpanda.wav");
    }
}

void AudioBox::refresh()
{
    if (!isValid()) {
        stop();
        return;
    }

    const Status inputValue = m_inputPorts.constFirst()->status();

    (inputValue == Status::Active) ? play() : stop();
}

void AudioBox::setAudio(const QString &audioPath)
{
    if (audioPath.isEmpty() || !m_hasOutputDevice || !m_audio) {
        return;
    }

    m_audio->setFile(audioPath);

    m_audioOutput->setVolume(0.5f);
    m_player->setAudioOutput(m_audioOutput);
    m_player->setSource(QUrl(audioPath));
    m_player->setLoops(QMediaPlayer::Infinite);

    // If already playing, restart playback with the new audio immediately
    if (m_isPlaying) {
        m_player->play();
    }
}

QString AudioBox::audio() const
{
    return m_audio ? m_audio->fileName() : QString();
}

void AudioBox::mute(const bool mute)
{
    if (!m_hasOutputDevice) {
        return;
    }

    m_audioOutput->setMuted(mute);
}

void AudioBox::play()
{
    if (m_isPlaying) {
        return;
    }

    setPixmap(1);

    if (m_hasOutputDevice && m_audio) {
        if (!m_audio->exists()) {
            setAudio("qrc:/Components/Output/Audio/wiredpanda.wav");
        }
        m_player->play();
    }

    m_isPlaying = true;
}

void AudioBox::stop()
{
    if (!m_isPlaying) {
        return;
    }

    setPixmap(0);

    if (m_hasOutputDevice) {
        m_player->stop();
    }

    m_isPlaying = false;
}

void AudioBox::save(QDataStream &stream) const
{
    GraphicElement::save(stream);

    QMap<QString, QVariant> map;
    if (m_audio) {
        map.insert("audiobox", m_audio->filePath());
    }

    stream << map;
}

void AudioBox::load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version)
{
    GraphicElement::load(stream, portMap, version);

    if (version < VERSION("2.4")) {
        // Audio was added in v2.4; nothing to read for earlier files
        return;
    }

    if (version < VERSION("4.1")) {
        // v2.4–4.0 stored the path as a bare QString
        QString audio; stream >> audio;
        setAudio(audio);
    }

    if (version >= VERSION("4.1")) {
        // v4.1+ uses a key-value map for forward-compatible extensibility
        QMap<QString, QVariant> map; stream >> map;

        if (map.contains("audiobox")) {
            setAudio(map.value("audiobox").toString());
        }
    }
}

