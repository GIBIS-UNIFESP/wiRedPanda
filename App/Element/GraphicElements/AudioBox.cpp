// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/AudioBox.h"

#include <QFileInfo>

#include "App/Core/Common.h"
#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/IO/ExternalFilePath.h"
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
        .hasLabel = true,
    };
    static_assert(validate(constraints));

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.pixmapPath = []{ return QStringLiteral(":/Components/Output/AudioBox/audioboxOff.svg"); };
        descriptor.titleText = QT_TRANSLATE_NOOP("AudioBox", "Audio Box");
        descriptor.translatedName = QT_TRANSLATE_NOOP("AudioBox", "Audio Box");
        descriptor.trContext = "AudioBox";
        descriptor.defaultAppearances = QStringList({
            ":/Components/Output/AudioBox/audioboxOff.svg",
            ":/Components/Output/AudioBox/audioboxOn.svg",
        });
        descriptor.parameters = {
            {
                // An empty path changes nothing; any other is kept as QFileInfo has it, whether or not its file is
                // there (loadAudio())
                .id = ParamId::AudioFile,
                .defaultValue = QString(AudioBox::kDefaultAudioPath),
                .key = "audiobox",
                .accept = [](const Element &, const ParamValue &value) -> std::optional<ParamValue> {
                    const QString &path = std::get<QString>(value);
                    return path.isEmpty() ? std::nullopt : std::optional<ParamValue>(QFileInfo(path).filePath());
                },
            },
            AudioOutputElement::volumeSpec(AudioOutputElement::kDefaultVolume),
        };
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
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

    loadAudio();
}

void AudioBox::parameterChanged(const ParamId id, const ParamValue &previous)
{
    AudioOutputElement::parameterChanged(id, previous);
    if (id == ParamId::AudioFile) {
        loadAudio();
    }
}

void AudioBox::takeAudio(const QString &audioPath)
{
    // The same path is loaded again: its file can have gone, or come back, since
    if (!audioPath.isEmpty() && !setParam(ParamId::AudioFile, audioPath)) {
        loadAudio();
    }
}

void AudioBox::loadAudio()
{
    // Resolution against contextDir already happened once, at load time (see
    // ExternalFilePath::forReading(), called from load() below) -- by the time a
    // path reaches here it's either a resource reference or already directly
    // usable (a fresh absolute path from a file picker, or an already-resolved
    // one read back from a saved project). Nothing here may throw: a snapshot's
    // restore runs inside QUndoStack, which an exception cannot pass on macOS, so a
    // sound that is gone keeps its path and the built-in sound plays in its place.
    QString path = audio();
    m_failedAudioPath.clear();
    if (!path.startsWith(":/")) {
        const QFileInfo info(path);
        if (!info.exists() || !info.isReadable()) {
            qCDebug(zero) << "Problem loading audio path:" << path;
            m_failedAudioPath = path;
            path = QString::fromLatin1(kDefaultAudioPath);
        }
    }

    // Only set up hardware if device is available
    if (!m_hasOutputDevice) {
        return;
    }

    const QUrl mediaUrl = path.startsWith(":/")
        ? QUrl("qrc" + path)
        : QUrl::fromLocalFile(path);

    m_audioOutput->setVolume(volume());
    m_player->setAudioOutput(m_audioOutput);
    m_player->setSource(mediaUrl);
    m_player->setLoops(QMediaPlayer::Infinite);

    // If already playing, restart playback with the new audio immediately
    if (m_isPlaying) {
        m_player->play();
    }
}

void AudioBox::startAudio()
{
    // Defensive fallback: startAudio() is only ever reached via play(), itself only reachable
    // once the constructor's loadAudio() call has already populated m_player's
    // source (m_hasOutputDevice never flips at runtime in production, so if that initial call
    // ran at all, the source can't have gone empty since). Kept as a belt-and-suspenders guard
    // in case a future loadAudio() failure path clears the source without repopulating it.
    if (m_player->source().isEmpty()) { // LCOV_EXCL_LINE
        loadAudio(); // LCOV_EXCL_LINE
    } // LCOV_EXCL_LINE
    // A sound whose file is back plays in place of the built-in one
    if (!m_failedAudioPath.isEmpty() && QFileInfo(m_failedAudioPath).isReadable()) {
        loadAudio();
    }
    m_player->play();
}

void AudioBox::stopAudio()
{
    m_player->stop();
}

void AudioBox::applyVolume()
{
    m_audioOutput->setVolume(volume());
}

void AudioBox::applyMute()
{
    m_audioOutput->setMuted(m_muted);
}

QStringList AudioBox::externalFiles() const
{
    QStringList result = Element::externalFiles();
    const QString audioPath = audio();
    if (!audioPath.isEmpty() && !audioPath.startsWith(":/")) {
        result.append(audioPath);
    }
    return result;
}

void AudioBox::reportFailedFiles() const
{
    Element::reportFailedFiles();
    // Reported only while it still cannot be read: a file back since plays once the box next plays
    const QFileInfo info(m_failedAudioPath);
    if (m_failedAudioPath.isEmpty() || info.isReadable()) {
        return;
    }

    const QString reason = !info.exists()
                               ? tr("File does not exist")
                               : tr("File is not readable");
    throw PANDACEPTION("Couldn't load audio: %1 (%2)", m_failedAudioPath, reason);
}

void AudioBox::save(QDataStream &stream, SerializationOptions options) const
{
    Element::save(stream, options);

    const bool slim = (options.purpose == SerializationPurpose::PortableFile);
    const QString audioPath = ExternalFilePath::forWriting(audio(), options.purpose);

    QMap<QString, QVariant> map;
    // PortableFile streams omit resource paths outright — forReading() discards
    // them on load anyway (the default source is the bundled kDefaultAudioPath
    // resource, which a fresh element already has), matching the appearance
    // rule in ElementSerializer::save(). Snapshots write unconditionally.
    if (!slim || !audioPath.startsWith(":/")) {
        map.insert("audiobox", audioPath);
    }
    if (!slim || volume() != kDefaultVolume) {
        map.insert("volume", static_cast<double>(volume()));
    }

    stream << map;
}

void AudioBox::load(QDataStream &stream, SerializationContext &context)
{
    Element::load(stream, context);

    if (!VersionInfo::hasAudio(context.version)) {
        // Audio was added in v2.4; nothing to read for earlier files
        return;
    }

    if (!VersionInfo::hasQMapFormat(context.version)) {
        // v2.4–4.0 stored the path as a bare QString
        const QString audio = Serialization::readBoundedString(stream);
        if (const auto resolved = ExternalFilePath::forReading(audio, context.contextDir, context.purpose)) {
            takeAudio(*resolved);
        }
    }

    if (VersionInfo::hasQMapFormat(context.version)) {
        // v4.1+ uses a key-value map for forward-compatible extensibility
        QMap<QString, QVariant> map = Serialization::readBoundedMetadata(stream);

        if (map.contains("audiobox")) {
            if (const auto resolved = ExternalFilePath::forReading(map.value("audiobox").toString(), context.contextDir, context.purpose)) {
                takeAudio(*resolved);
            }
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
