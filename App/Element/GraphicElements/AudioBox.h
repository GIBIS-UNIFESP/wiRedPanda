// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the AudioBox (external audio file player) output.
 */

#pragma once

#include <QAudioOutput>
#include <QFileInfo>
#include <QMediaPlayer>

#include "App/Element/GraphicElement.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QMediaPlaylist>
#endif

/**
 * \class AudioBox
 * \brief Audio output element that streams an external audio file when its input is logic-1.
 *
 * \details Uses QMediaPlayer to play arbitrary audio files (MP3, WAV, OGG, etc.).
 * Supports looping and can be globally muted.
 */
class AudioBox : public GraphicElement
{
    Q_OBJECT

public:
    /// Constructs an AudioBox element.
    explicit AudioBox(QGraphicsItem *parent = nullptr);

    /// Copy-constructs by delegating to the parent item constructor.
    AudioBox(const AudioBox &other) : AudioBox(other.parentItem()) {}

    // --- State Queries ---

    /// Returns the file path of the currently loaded audio file.
    QString audio() const override;

    // --- Playback Control ---

    /// Mutes or unmutes playback according to \a mute.
    void mute(const bool mute = true);
    /// Sets the audio source to \a audioPath and reloads the player.
    void setAudio(const QString &audioPath) override;
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

    QAudioOutput *m_audioOutput = nullptr;
    QFileInfo *m_audio = nullptr;
    QMediaPlayer *m_player = nullptr;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QMediaPlaylist *m_playlist = nullptr;
#endif

    bool m_hasOutputDevice = false;
    bool m_isPlaying = false;
};

Q_DECLARE_METATYPE(AudioBox)
