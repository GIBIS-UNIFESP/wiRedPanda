/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelementinput.h"

class Clock : public GraphicElementInput
{
    Q_OBJECT
    Q_PROPERTY(QString pixmapPath MEMBER m_pixmapPath CONSTANT)
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)

public:
    explicit Clock(QGraphicsItem *parent = nullptr);

    inline static bool pause = false; // pause clock counting while simulating.
    inline static bool reset = true;

    QString genericProperties() override;
    bool isOn(const int port = 0) const override;
    float frequency() const override;
    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const double version) override;
    void resetClock();
    void save(QDataStream &stream) const override;
    void setFrequency(const float freq) override;
    void setOff() override;
    void setOn() override;
    void setOn(const bool value, const int port = 0) override;
    void setSkin(const bool defaultSkin, const QString &fileName) override;
    void updateClock();

private:
    const QString m_pixmapPath = ":/input/clock1.svg";
    const QString m_titleText = tr("<b>CLOCK SIGNAL</b>");
    const QString m_translatedName = tr("Clock");

    bool m_isOn = false;
    double m_frequency = 0;
    int m_elapsed = 0;
    int m_interval = 0;
};

Q_DECLARE_METATYPE(Clock)
