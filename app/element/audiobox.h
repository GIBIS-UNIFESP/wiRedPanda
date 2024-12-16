// Copyright 2015 - 2024, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "graphicelement.h"

#include <QAudioOutput>
#include <QFileInfo>
#include <QMediaPlayer>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QMediaPlaylist>
#endif

class AudioBox : public GraphicElement
{
    Q_OBJECT

public:
    explicit AudioBox(QGraphicsItem* parent = nullptr);
    AudioBox(const AudioBox &other) : AudioBox(other.parentItem()) {}

    QString audio() const override;
    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version) override;
    void mute(const bool mute = true);
    void refresh() override;
    void save(QDataStream &stream) const override;
    void setAudio(const QString &audioPath) override;

private:
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
