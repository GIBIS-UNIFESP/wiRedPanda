// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Shared base class for audio output elements (AudioBox and Buzzer).
 */

#pragma once

#include "App/Element/Element.h"
#include "App/Element/Parameters.h"

/**
 * \class AudioOutputElement
 * \brief Abstract base for single-input audio output elements.
 *
 * \details Centralises the hardware-detection pattern, shared state
 * (m_muted, m_hasOutputDevice), the specs of the volume and of whether it plays
 * (volumeSpec(), playingSpec()),
 * and the common implementations of refresh(), isPlaying(), isMuted(),
 * mute(), play(), and stop().
 *
 * Concrete subclasses implement the four pure-virtual hardware hooks:
 * startAudio(), stopAudio(), applyVolume(), and applyMute().
 */
class AudioOutputElement : public Element
{
    Q_OBJECT

    friend class TestAudioBox;
    friend class TestBuzzer;

public:
    /// Default playback volume. Single source of truth: used by the audio box's
    /// volume spec and serialization's default-elision checks. Buzzer has its own
    /// lower kDefaultVolume instead.
    static constexpr float kDefaultVolume = 0.5f;

    /// Returns the volume's spec (ParamId::Volume, "volume") with default \a defaultVolume: any value is taken at a
    /// float's precision.
    static ParamSpec volumeSpec(float defaultVolume);

    /// Returns the spec of whether the element plays (ParamId::Playing), transient: its picture's slot (0 = silent,
    /// 1 = playing), which the presenter picks by it.
    static ParamSpec playingSpec();

    /// \param type Concrete audio element type, forwarded to the Element base.
    /// \param parent Optional parent item.
    explicit AudioOutputElement(ElementType type, QGraphicsItem *parent = nullptr);

    // --- State Queries ---

    /// Returns \c true if audio is currently playing.
    bool isPlaying() const;
    /// Returns \c true if audio output is muted.
    bool isMuted() const;

    // --- Playback Control ---

    /// Mutes or unmutes audio according to \a mute.
    void mute(const bool mute = true);

    /// Refreshes the visual appearance based on the current input state.
    void refresh() override;

protected:
    // --- Template Method hooks (backend-specific) ---

    /// Starts hardware playback (called from play() when m_hasOutputDevice is true).
    virtual void startAudio() = 0;
    /// Stops hardware playback (called from stop() when m_hasOutputDevice is true).
    virtual void stopAudio() = 0;
    /// Applies the current volume() to the hardware backend.
    virtual void applyVolume() = 0;
    /// Applies the current m_muted state to the hardware backend.
    virtual void applyMute() = 0;

    /// Applies a new volume to the hardware backend, and shows a start or a stop on the picture.
    void parameterChanged(ParamId id, const ParamValue &previous) override;

    // --- Shared state (accessible to derived classes) ---

    bool m_hasOutputDevice = false;
    bool m_muted = false;

    // --- Non-virtual play/stop (delegate to hooks above) ---

    void play();
    void stop();

private:
    /// Test-only seam letting TestAudioBox/TestBuzzer force the hardware-detection result
    /// deterministically. The real result depends on whether the machine running the test
    /// has an audio device, which varies between sandboxes and CI and would otherwise make
    /// the m_hasOutputDevice-gated branches' coverage environment-dependent.
    void setHasOutputDeviceForTesting(bool has) { m_hasOutputDevice = has; }
};
