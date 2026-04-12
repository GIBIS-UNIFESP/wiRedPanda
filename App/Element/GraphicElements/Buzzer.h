// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/// \file
/// \brief Graphic element for the buzzer tone output.

#pragma once

#include "App/Element/GraphicElements/AudioOutputElement.h"

class QAudioSink;
class ToneGenerator;

/// Audio output element that plays a continuous tone when its input is logic-1.
///
/// Uses a sine-wave ToneGenerator fed to QAudioSink for seamless,
/// glitch-free playback at any user-chosen frequency.
class Buzzer : public AudioOutputElement
{
    Q_OBJECT

public:
    /// Constructs a Buzzer element (default: 1047 Hz / C6).
    explicit Buzzer(QGraphicsItem *parent = nullptr);

    /// Copy-constructs by delegating to the parent item constructor.
    Buzzer(const Buzzer &other) : Buzzer(other.parentItem()) {}

    // --- State Queries ---

    /// Returns the tone frequency in Hz.
    double frequency() const override;

    // --- Playback Control ---

    /// Sets the tone frequency in Hz.
    void setFrequency(double freq) override;
    /// Sets the tone by note name (e.g. "C6") for backward compatibility.
    void setAudio(const QString &note) override;

    // --- Serialization ---

    /// \reimp
    void load(QDataStream &stream, SerializationContext &context) override;
    /// \reimp
    void save(QDataStream &stream) const override;

    /// Maps a note name (e.g. "C6") to its frequency in Hz. Returns 1047 for unknown notes.
    static int noteToFrequency(const QString &note);

protected:
    // --- AudioOutputElement hooks ---

    void startAudio() override;
    void stopAudio() override;
    void applyVolume() override;
    void applyMute() override;

private:
    QAudioSink *m_sink = nullptr;
    ToneGenerator *m_generator = nullptr;
    double m_frequency = 1047.0;
};

