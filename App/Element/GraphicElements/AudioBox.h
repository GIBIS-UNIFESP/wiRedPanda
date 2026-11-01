// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the AudioBox (external audio file player) output.
 */

#pragma once

#include "App/Element/GraphicElements/AudioOutputElement.h"

template<typename T>
struct ElementInfo;

/**
 * \class AudioBox
 * \brief Audio output element that plays a sound file, looping, while its input is logic-1 (AudioService).
 *
 * \details A file the application's media backend reads: MP3, WAV, OGG and others.
 */
class AudioBox : public AudioOutputElement
{
    Q_OBJECT

    friend struct ElementInfo<AudioBox>; // its descriptor's every-load rule checks the sound file again

public:
    /// Default audio source: the bundled sample resource, NOT an empty path.
    /// Single source of truth: used by the sound file's spec and
    /// serialization's default-elision check.
    static constexpr const char *kDefaultAudioPath = ":/Components/Output/Audio/wiredpanda.wav";

    /// Constructs an AudioBox element.
    explicit AudioBox(QGraphicsItem *parent = nullptr);

    /// Copy-constructs by delegating to the parent item constructor.
    AudioBox(const AudioBox &other) : AudioBox(other.parentItem()) {}

    // --- External file dependencies ---

    /// \reimp Returns audio file path in addition to appearances.
    QStringList externalFiles() const override;

    /// \reimp Also names the sound when it did not load and the built-in sound plays in its place.
    void reportFailedFiles() const override;

protected:
    /// Checks a new sound file (checkAudio()).
    void parameterChanged(ParamId id, const ParamValue &previous) override;

private:
    /**
     * \brief Checks that the sound file, audio(), can be read.
     * \details A sound that is gone or unreadable keeps its path, and the built-in sound plays in its place: an undo
     * goes on. It plays again once its file is back; opening a file reports it (reportFailedFiles()).
     */
    void checkAudio();

    QString m_failedAudioPath; ///< The sound last set when it could not be read, else empty.
};
