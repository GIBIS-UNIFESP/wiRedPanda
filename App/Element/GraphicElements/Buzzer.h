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
    /// Default tone frequency in Hz (C6). Single source of truth: used by the
    /// member initializer and serialization's default-elision check.
    static constexpr double kDefaultFrequency = 1047.0;

    /// Default volume; deliberately lower than AudioOutputElement's 0.5 base
    /// default to avoid harsh tones. Single source of truth: used by the
    /// constructor and serialization's default-elision check.
    static constexpr float kDefaultVolume = 0.35f;

    /// Constructs a Buzzer element (default: 1047 Hz / C6).
    explicit Buzzer(QGraphicsItem *parent = nullptr);

    /// Copy-constructs by delegating to the parent item constructor.
    Buzzer(const Buzzer &other) : Buzzer(other.parentItem()) {}

    /// Destroys the sink before its tone generator: QObject deletes children in creation order, the generator first,
    /// and Qt 6.7's sink touches its source device as it is destroyed.
    ~Buzzer() override;

    // --- State Queries ---

    /// Returns the tone frequency in Hz.
    double frequency() const override;

    // --- Playback Control ---

    /// Sets the tone frequency in Hz, when acceptsFrequency().
    void setFrequency(double freq) override;
    /// Returns whether \a freq is finite and positive.
    bool acceptsFrequency(double freq) const override;
    /// Sets the tone by note name (e.g. "C6"), as older files hold it; an empty name changes nothing.
    void setNote(const QString &note);

    // --- Serialization ---

    /// \reimp
    void load(QDataStream &stream, SerializationContext &context) override;
    /// \reimp
    void save(QDataStream &stream, SerializationOptions options) const override;

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
    double m_frequency = kDefaultFrequency;
};
