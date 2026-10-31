// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the AudioBox (external audio file player) output.
 */

#pragma once

#include <QAudioOutput>
#include <QMediaPlayer>

#include "App/Element/GraphicElements/AudioOutputElement.h"

template<typename T>
struct ElementInfo;

/**
 * \class AudioBox
 * \brief Audio output element that streams an external audio file when its input is logic-1.
 *
 * \details Uses QMediaPlayer to play arbitrary audio files (MP3, WAV, OGG, etc.).
 * Supports looping and can be globally muted.
 */
class AudioBox : public AudioOutputElement
{
    Q_OBJECT

    friend struct ElementInfo<AudioBox>; // its descriptor's every-load rule reads the sound file again

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
    // --- AudioOutputElement hooks ---

    void startAudio() override;
    void stopAudio() override;
    void applyVolume() override;
    void applyMute() override;

    /// Loads a new sound file (loadAudio()).
    void parameterChanged(ParamId id, const ParamValue &previous) override;

private:
    /**
     * \brief Loads the sound file, audio(), into the player.
     * \details A sound that is gone or unreadable keeps its path and plays the built-in sound in its place: an undo
     * goes on. It plays again once its file is back; opening a file reports it (reportFailedFiles()).
     */
    void loadAudio();

    QAudioOutput *m_audioOutput = nullptr;
    QString m_failedAudioPath;   ///< The sound last set when it did not load, else empty.
    QMediaPlayer *m_player = nullptr;
};
