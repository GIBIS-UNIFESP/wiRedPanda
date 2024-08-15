// Copyright 2015 - 2024, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "audiobox.h"

#include "globalproperties.h"
#include "qneport.h"
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QAudioDeviceInfo>
#else
#include <QAudioDevice>
#include <QMediaDevices>
#endif
#include <QMessageBox>

namespace
{
    int id = qRegisterMetaType<AudioBox>();
}

AudioBox::AudioBox(QGraphicsItem* parent)
    : GraphicElement(ElementType::AudioBox, ElementGroup::Output, ":output/audiobox/audioboxOff.svg", tr("Audio Box"), tr("Audio Box"), 1, 1, 0, 0, parent)
{
    if (GlobalProperties::skipInit)
        return;

    m_defaultSkins = QStringList{
        ":/output/audiobox/audioboxOff.svg",
        ":/output/audiobox/audioboxOn.svg"
    };
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    m_label->setPos(64, 34);

    setCanChangeSkin(true);
    setHasAudioBox(true);
    setHasLabel(true);
    setRotatable(true);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_hasOutputDevice = !QAudioDeviceInfo::defaultOutputDevice().deviceName().isEmpty();
#else
    m_hasOutputDevice = !QMediaDevices::defaultAudioOutput().description().isEmpty();
#endif

    if (m_hasOutputDevice) {
        m_player = new QMediaPlayer;
        #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            m_playlist = new QMediaPlaylist;
        #else
            m_audioOutput = new QAudioOutput;
        #endif
        m_audio = new QFileInfo();
        AudioBox::setAudio("qrc:/output/audio/wiredpanda.wav");
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
    if (audioPath.isEmpty() || !m_hasOutputDevice) {
        return;
    }

    m_audio->setFile(audioPath);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_player->setVolume(50);
    m_playlist->addMedia(QUrl(audioPath));
    m_playlist->setPlaybackMode(QMediaPlaylist::Loop);
    m_player->setPlaylist(playlist);
#else
    m_audioOutput->setVolume(0.5f);
    m_player->setAudioOutput(m_audioOutput);
    m_player->setSource(QUrl(audioPath));
    m_player->setLoops(QMediaPlayer::Infinite);
#endif

}

QString AudioBox::audio() const
{
    return m_audio->fileName();
}

void AudioBox::mute(const bool mute)
{
    if (!m_hasOutputDevice) {
        return;
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_player->setMuted(mute);
#else
    m_audioOutput->setMuted(mute);
#endif
}

void AudioBox::play()
{
    if (m_isPlaying) {
        return;
    }

    setPixmap(1);

    if (m_hasOutputDevice) {
        if (!m_audio->exists()) {
            setAudio("qrc:/output/audio/wiredpanda.wav");
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
    map.insert("audiobox", m_audio->filePath());

    stream << map;
}

void AudioBox::load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version)
{
    GraphicElement::load(stream, portMap, version);

    if (version < VERSION("2.4")) {
        return;
    }

    if (version < VERSION("4.1")) {
        QString audio; stream >> audio;
        setAudio(audio);
    }

    if (version >= VERSION("4.1")) {
        QMap<QString, QVariant> map; stream >> map;

        if (map.contains("audiobox")) {
            setAudio(map.value("audiobox").toString());
        }
    }
}
