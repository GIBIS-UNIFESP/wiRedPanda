/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"
#include "input.h"

class Clock : public GraphicElement, public Input
{
    Q_OBJECT
    Q_PROPERTY(QString pixmap MEMBER m_pixmap CONSTANT)
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)

public:
    explicit Clock(QGraphicsItem *parent = nullptr);

    inline static bool pause = false; // pause clock counting while simulating.
    inline static bool reset = true;

    QString genericProperties() override;
    bool on(const int port = 0) const override;
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
    const QString m_pixmap = ":/input/clock1.png";
    const QString m_titleText = tr("<b>CLOCK SIGNAL</b>");
    const QString m_translatedName = tr("Clock");

    bool m_isOn = false;
    double m_frequency;
    int m_elapsed;
    int m_interval;
};

Q_DECLARE_METATYPE(Clock)
