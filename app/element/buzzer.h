/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"

#include <QSoundEffect>

class Buzzer : public GraphicElement
{
    Q_OBJECT
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)
    Q_PROPERTY(QString pixmap MEMBER m_pixmap CONSTANT)

public:
    explicit Buzzer(QGraphicsItem *parent = nullptr);
    Buzzer(const Buzzer &other) : Buzzer(other.parentItem()) {};

    static int current_id_number; // Number used to create distinct labels for each instance of this element.

    QString getAudio() const override;
    void load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version) override;
    void mute(bool _mute = true);
    void refresh() override;
    void save(QDataStream &ds) const override;
    void setAudio(const QString &note) override;
    void setSkin(bool defaultSkin, const QString &filename) override;

private:
    void playbuzzer();
    void stopbuzzer();

    const QString m_titleText = tr("<b>BUZZER</b>");
    const QString m_translatedName = tr("Buzzer");
    const QString m_pixmap = ":/output/BuzzerOff.png";

    QVector<QString> m_alternativeSkins;
    // TODO: this could just be a bool
    int m_play;
    QSoundEffect m_audio;
    QString m_note;
};

Q_DECLARE_METATYPE(Buzzer)
