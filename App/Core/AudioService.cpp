// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/AudioService.h"

#include <QAudioDevice>
#include <QAudioOutput>
#include <QAudioSink>
#include <QCoreApplication>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QMediaDevices>
#include <QMediaPlayer>
#include <QUrl>

#include "App/Core/ToneGenerator.h"
#include "App/Element/AudioOutput.h"
#include "App/Element/Element.h"

AudioService &AudioService::instance()
{
    static QPointer<AudioService> service;
    if (!service) {
        service = new AudioService();
        // The application deletes it while Qt's multimedia backend is still there
        service->setParent(QCoreApplication::instance());
    }
    return *service;
}

AudioService::AudioService()
{
    // Detected once: a machine with no device plays nothing and keeps each voice's state all the same
    m_hasOutputDevice = !QMediaDevices::defaultAudioOutput().description().isEmpty();
}

AudioService::~AudioService()
{
    for (auto &[element, voice] : m_voices) {
        QObject::disconnect(voice->destroyed);
    }
}

QString AudioService::soundOf(const Element &element)
{
    // A sound that is gone or unreadable keeps its path, and the built-in sound plays in its place until it is back
    const QString path = element.audio();
    if (path.startsWith(":/")) {
        return path;
    }
    const QFileInfo info(path);
    return (info.exists() && info.isReadable()) ? path : QString::fromLatin1(AudioOutput::kBuiltInSound);
}

void AudioService::sync(Element &element)
{
    if (!element.hasParam(ParamId::Playing)) {
        return;
    }

    const bool playing = element.param<bool>(ParamId::Playing);
    auto found = m_voices.find(&element);
    if (found == m_voices.end()) {
        if (!playing) {
            return;
        }
        auto voice = std::make_unique<Voice>();
        voice->destroyed = connect(&element, &QObject::destroyed, this, [this, key = &element] { m_voices.erase(key); });
        found = m_voices.emplace(&element, std::move(voice)).first;
    }
    Voice &voice = *found->second;
    const bool tone = element.hasParam(ParamId::Frequency);
    const Playback before = voice.state;

    voice.document = element.scene();
    voice.state.muted = m_muted.value(element.scene(), false);
    voice.state.volume = element.volume();
    voice.state.frequency = tone ? element.frequency() : 0.0;
    voice.state.source = tone ? QString() : soundOf(element);
    voice.state.sounding = playing;

    if (!m_hasOutputDevice) {
        return;
    }

    if (tone) {
        if (!voice.sink) {
            voice.generator = std::make_unique<ToneGenerator>();
            voice.sink = std::make_unique<QAudioSink>(ToneGenerator::format());
        }
        voice.generator->setFrequency(voice.state.frequency);
        applyMute(voice);
        if (playing && (!before.sounding || (before.frequency != voice.state.frequency))) {
            // A new tone while playing restarts it, so it sounds at once. reset() discards what the sink holds of the
            // old tone, which stop() would play out first (Qt drains the buffer on Linux and macOS)
            voice.sink->reset();
            voice.generator->start();
            voice.sink->start(voice.generator.get());
        } else if (!playing && before.sounding) {
            voice.sink->reset();
            voice.generator->close();
        }
        return;
    }

    if (!voice.player) {
        voice.output = std::make_unique<QAudioOutput>();
        voice.player = std::make_unique<QMediaPlayer>();
        voice.player->setAudioOutput(voice.output.get());
        voice.player->setLoops(QMediaPlayer::Infinite);
    }
    applyMute(voice);
    if (voice.player->source().isEmpty() || (before.source != voice.state.source)) {
        const QString &source = voice.state.source;
        voice.player->setSource(source.startsWith(":/") ? QUrl("qrc" + source) : QUrl::fromLocalFile(source));
        // A new sound while playing plays at once
        if (before.sounding && playing) {
            voice.player->play();
        }
    }
    if (playing && !before.sounding) {
        voice.player->play();
    } else if (!playing && before.sounding) {
        voice.player->stop();
    }
}

void AudioService::applyMute(Voice &voice) const
{
    if (voice.sink) {
        voice.sink->setVolume(voice.state.muted ? 0.0 : voice.state.volume);
    }
    if (voice.output) {
        voice.output->setVolume(static_cast<float>(voice.state.volume));
        voice.output->setMuted(voice.state.muted);
    }
}

void AudioService::release(const Element &element)
{
    const auto found = m_voices.find(&element);
    if (found == m_voices.end()) {
        return;
    }
    QObject::disconnect(found->second->destroyed);
    m_voices.erase(found);
}

void AudioService::setMuted(const QObject *document, const bool muted)
{
    if (!m_muted.contains(document)) {
        connect(document, &QObject::destroyed, this, [this, document] { m_muted.remove(document); });
    }
    m_muted.insert(document, muted);
    for (auto &[element, voice] : m_voices) {
        if (voice->document.data() == document) {
            voice->state.muted = muted;
            applyMute(*voice);
        }
    }
}

bool AudioService::isMuted(const QObject *document) const
{
    return m_muted.value(document, false);
}

std::optional<AudioService::Playback> AudioService::playback(const Element &element) const
{
    const auto found = m_voices.find(&element);
    return (found == m_voices.end()) ? std::nullopt : std::optional<Playback>(found->second->state);
}
