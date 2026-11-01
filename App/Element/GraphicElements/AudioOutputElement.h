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
 * \details Holds what the element sounds, as parameters: whether it plays (playingSpec(), which the simulation's
 * visual push sets from its input) and its volume (volumeSpec()). The application's AudioService plays it.
 */
class AudioOutputElement : public Element
{
    Q_OBJECT

public:
    /// Default playback volume. Single source of truth: used by the audio box's
    /// volume spec and serialization's default-elision checks. Buzzer has its own
    /// lower kDefaultVolume instead.
    static constexpr double kDefaultVolume = 0.5;

    /// Returns the volume's spec (ParamId::Volume, "volume") with default \a defaultVolume; a finite value is taken as
    /// volumeTaken() has it, one that is not finite refused.
    static ParamSpec volumeSpec(double defaultVolume);

    /// Returns \a volume, finite, as the spec takes it: a value that is exactly a float, as every earlier version kept
    /// the volume, is that float's shortest decimal; any other value is itself.
    static double volumeTaken(double volume);

    /// Returns the spec of whether the element plays (ParamId::Playing), transient: its picture's slot (0 = silent,
    /// 1 = playing), which the presenter picks by it.
    static ParamSpec playingSpec();

    /// \param type Concrete audio element type, forwarded to the Element base.
    /// \param parent Optional parent item.
    explicit AudioOutputElement(ElementType type, QGraphicsItem *parent = nullptr);

    /// Returns \c true if the element plays.
    bool isPlaying() const;

protected:
    /// Shows a start or a stop on the picture.
    void parameterChanged(ParamId id, const ParamValue &previous) override;
};
