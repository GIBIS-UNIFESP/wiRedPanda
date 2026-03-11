// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "graphicelementinput.h"

class Clock : public GraphicElementInput
{
    Q_OBJECT

public:
    explicit Clock(QGraphicsItem *parent = nullptr);

    QString genericProperties() override;
    bool isOn(const int port = 0) const override;
    float delay() const override;
    float frequency() const override;
    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version) override;
    void resetClock(const std::chrono::steady_clock::time_point &globalTime);
    void save(QDataStream &stream) const override;
    void setDelay(const float delay) override;
    void setFrequency(const float freq) override;
    void setOff() override;
    void setOn() override;
    void setOn(const bool value, const int port = 0) override;
    void setSkin(const bool defaultSkin, const QString &fileName) override;
    void updateClock(const std::chrono::steady_clock::time_point &globalTime);

private:
    bool m_isOn = false;
    double m_delay = 0;
    double m_frequency = 0;
    std::chrono::microseconds m_interval;
    std::chrono::steady_clock::time_point m_startTime;
};

Q_DECLARE_METATYPE(Clock)
