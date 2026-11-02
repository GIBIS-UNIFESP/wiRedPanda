// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/AudioOutput.h"

#include <cmath>

#include <QHash>

#include "App/Element/Element.h"

double AudioOutput::volumeTaken(const double volume)
{
    // Every version before kept the volume as a float, which a file holds widened (0.35 as 0.3499999940395355): such a
    // value is the float's shortest decimal, the volume the user set. A volume that is no float is kept as it is.
    Q_ASSERT_X(std::isfinite(volume), "AudioOutput::volumeTaken()", "the spec refuses a volume that is not finite");
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

ParamSpec AudioOutput::volumeSpec(const double defaultVolume)
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

ParamSpec AudioOutput::playingSpec()
{
    return {.id = ParamId::Playing, .defaultValue = false, .showsPicture = true};
}

int AudioOutput::noteToFrequency(const QString &note)
{
    // Backward-compatible aliases: the scale historically listed A7/B7 (an octave above
    // their neighbours); pre-4.x files persist the note *name*, so keep resolving these
    // to their true pitches. Kept on one physical line: gcov misattributes per-entry hit
    // counts across a multi-line brace-init list (see CodeGen's chained-.arg() precedent).
    static const QHash<QString, int> map = {{"C6", 1047}, {"D6", 1175}, {"E6", 1319}, {"F6", 1397}, {"G6", 1568}, {"A6", 1760}, {"B6", 1976}, {"C7", 2093}, {"A7", 3520}, {"B7", 3951}};

    return map.value(note, 1047);
}

void AudioOutput::setNote(Element &buzzer, const QString &note)
{
    if (note.isEmpty()) {
        return;
    }

    buzzer.setParam(ParamId::Frequency, static_cast<double>(noteToFrequency(note)));
}
