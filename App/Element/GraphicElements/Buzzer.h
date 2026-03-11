// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QSoundEffect>
#include <QVersionNumber>

#include "App/Element/GraphicElement.h"

class Buzzer : public GraphicElement
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    explicit Buzzer(QGraphicsItem *parent = nullptr);
    Buzzer(const Buzzer &other) : Buzzer(other.parentItem()) {}

    // --- State Queries ---

    QString audio() const override;

    // --- Playback Control ---

    void mute(const bool mute = true);
    void setAudio(const QString &note) override;
    void refresh() override;

    // --- Serialization ---

    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version) override;
    void save(QDataStream &stream) const override;

private:
    // --- Internal methods ---

    void play();
    void stop();

    QSoundEffect *m_audio = nullptr;
    QString m_note;
    bool m_isPlaying = false;
    bool m_hasOutputDevice = false;
};

Q_DECLARE_METATYPE(Buzzer)
