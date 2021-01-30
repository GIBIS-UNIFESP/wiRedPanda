/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef BUZZER_H
#define BUZZER_H

#include "graphicelement.h"

#include <QSound>
#include <QSoundEffect>

class Buzzer : public GraphicElement
{
public:
    explicit Buzzer(QGraphicsItem *parent = nullptr);
    ~Buzzer() override = default;
    static int current_id_number; // Number used to create distinct labels for each instance of this element.

public:
    /* GraphicElement interface */
    ElementType elementType() override
    {
        return ElementType::BUZZER;
    }
    ElementGroup elementGroup() override
    {
        return ElementGroup::OUTPUT;
    }
    void refresh() override;

    void setAudio(QString note) override;
    QString getAudio() const override;

    void mute(bool _mute = true);

private:
    void playbuzzer();
    void stopbuzzer();

    QVector<QString> alternativeSkins;
    int play;
    QSoundEffect m_audio;
    QString m_note;
    // GraphicElement interface
public:
    void save(QDataStream &ds) const override;
    void load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version) override;
    void setSkin(bool defaultSkin, QString filename) override;
};

#endif // BUZZER_H
