/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelementinput.h"

class Clock : public GraphicElementInput
{
    Q_OBJECT

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
    bool m_isOn = false;
    double m_frequency = 0;
    std::chrono::duration<float, std::milli> m_interval;
    std::chrono::steady_clock::time_point m_timePoint;
};

Q_DECLARE_METATYPE(Clock)
