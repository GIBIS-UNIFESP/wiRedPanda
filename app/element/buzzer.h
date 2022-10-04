// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "graphicelement.h"

#include <QSoundEffect>
#include <QVersionNumber>

class Buzzer : public GraphicElement
{
    Q_OBJECT

public:
    explicit Buzzer(QGraphicsItem *parent = nullptr);
    Buzzer(const Buzzer &other) : Buzzer(other.parentItem()) {}

    QString audio() const override;
    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version) override;
    void mute(const bool mute = true);
    void refresh() override;
    void save(QDataStream &stream) const override;
    void setAudio(const QString &note) override;

private:
    void play();
    void stop();

    QSoundEffect *m_audio = nullptr;
    QString m_note;
    bool m_isPlaying = false;
    bool m_hasOutputDevice = false;
};

Q_DECLARE_METATYPE(Buzzer)
