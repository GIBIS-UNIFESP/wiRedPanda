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
    Q_PROPERTY(QString pixmap MEMBER m_pixmap CONSTANT)
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)

public:
    explicit Buzzer(QGraphicsItem *parent = nullptr);
    Buzzer(const Buzzer &other) : Buzzer(other.parentItem()) {};

    QString audio() const override;
    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const double version) override;
    void mute(const bool mute = true);
    void refresh() override;
    void save(QDataStream &stream) const override;
    void setAudio(const QString &note) override;
    void setSkin(const bool defaultSkin, const QString &fileName) override;

private:
    void playBuzzer();
    void stopBuzzer();

    inline static int m_labelNumber = 0;

    const QString m_pixmap = ":/output/BuzzerOff.png";
    const QString m_titleText = tr("<b>BUZZER</b>");
    const QString m_translatedName = tr("Buzzer");

    QSoundEffect m_audio;
    QString m_note;
    bool m_isPlaying = false;

};

Q_DECLARE_METATYPE(Buzzer)
