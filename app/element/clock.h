/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef CLOCK_H
#define CLOCK_H

#include "globalproperties.h"
#include "graphicelement.h"
#include "input.h"

#include <QTime>

class Clock : public GraphicElement, public Input
{
    int interval;
    int elapsed;
    bool on;
    double m_frequency;

public:
    explicit Clock(QGraphicsItem *parent = nullptr);
    ~Clock() override;
    static int current_id_number; // Number used to create distinct labels for each instance of this element.
    static bool reset;
public:
    void save(QDataStream &ds) const override;
    void load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version) override;
    float getFrequency() const override;
    void setFrequency(float freq) override;
    void updateClock();
    void resetClock();
    QString genericProperties() override;

public:
    bool getOn() const override;
    void setOn(bool value) override;
    void setSkin(bool defaultSkin, QString filename) override;
};

#endif // CLOCK_H
