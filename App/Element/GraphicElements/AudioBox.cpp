// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/AudioBox.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QAudioDeviceInfo>
#else
#include <QAudioDevice>
#include <QMediaDevices>
#endif

#include <QDir>

#include "App/Core/Common.h"
#include "App/Element/ElementInfo.h"
#include "App/Element/LogicElements/LogicSink.h"
#include "App/IO/Serialization.h"
#include "App/Nodes/QNEPort.h"
#include "App/Versions.h"

template<>
struct ElementInfo<AudioBox> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::AudioBox,
        .group = ElementGroup::Output,
        .minInputSize = 1,
        .maxInputSize = 1,
        .minOutputSize = 0,
        .maxOutputSize = 0,
        .canChangeSkin = true,
        .hasColors = false,
        .hasAudio = false,
        .hasAudioBox = true,
        .hasTrigger = false,
        .hasFrequency = false,
        .hasDelay = false,
        .hasLabel = true,
        .hasTruthTable = false,
        .rotatable = true,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Output/AudioBox/audioboxOff.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("AudioBox", "Audio Box");
        meta.translatedName = QT_TRANSLATE_NOOP("AudioBox", "Audio Box");
        meta.trContext = "AudioBox";
        meta.defaultSkins = QStringList({
            ":/Components/Output/AudioBox/audioboxOff.svg",
            ":/Components/Output/AudioBox/audioboxOn.svg",
        });
        meta.logicCreator = [](GraphicElement *elm) { return std::make_shared<LogicSink>(elm->inputSize()); };
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new AudioBox(); });
        return true;
    }();
};

AudioBox::AudioBox(QGraphicsItem *parent)
    : GraphicElement(ElementType::AudioBox, parent)
{
    // 64,34: label sits to the right of the 64×64 body, vertically centred
    m_label->setPos(64, 34);

    // Detect audio hardware at construction time; all subsequent audio calls are
    // guarded by m_hasOutputDevice so the element works silently in headless/CI environments.
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_hasOutputDevice = !QAudioDeviceInfo::defaultOutputDevice().deviceName().isEmpty();
#else
    m_hasOutputDevice = !QMediaDevices::defaultAudioOutput().description().isEmpty();
#endif

    m_audio.setFile(":/Components/Output/Audio/wiredpanda.wav");

    if (m_hasOutputDevice) {
        m_player = new QMediaPlayer(this);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        m_playlist = new QMediaPlaylist(this);
#else
        m_audioOutput = new QAudioOutput(this);
#endif
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
    if (audioPath.isEmpty()) {
        return;
    }

    // Resolve relative paths against the project directory so that projects
    // loaded from different locations find their audio files correctly.
    // Qt-resource paths (":/...") are always used as-is.
    QString resolvedPath = audioPath;
    if (!audioPath.startsWith(":/") && QDir::isRelativePath(audioPath)) {
        resolvedPath = Serialization::contextDir + "/" + audioPath;
    }

    if (!audioPath.startsWith(":/")) {
        const QFileInfo info(resolvedPath);
        if (!info.exists() || !info.isReadable()) {
            const QString reason = !info.exists()
                                       ? tr("File does not exist")
                                       : tr("File is not readable");
            qCDebug(zero) << "Problem loading audio path:" << resolvedPath;
            throw PANDACEPTION("Couldn't load audio: %1 (%2)", resolvedPath, reason);
        }
    }

    // Store the original (possibly relative) path for serialization portability.
    m_audio.setFile(audioPath);

    // Only set up hardware if device is available
    if (!m_hasOutputDevice) {
        return;
    }

    const QUrl mediaUrl = audioPath.startsWith(":/")
        ? QUrl(resolvedPath)
        : QUrl::fromLocalFile(resolvedPath);

    // Volume is set at 50% (Qt5: integer 0-100, Qt6: float 0.0-1.0)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_player->setVolume(50);
    m_playlist->clear();
    m_playlist->addMedia(mediaUrl);
    m_playlist->setPlaybackMode(QMediaPlaylist::Loop);
    m_player->setPlaylist(m_playlist);
#else
    m_audioOutput->setVolume(0.5f);
    m_player->setAudioOutput(m_audioOutput);
    m_player->setSource(mediaUrl);
    m_player->setLoops(QMediaPlayer::Infinite);
#endif

    // If already playing, restart playback with the new audio immediately
    if (m_isPlaying) {
        m_player->play();
    }
}

QString AudioBox::audio() const
{
    return m_audio.filePath();
}

bool AudioBox::isPlaying() const
{
    return m_isPlaying;
}

bool AudioBox::isMuted() const
{
    return m_muted;
}

void AudioBox::mute(const bool mute)
{
    m_muted = mute;
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
        if (!m_audio.exists()) {
            setAudio(":/Components/Output/Audio/wiredpanda.wav");
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

    QString audioPath = m_audio.filePath();

    // Store only the bare filename when the audio file lives inside the project
    // directory, so the project remains portable across machines and platforms.
    if (!audioPath.startsWith(":/")) {
        QFileInfo info(audioPath);
        if (info.absoluteDir() == QDir(Serialization::contextDir)) {
            audioPath = info.fileName();
        }
    }

    QMap<QString, QVariant> map;
    map.insert("audiobox", audioPath);

    stream << map;
}

void AudioBox::load(QDataStream &stream, SerializationContext &context)
{
    GraphicElement::load(stream, context);

    if (context.version < Versions::V_2_4) {
        // Audio was added in v2.4; nothing to read for earlier files
        return;
    }

    if (context.version < Versions::V_4_1) {
        // v2.4–4.0 stored the path as a bare QString
        QString audio; stream >> audio;
        setAudio(audio);
    }

    if (context.version >= Versions::V_4_1) {
        // v4.1+ uses a key-value map for forward-compatible extensibility
        QMap<QString, QVariant> map; stream >> map;

        if (map.contains("audiobox")) {
            setAudio(map.value("audiobox").toString());
        }
    }
}
