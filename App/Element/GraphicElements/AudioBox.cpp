// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QCoreApplication>
#include <QFileInfo>

#include "App/Core/Common.h"
#include "App/Element/AudioOutput.h"
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
                // there: a sound that is gone or unreadable keeps its path, and the built-in sound plays in its place
                // (AudioService), so an undo goes on; opening a file reports it (reportFailedFiles)
                .id = ParamId::AudioFile,
                .defaultValue = QString(AudioOutput::kBuiltInSound),
                .key = "audiobox",
                .accept = [](const Element &, const ParamValue &value) -> std::optional<ParamValue> {
                    const QString &path = std::get<QString>(value);
                    return path.isEmpty() ? std::nullopt : std::optional<ParamValue>(QFileInfo(path).filePath());
                },
                .fileReference = true,
            },
            AudioOutput::volumeSpec(AudioOutput::kBoxVolume),
            AudioOutput::playingSpec(),
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
        // A sound that cannot be read now, which plays the built-in sound in its place; resolution against the context
        // directory already happened at load (ExternalFilePath::forReading()), so a path here is a resource or one
        // directly usable
        descriptor.reportFailedFiles = [](const Element &element) {
            const QString path = element.audio();
            const QFileInfo info(path);
            if (path.startsWith(":/") || info.isReadable()) {
                return;
            }
            const QString reason = !info.exists() ? QCoreApplication::translate("AudioBox", "File does not exist")
                                                  : QCoreApplication::translate("AudioBox", "File is not readable");
            throw PANDACEPTION_WITH_CONTEXT("AudioBox", QT_TRANSLATE_NOOP("AudioBox", "Couldn't load audio: %1 (%2)"),
                                            path, reason);
        };
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Element(constraints.type); });
        return true;
    }();
};
