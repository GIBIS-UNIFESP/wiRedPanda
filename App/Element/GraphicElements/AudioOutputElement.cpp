// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/AudioOutputElement.h"

#include <cmath>

#include <QString>

double AudioOutputElement::volumeTaken(const double volume)
{
    // Every version before kept the volume as a float, which a file holds widened (0.35 as 0.3499999940395355): such a
    // value is the float's shortest decimal, the volume the user set. A volume that is no float is kept as it is.
    Q_ASSERT_X(std::isfinite(volume), "AudioOutputElement::volumeTaken()", "the spec refuses a volume that is not finite");
    const auto single = static_cast<float>(volume);
    if (static_cast<double>(single) != volume) {
        return volume;
    }
    for (int digits = 1; digits <= 9; ++digits) {
        const double decimal = QString::number(static_cast<double>(single), 'g', digits).toDouble();
        if (static_cast<float>(decimal) == single) {
            return decimal;
        }
    }
    return volume; // LCOV_EXCL_LINE — nine significant digits name every finite float
}

ParamSpec AudioOutputElement::volumeSpec(const double defaultVolume)
{
    return {
        .id = ParamId::Volume,
        .defaultValue = defaultVolume,
        .key = "volume",
        .range = {0.0, 1.0},
        // A volume that is not finite is refused, leaving the volume as it was: a file's loads as the default
        .accept = [](const Element &, const ParamValue &value) -> std::optional<ParamValue> {
            const double volume = std::get<double>(value);
            return std::isfinite(volume) ? std::optional<ParamValue>(volumeTaken(volume)) : std::nullopt;
        },
    };
}

ParamSpec AudioOutputElement::playingSpec()
{
    return {.id = ParamId::Playing, .defaultValue = false};
}

AudioOutputElement::AudioOutputElement(ElementType type, QGraphicsItem *parent)
    : Element(type, parent)
{
}

bool AudioOutputElement::isPlaying() const
{
    return param<bool>(ParamId::Playing);
}

void AudioOutputElement::parameterChanged(const ParamId id, const ParamValue &previous)
{
    Element::parameterChanged(id, previous);
    if (id == ParamId::Playing) {
        pictureReplaced();
    }
}
