// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief AudioService: the application's playback of its audio elements.
 */

#pragma once

#include <memory>
#include <optional>
#include <unordered_map>

#include <QHash>
#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QString>

class Element;
class QAudioOutput;
class QAudioSink;
class QMediaPlayer;
class ToneGenerator;

/**
 * \class AudioService
 * \brief Plays the audio elements of every open document: a buzzer's tone, an audio box's sound file.
 *
 * \details An element holds only what it sounds: whether it plays (ParamId::Playing, which the simulation's visual
 * push sets from its input and announces through its host), its volume, and its tone or sound file. The service keeps
 * a voice for each element that has played, and sync() brings the voice to what the element holds; the scene calls it
 * for the elements the push changed, for a parameter changed, and for an element added, and release() for one
 * removed. Each document has its own mute, which pauses set too.
 *
 * A machine with no audio output device gets voices with no hardware, so what would play is known all the same.
 */
class AudioService : public QObject
{
    Q_OBJECT

    friend class TestAudioService;

public:
    /// What a voice holds: what it plays, and how.
    struct Playback {
        bool sounding = false;  ///< It plays.
        bool muted = false;     ///< Its document is muted.
        double volume = 0.0;    ///< Its volume, 0 to 1.
        double frequency = 0.0; ///< A buzzer's tone in Hz; 0 for a sound file.
        QString source;         ///< An audio box's sound: its file, or the built-in sound while that cannot be read.
    };

    /// Returns the application's service, a child of the application object, made on first use.
    static AudioService &instance();

    ~AudioService() override;

    /// Brings \a element's voice to what it holds: playing while it plays, at its volume and tone or sound, muted with
    /// its document. An element that never played gets no voice; one that is no audio element is skipped.
    void sync(Element &element);
    /// Stops and drops \a element's voice: it left its document.
    void release(const Element &element);

    /// Mutes or unmutes every voice of \a document, the scene its elements are in, and those it gets later.
    void setMuted(const QObject *document, bool muted);
    /// Returns whether \a document is muted.
    bool isMuted(const QObject *document) const;

    /// Returns what \a element's voice holds, or nothing when it has none.
    std::optional<Playback> playback(const Element &element) const;

private:
    /// One element's playback, and the hardware that plays it when there is an output device.
    struct Voice {
        Playback state;
        QPointer<QObject> document; ///< The scene the voice's element was in when last synced.
        QMetaObject::Connection destroyed;

        // A buzzer's: the generator outlives its sink, which Qt 6.7's sink touches as it is destroyed
        std::unique_ptr<ToneGenerator> generator;
        std::unique_ptr<QAudioSink> sink;
        // An audio box's: the player goes before the output it plays to
        std::unique_ptr<QAudioOutput> output;
        std::unique_ptr<QMediaPlayer> player;
    };

    AudioService();

    /// Returns the sound \a element plays: its file, or the built-in sound while the file cannot be read.
    static QString soundOf(const Element &element);
    void applyMute(Voice &voice) const;

    std::unordered_map<const Element *, std::unique_ptr<Voice>> m_voices;
    QHash<const QObject *, bool> m_muted; ///< Each document's mute; a document left out is not muted.
    bool m_hasOutputDevice = false;
};
