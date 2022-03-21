/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef CLOCK_H
#define CLOCK_H

#include "graphicelement.h"
#include "input.h"

class Clock : public GraphicElement, public Input
{
private:
    int m_interval;
    int m_elapsed;
    bool m_isOn;
    double m_frequency;

public:
    explicit Clock(QGraphicsItem *parent = nullptr);
    ~Clock() override;
    static int current_id_number; // Number used to create distinct labels for each instance of this element.
    static bool reset;
    static bool pause; // static var to pause clock counting while simulating.
public:
    void save(QDataStream &ds) const override;
    void load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version) override;
    float getFrequency() const override;
    void setFrequency(float freq) override;
    void updateClock();
    void resetClock();
    QString genericProperties() override;

public:
    bool getOn(int port=0) const override;
    void setOn(bool value, int port=0) override;
    void setSkin(bool defaultSkin, const QString &filename) override;
};

#endif // CLOCK_H
