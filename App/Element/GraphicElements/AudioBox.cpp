// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/AudioBox.h"

#include <QDir>

#include "App/Core/Common.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/Element/GraphicElement.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"
#include "App/Wiring/Port.h"

template<>
struct ElementInfo<AudioBox> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::AudioBox,
        .group = ElementGroup::Output,
        .minInputSize = 1,
        .maxInputSize = 1,
        .canChangeAppearance = true,
        .hasAudioBox = true,
        .hasLabel = true,
        .hasVolume = true,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Output/AudioBox/audioboxOff.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("AudioBox", "Audio Box");
        meta.translatedName = QT_TRANSLATE_NOOP("AudioBox", "Audio Box");
        meta.trContext = "AudioBox";
        meta.defaultAppearances = QStringList({
            ":/Components/Output/AudioBox/audioboxOff.svg",
            ":/Components/Output/AudioBox/audioboxOn.svg",
        });
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new AudioBox(); });
        return true;
    }();
};

AudioBox::AudioBox(QGraphicsItem *parent)
    : AudioOutputElement(ElementType::AudioBox, parent)
{
    if (m_hasOutputDevice) {
        m_player = new QMediaPlayer(this);
        m_audioOutput = new QAudioOutput(this);
    }

    setAudio(":/Components/Output/Audio/wiredpanda.wav");
}

void AudioBox::setAudio(const QString &audioPath)
{
    if (audioPath.isEmpty()) {
        return;
    }

    // Qt resource paths start with ":/"; anything else is a filesystem path
    // relative to the project's working directory (where the .panda file lives).
    // Try the path as-is against contextDir first; if not found, fall back to
    // just the filename — handles cross-platform absolute paths from old files.
    QString path = audioPath;
    if (!path.startsWith(":/")) {
        const QDir dir(GraphicElement::resolveContextDir(this));
        const QString resolved = dir.filePath(path);

        if (!QFileInfo::exists(resolved)) {
            path = dir.filePath(QFileInfo(QString(path).replace('\\', '/')).fileName());
        } else {
            path = resolved;
        }

        const QFileInfo info(path);
        if (!info.exists() || !info.isReadable()) {
            const QString reason = !info.exists()
                                       ? tr("File does not exist")
                                       : tr("File is not readable");
            qCDebug(zero) << "Problem loading audio path:" << path;
            throw PANDACEPTION("Couldn't load audio: %1 (%2)", path, reason);
        }
    }

    m_audio.setFile(path);

    // Only set up hardware if device is available
    if (!m_hasOutputDevice) {
        return;
    }

    const QUrl mediaUrl = path.startsWith(":/")
        ? QUrl("qrc" + path)
        : QUrl::fromLocalFile(path);

    m_audioOutput->setVolume(m_volume);
    m_player->setAudioOutput(m_audioOutput);
    m_player->setSource(mediaUrl);
    m_player->setLoops(QMediaPlayer::Infinite);

    // If already playing, restart playback with the new audio immediately
    if (m_isPlaying) {
        m_player->play();
    }
}

QString AudioBox::audio() const
{
    return m_audio.filePath();
}

void AudioBox::startAudio()
{
    if (m_player->source().isEmpty()) {
        setAudio(":/Components/Output/Audio/wiredpanda.wav");
    }
    m_player->play();
}

void AudioBox::stopAudio()
{
    m_player->stop();
}

void AudioBox::applyVolume()
{
    m_audioOutput->setVolume(m_volume);
}

void AudioBox::applyMute()
{
    m_audioOutput->setMuted(m_muted);
}

QStringList AudioBox::externalFiles() const
{
    QStringList result = GraphicElement::externalFiles();
    const QString audioPath = m_audio.filePath();
    if (!audioPath.isEmpty() && !audioPath.startsWith(":/")) {
        result.append(audioPath);
    }
    return result;
}

void AudioBox::save(QDataStream &stream) const
{
    GraphicElement::save(stream);

    const QString &path = m_audio.filePath();
    QString audioPath = path;
    if (!path.startsWith(":/")) {
        // Truncating to a bare filename is only lossless if that name already resolves
        // next to the project; otherwise this is an in-memory-only snapshot (e.g. an
        // UpdateCommand undo/redo round-trip triggered by an unrelated property edit) or
        // an unsaved project, and shortening the path now would make it permanently
        // unresolvable — keep the full path instead.
        const QString contextDir = GraphicElement::resolveContextDir(this);
        const QString basename = m_audio.fileName();
        const bool alreadyResolvable = !contextDir.isEmpty()
            && QFileInfo(QDir(contextDir).filePath(basename)).exists();
        if (alreadyResolvable) {
            audioPath = basename;
        }
    }

    QMap<QString, QVariant> map;
    map.insert("audiobox", audioPath);
    map.insert("volume", static_cast<double>(m_volume));

    stream << map;
}

void AudioBox::load(QDataStream &stream, SerializationContext &context)
{
    GraphicElement::load(stream, context);

    if (!VersionInfo::hasAudio(context.version)) {
        // Audio was added in v2.4; nothing to read for earlier files
        return;
    }

    if (!VersionInfo::hasQMapFormat(context.version)) {
        // v2.4–4.0 stored the path as a bare QString
        const QString audio = Serialization::readBoundedString(stream);
        setAudio(audio);
    }

    if (VersionInfo::hasQMapFormat(context.version)) {
        // v4.1+ uses a key-value map for forward-compatible extensibility
        QMap<QString, QVariant> map = Serialization::readBoundedMetadata(stream);

        if (map.contains("audiobox")) {
            setAudio(map.value("audiobox").toString());
        }
        if (map.contains("volume")) {
            bool ok = false;
            const float vol = map.value("volume").toFloat(&ok);
            if (ok) {
                setVolume(vol);
            }
        }
    }
}
