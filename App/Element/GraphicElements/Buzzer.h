// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/// \file
/// \brief Graphic element for the musical note buzzer output.

#pragma once

#include <QVersionNumber>

#include "App/Element/GraphicElement.h"

class QAudioSink;
class ToneGenerator;

/// Audio output element that plays a continuous tone when its input is logic-1.
///
/// Uses a sine-wave ToneGenerator fed to QAudioSink for seamless,
/// glitch-free playback at any frequency.
class Buzzer : public GraphicElement
{
    Q_OBJECT

public:
    /// Constructs a Buzzer element (default note: C6 / 1047 Hz).
    explicit Buzzer(QGraphicsItem *parent = nullptr);

    /// Copy-constructs by delegating to the parent item constructor.
    Buzzer(const Buzzer &other) : Buzzer(other.parentItem()) {}

    // --- State Queries ---

    /// Returns the name of the currently selected musical note.
    QString audio() const override;
    /// Returns \c true if the buzzer sound is currently playing.
    bool isPlaying() const;
    /// Returns \c true if audio output is muted.
    bool isMuted() const;

    // --- Playback Control ---

    /// Mutes or unmutes playback according to \a mute.
    void mute(const bool mute = true);
    /// Sets the note to \a note (e.g. "C6") and updates the tone frequency.
    void setAudio(const QString &note) override;
    /// Refreshes the visual appearance based on current state.
    void refresh() override;

    // --- Serialization ---

    /// \reimp
    void load(QDataStream &stream, SerializationContext &context) override;
    /// \reimp
    void save(QDataStream &stream) const override;

    /// Maps a note name (e.g. "C6") to its frequency in Hz.
    static int noteToFrequency(const QString &note);

private:
    // --- Internal methods ---

    void play();
    void stop();

    QAudioSink *m_sink = nullptr;
    ToneGenerator *m_generator = nullptr;
    QString m_note;
    bool m_isPlaying = false;
    bool m_hasOutputDevice = false;
    bool m_muted = false;
};

