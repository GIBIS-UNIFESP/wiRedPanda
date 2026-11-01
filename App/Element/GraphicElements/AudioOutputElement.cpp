// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/AudioOutputElement.h"

#include <cmath>

#include <QAudioDevice>
#include <QMediaDevices>
#include <QString>

#include "App/Wiring/Port.h"

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
    // Detect audio hardware once per process; subsequent calls are guarded by
    // m_hasOutputDevice so the element works silently in headless/CI environments.
    static const bool hasOutputDevice = !QMediaDevices::defaultAudioOutput().description().isEmpty();
    m_hasOutputDevice = hasOutputDevice;
}

void AudioOutputElement::refresh()
{
    if (!isValid()) {
        stop();
        return;
    }

    const Status inputValue = inputs().constFirst()->status();

    (inputValue == Status::Active) ? play() : stop();
}

bool AudioOutputElement::isPlaying() const
{
    return param<bool>(ParamId::Playing);
}

bool AudioOutputElement::isMuted() const
{
    return m_muted;
}

void AudioOutputElement::mute(const bool mute)
{
    m_muted = mute;
    if (!m_hasOutputDevice) {
        return;
    }
    applyMute();
}

void AudioOutputElement::parameterChanged(const ParamId id, const ParamValue &previous)
{
    Element::parameterChanged(id, previous);
    if ((id == ParamId::Volume) && m_hasOutputDevice) {
        applyVolume();
    }
    if (id == ParamId::Playing) {
        pictureReplaced();
    }
}

void AudioOutputElement::play()
{
    if (isPlaying()) {
        return;
    }

    // The player starts before the element plays: a box whose sound comes back reloads it as it starts
    // (AudioBox::startAudio()), and a reload starts a box already playing, which would start it twice
    if (m_hasOutputDevice) {
        startAudio();
    }

    setParam(ParamId::Playing, true);
}

void AudioOutputElement::stop()
{
    if (!isPlaying()) {
        return;
    }

    setParam(ParamId::Playing, false);

    if (m_hasOutputDevice) {
        stopAudio();
    }
}
