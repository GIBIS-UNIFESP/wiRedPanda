// Copyright 2015 - 2024, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "graphicelement.h"

#include <QMediaPlayer>
#include <QAudioOutput>
#include <QFileInfo>

class AudioBox : public GraphicElement
{
    Q_OBJECT

public:
    explicit AudioBox(QGraphicsItem* parent = nullptr);
    AudioBox(const AudioBox &other) : AudioBox(other.parentItem()) {};

    QString audio() const override;
    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version) override;
    void mute(const bool mute = true);
    void refresh() override;
    void save(QDataStream &stream) const override;
    void setAudio(const QString &audioPath) override;

private:
    void play();
    void stop();

    QMediaPlayer *m_player = nullptr;
    QAudioOutput *m_audioOutput = nullptr;
    QFileInfo *m_audio = nullptr;

    bool m_isPlaying = false;
    bool m_hasOutputDevice = false;
};

Q_DECLARE_METATYPE(AudioBox)
