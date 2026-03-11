// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QAudioOutput>
#include <QFileInfo>
#include <QMediaPlayer>

#include "App/Element/GraphicElement.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QMediaPlaylist>
#endif

class AudioBox : public GraphicElement
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    explicit AudioBox(QGraphicsItem *parent = nullptr);
    AudioBox(const AudioBox &other) : AudioBox(other.parentItem()) {}

    // --- State Queries ---

    QString audio() const override;

    // --- Playback Control ---

    void mute(const bool mute = true);
    void setAudio(const QString &audioPath) override;
    void refresh() override;

    // --- Serialization ---

    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version) override;
    void save(QDataStream &stream) const override;

private:
    // --- Internal methods ---

    void play();
    void stop();

    QAudioOutput *m_audioOutput = nullptr;
    QFileInfo *m_audio = nullptr;
    QMediaPlayer *m_player = nullptr;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QMediaPlaylist *m_playlist = nullptr;
#endif

    bool m_hasOutputDevice = false;
    bool m_isPlaying = false;
};

Q_DECLARE_METATYPE(AudioBox)
