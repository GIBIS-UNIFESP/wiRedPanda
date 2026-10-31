// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/AudioOutputElement.h"

#include <QAudioDevice>
#include <QMediaDevices>

#include "App/Wiring/Port.h"

ParamSpec AudioOutputElement::volumeSpec(const float defaultVolume)
{
    return {
        .id = ParamId::Volume,
        .defaultValue = static_cast<double>(defaultVolume),
        .key = "volume",
        .accept = [](const Element &, const ParamValue &value) -> std::optional<ParamValue> {
            return static_cast<double>(static_cast<float>(std::get<double>(value)));
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
    // Position label to the right of the 64×64 body, vertically centred
    setLabelAnchor(QPointF(64, 34));

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

    setParam(ParamId::Playing, true);

    if (m_hasOutputDevice) {
        startAudio();
    }
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
