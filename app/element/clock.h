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
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)
    Q_PROPERTY(QString pixmap MEMBER m_pixmap CONSTANT)

public:
    explicit Clock(QGraphicsItem *parent = nullptr);
    ~Clock() override = default;

    static int current_id_number; // Number used to create distinct labels for each instance of this element.
    static bool reset;
    static bool pause; // static var to pause clock counting while simulating.

    void save(QDataStream &ds) const override;
    void load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version) override;
    float getFrequency() const override;
    void setFrequency(float freq) override;
    void updateClock();
    void resetClock();
    QString genericProperties() override;

    bool getOn(int port = 0) const override;
    void setOn(bool value, int port = 0) override;
    void setSkin(bool defaultSkin, const QString &filename) override;

    const QString m_titleText = tr("<b>CLOCK SIGNAL</b>");
    const QString m_translatedName = tr("Clock");
    const QString m_pixmap = ":/input/clock1.png";

private:
    int m_interval;
    int m_elapsed;
    bool m_isOn;
    double m_frequency;
};

Q_DECLARE_METATYPE(Clock)
