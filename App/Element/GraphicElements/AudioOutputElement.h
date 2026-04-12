// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Shared base class for audio output elements (AudioBox and Buzzer).
 */

#pragma once

#include "App/Element/GraphicElement.h"

/**
 * \class AudioOutputElement
 * \brief Abstract base for single-input audio output elements.
 *
 * \details Centralises the hardware-detection pattern, shared state
 * (m_isPlaying, m_muted, m_volume, m_hasOutputDevice), and the common
 * implementations of refresh(), isPlaying(), isMuted(), volume(),
 * mute(), setVolume(), play(), and stop().
 *
 * Concrete subclasses implement the four pure-virtual hardware hooks:
 * startAudio(), stopAudio(), applyVolume(), and applyMute().
 */
class AudioOutputElement : public GraphicElement
{
    Q_OBJECT

public:
    explicit AudioOutputElement(ElementType type, QGraphicsItem *parent = nullptr);

    // --- State Queries ---

    /// Returns \c true if audio is currently playing.
    bool isPlaying() const;
    /// Returns \c true if audio output is muted.
    bool isMuted() const;
    /// Returns the audio playback volume (0.0–1.0).
    float volume() const override;

    // --- Playback Control ---

    /// Mutes or unmutes audio according to \a mute.
    void mute(const bool mute = true);
    /// Sets the audio playback volume to \a vol (0.0–1.0).
    void setVolume(float vol) override;

    /// Refreshes the visual appearance based on the current input state.
    void refresh() override;

protected:
    // --- Template Method hooks (backend-specific) ---

    /// Starts hardware playback (called from play() when m_hasOutputDevice is true).
    virtual void startAudio() = 0;
    /// Stops hardware playback (called from stop() when m_hasOutputDevice is true).
    virtual void stopAudio() = 0;
    /// Applies the current m_volume to the hardware backend.
    virtual void applyVolume() = 0;
    /// Applies the current m_muted state to the hardware backend.
    virtual void applyMute() = 0;

    // --- Shared state (accessible to derived classes) ---

    float m_volume = 0.5f;
    bool m_hasOutputDevice = false;
    bool m_isPlaying = false;
    bool m_muted = false;

    // --- Non-virtual play/stop (delegate to hooks above) ---

    void play();
    void stop();
};

