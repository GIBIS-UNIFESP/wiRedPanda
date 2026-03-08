// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the musical note buzzer output.
 */

#pragma once

#include <QSoundEffect>
#include <QVersionNumber>

#include "App/Element/GraphicElement.h"

/**
 * \class Buzzer
 * \brief Audio output element that plays a musical note when its input is logic-1.
 *
 * \details Uses QSoundEffect to play short WAV notes from the resource bundle.
 * Supports multiple note selections and can be globally muted.
 */
class Buzzer : public GraphicElement
{
    Q_OBJECT

public:
    /// Constructs a Buzzer element (default note).
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
    /// Sets the note to \a note and reloads the sound effect resource.
    void setAudio(const QString &note) override;
    /// Refreshes the visual appearance based on current state.
    void refresh() override;

    // --- Serialization ---

    /// \reimp
    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version) override;
    /// \reimp
    void save(QDataStream &stream) const override;

private:
    // --- Internal methods ---

    void play();
    void stop();

    QSoundEffect *m_audio = nullptr;
    QString m_note;
    bool m_isPlaying = false;
    bool m_hasOutputDevice = false;
    bool m_muted = false;
};
