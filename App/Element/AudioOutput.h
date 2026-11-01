// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief AudioOutput: what the audio outputs (the buzzer and the audio box) share.
 */

#pragma once

#include <QString>

#include "App/Element/Parameters.h"

class Element;

/// What the buzzer and the audio box hold of their sound, as parameters: whether they play (playingSpec(), which the
/// simulation's visual push sets from the input) and their volume (volumeSpec()). The application's AudioService plays
/// them.
namespace AudioOutput {

/// The audio box's default volume.
inline constexpr double kBoxVolume = 0.5;
/// The buzzer's default volume, lower than the audio box's to avoid harsh tones.
inline constexpr double kBuzzerVolume = 0.35;
/// The buzzer's default tone in Hz (C6).
inline constexpr double kBuzzerFrequency = 1047.0;
/// The audio box's default sound, the bundled sample: also what plays in place of a sound that cannot be read.
inline constexpr const char *kBuiltInSound = ":/Components/Output/Audio/wiredpanda.wav";

/// Returns the volume's spec (ParamId::Volume, "volume") with default \a defaultVolume; a finite value is taken as
/// volumeTaken() has it, one that is not finite refused.
ParamSpec volumeSpec(double defaultVolume);

/// Returns \a volume, finite, as the spec takes it: a value that is exactly a float, as every earlier version kept the
/// volume, is that float's shortest decimal; any other value is itself.
double volumeTaken(double volume);

/// Returns the spec of whether the element plays (ParamId::Playing), transient: its picture's slot (0 = silent,
/// 1 = playing), which the presenter picks by it, so a new value replaces the picture.
ParamSpec playingSpec();

/// Returns the frequency in Hz of the note named \a note, as older files hold a buzzer's tone; C6's for a name not on
/// the scale.
int noteToFrequency(const QString &note);

/// Sets \a buzzer's tone by note name (noteToFrequency()); an empty name changes nothing.
void setNote(Element &buzzer, const QString &note);

} // namespace AudioOutput
