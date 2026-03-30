// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/// \file
/// \brief Graphic element for the buzzer tone output.

#pragma once

#include <QVersionNumber>

#include "App/Element/GraphicElement.h"

class QAudioSink;
class ToneGenerator;

/// Audio output element that plays a continuous tone when its input is logic-1.
///
/// Uses a sine-wave ToneGenerator fed to QAudioSink for seamless,
/// glitch-free playback at any user-chosen frequency.
class Buzzer : public GraphicElement
{
    Q_OBJECT

public:
    /// Constructs a Buzzer element (default: 1047 Hz / C6).
    explicit Buzzer(QGraphicsItem *parent = nullptr);

    /// Copy-constructs by delegating to the parent item constructor.
    Buzzer(const Buzzer &other) : Buzzer(other.parentItem()) {}

    // --- State Queries ---

    /// Returns \c true if the buzzer sound is currently playing.
    bool isPlaying() const;
    /// Returns \c true if audio output is muted.
    bool isMuted() const;
    /// Returns the audio playback volume (0.0–1.0).
    float volume() const override;
    /// Returns the tone frequency in Hz.
    double frequency() const override;

    // --- Playback Control ---

    /// Mutes or unmutes playback according to \a mute.
    void mute(const bool mute = true);
    /// Sets the audio playback volume to \a vol (0.0–1.0).
    void setVolume(float vol) override;
    /// Sets the tone frequency in Hz.
    void setFrequency(double freq) override;
    /// Sets the tone by note name (e.g. "C6") for backward compatibility.
    void setAudio(const QString &note) override;
    /// Refreshes the visual appearance based on current state.
    void refresh() override;

    // --- Serialization ---

    /// \reimp
    void load(QDataStream &stream, SerializationContext &context) override;
    /// \reimp
    void save(QDataStream &stream) const override;

    /// Maps a note name (e.g. "C6") to its frequency in Hz. Returns 1047 for unknown notes.
    static int noteToFrequency(const QString &note);

private:
    // --- Internal methods ---

    void play();
    void stop();

    QAudioSink *m_sink = nullptr;
    ToneGenerator *m_generator = nullptr;
    double m_frequency = 1047.0;
    float m_volume = 0.35f;
    bool m_isPlaying = false;
    bool m_hasOutputDevice = false;
    bool m_muted = false;
};

