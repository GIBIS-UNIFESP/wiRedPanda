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

template<>
struct ElementInfo<ElementType::AudioBox> {
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
        descriptor.pixmapPath = QStringLiteral(":/Components/Output/AudioBox/audioboxOff.svg");
        descriptor.titleText = QT_TRANSLATE_NOOP("AudioBox", "Audio Box");
        descriptor.translatedName = QT_TRANSLATE_NOOP("AudioBox", "Audio Box");
        descriptor.trContext = "AudioBox";
        descriptor.appearanceSlots = {
            {"off", ":/Components/Output/AudioBox/audioboxOff.svg"},
            {"on", ":/Components/Output/AudioBox/audioboxOn.svg"},
        };
        descriptor.parameters = {
            {
                // An empty path changes nothing; any other is kept as QFileInfo has it, whether or not its file is
                // there (checkAudio())
                .id = ParamId::AudioFile,
                .defaultValue = QString(AudioBox::kDefaultAudioPath),
                .key = "audiobox",
                .accept = [](const Element &, const ParamValue &value) -> std::optional<ParamValue> {
                    const QString &path = std::get<QString>(value);
                    return path.isEmpty() ? std::nullopt : std::optional<ParamValue>(QFileInfo(path).filePath());
                },
                .fileReference = true,
            },
            AudioOutputElement::volumeSpec(AudioOutputElement::kDefaultVolume),
            AudioOutputElement::playingSpec(),
        };
        descriptor.readBeforeMap = [](Element &element, QDataStream &stream, const SerializationContext &context) {
            // From 2.4 to 4.0 the sound file was a bare path
            if (!VersionInfo::hasAudio(context.version)) {
                return;
            }
            const QString stored = Serialization::readBoundedString(stream);
            if (const auto path = ExternalFilePath::forReading(stored, context.contextDir, context.purpose)) {
                element.setParam(ParamId::AudioFile, *path);
            }
        };
        // Every load checks the sound file again: one restored with the path the box has can have gone, or come back,
        // since
        descriptor.loaded = [](Element &element) { static_cast<AudioBox &>(element).checkAudio(); };
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
    checkAudio();
}

void AudioBox::parameterChanged(const ParamId id, const ParamValue &previous)
{
    AudioOutputElement::parameterChanged(id, previous);
    if (id == ParamId::AudioFile) {
        checkAudio();
    }
}

void AudioBox::checkAudio()
{
    // Resolution against contextDir already happened once, at load time (ExternalFilePath::forReading()): a path here
    // is a resource, or one directly usable. Nothing here may throw: a snapshot's restore runs inside QUndoStack, which
    // an exception cannot pass on macOS, so a sound that is gone keeps its path.
    const QString path = audio();
    m_failedAudioPath.clear();
    if (path.startsWith(":/")) {
        return;
    }
    const QFileInfo info(path);
    if (!info.exists() || !info.isReadable()) {
        qCDebug(zero) << "Problem loading audio path:" << path;
        m_failedAudioPath = path;
    }
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
